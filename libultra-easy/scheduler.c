#include <ultra64.h>
#include "libultra-easy.h"
#include "strings.h"

/* =================================================== *
 *                 FUNCTION PROTOTYPES                 *
 * =================================================== */

static void scheduler_threadfunc(void *arg) __attribute__ ((noreturn));
static void scheduler_vsync();
static void scheduler_prenmi();

/* =================================================== *
 *                     PROTOTYPES                      *
 * =================================================== */

static OSThread	scheduler_thread;
static Scheduler scheduler;

/**
 * @brief Returns the framebuffer currently in use.
 */
FrameBuffer* fb_current;

/* =================================================== *
 *                      FUNCTIONS                      *
 * =================================================== */

Scheduler* init_scheduler()
{
	// Initialize globals
	fb_current = NULL;

	// Initialize the scheduler
	scheduler.initialized = FALSE;
	scheduler.display = FALSE;
	scheduler.is_changing_res = FALSE;
	scheduler.reset = FALSE;
	scheduler.crash = FALSE;
	scheduler.current_status = SC_MSG_IDLE;
	scheduler.task_gfx = NULL;
	scheduler.task_audio = NULL;
	scheduler.gfx_notify = NULL;
	scheduler.audio_notify = NULL;
	osCreateMesgQueue(&scheduler.queue, scheduler.msg, SC_MSG_COUNT);

	// Initialize the TV
	osCreateViManager(OS_PRIORITY_VIMGR);
	display_set(-1);
	display_off();

	// Set the target framerate and register the event callbacks
	osViSetEvent(&scheduler.queue, (OSMesg)SC_MSG_VSYNC, 1);
	osSetEventMesg(OS_EVENT_SP, &scheduler.queue, (OSMesg)SC_MSG_SP);
	osSetEventMesg(OS_EVENT_AI, &scheduler.queue, (OSMesg)SC_MSG_AUDIO);
	osSetEventMesg(OS_EVENT_PRENMI, &scheduler.queue, (OSMesg)SC_MSG_PRENMI);

	// Start the scheduler thread
	osCreateThread(&scheduler_thread, ID_SCHEDULER, scheduler_threadfunc, NULL, REAL_STACK(SCHEDULER), PR_SCHEDULER);
	osStartThread(&scheduler_thread);

	// Return the scheduler object
	scheduler.initialized = TRUE;
	return &scheduler;
}

static void scheduler_threadfunc(void *arg)
{
	OSMesg msg;

	while (1)
	{
		osRecvMesg(&scheduler.queue, (OSMesg *)&msg, OS_MESG_BLOCK);

		// Ignore if the framebuffer resolution is being changed
		if (scheduler.is_changing_res)
		{
			debug_printf("[Scheduler] Waiting until display resolution change is completed\n");
			while (scheduler.is_changing_res) { ; }
		}

		scheduler.current_status = (int)msg;

		switch (scheduler.current_status)
		{
			case SC_MSG_VSYNC:
				if (scheduler.gfx_notify != NULL)
				{
					osSendMesg(scheduler.gfx_notify, NULL, OS_MESG_BLOCK);
					scheduler.gfx_notify = NULL;
				}
				scheduler_vsync();

				if (scheduler.audio_notify != NULL)
					osSendMesg(scheduler.audio_notify, NULL, OS_MESG_BLOCK);
				break;

			case SC_MSG_AUDIO:
				break;

			case SC_MSG_PRENMI:
				scheduler_prenmi();
				break;

			case SC_MSG_RCPDEAD:
				display_set(0);
				my_assert(FALSE, "RCP hang");
				break;

			default:
				break;
		}
	}
}

static void render_reset_screen()
{
	// Draw "restarting" screen
	extern void clear_screen_raw(FrameBuffer *fb);

	if (scheduler.crash == TRUE)
		return;

	osViBlack(FALSE);
	debug_printf("[Scheduler] Rendering Pre-NMI screen to CPU\n");

	if (fb_current != NULL)
	{
		clear_screen_raw(fb_current);
		console_clear();
		console_puts("%s", strings[0]);
		console_draw_raw(fb_current);

		osWritebackDCache(fb_current->address, fb_current->size);
		osViSwapBuffer(fb_current->address);
	}
}

static void scheduler_vsync()
{
	extern bool is_framebuffer_ready();

	// Check if a framebuffer that is ready exists, if not then we have to wait for the next retrace
	if (!is_framebuffer_ready())
	{
		// If the reset button was pressed recently, we can do some silly screen wipe
		if (scheduler.reset)
		{
			render_reset_screen();
			for (;;) {;}
		}
	}

	else if (!scheduler.crash)
	{
		extern FrameBuffer* return_ready_framebuffer();
		FrameBuffer *fb = return_ready_framebuffer();

		// Mark the old framebuffer as free, and swap with the new one
		debug_printf("[Scheduler] Swapping framebuffer\n");
		if (fb_current != NULL)
			fb_current->status = FB_FREE;
		fb_current = fb;
		osViSwapBuffer(fb->address);

		// Turn the TV on if it isn't
		if (scheduler.display == FALSE)
			display_on();
	}
}

void scheduler_discard_inactive_framebuffers()
{
	extern int num_active_framebuffers();
	int i;
	for (i = 0; i < num_active_framebuffers(); i++)
	{
		if (framebuffers[i].address != fb_current)
			framebuffers[i].status = FB_BUSY;
		else
			framebuffers[i].status = FB_READY;
	}
}

static void scheduler_prenmi()
{
	debug_printf("[Scheduler] Reset button pressed, initiating pre-NMI\n");
	osViSetYScale(1.0);
	// Stop threads go here
	gfx_close();
	audio_close();
	osSpTaskYield();
	osAfterPreNMI();
	scheduler.reset = TRUE;
}