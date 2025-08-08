#include <ultra64.h>
#include "libultra-easy.h"
#include "strings.h"

/* =================================================== *
 *                 FUNCTION PROTOTYPES                 *
 * =================================================== */

static void			scheduler_threadfunc(void *arg) __attribute__ ((noreturn));
static void			scheduler_video_pop();
static void			scheduler_prenmi();

/* =================================================== *
 *                     PROTOTYPES                      *
 * =================================================== */

static OSThread		scheduler_thread;
static Scheduler	scheduler;

/**
 * @brief Returns the framebuffer currently in use.
 */
FrameBuffer* fb_current;

/* =================================================== *
 *                      FUNCTIONS                      *
 * =================================================== */

Scheduler* init_scheduler()
{
	if (!scheduler.initialized)
	{
		// Initialize globals
		fb_current = NULL;

		// Initialize the scheduler
		scheduler = (Scheduler){0};
		osCreateMesgQueue(&scheduler.event_queue, scheduler.event_msg, SC_MSG_COUNT);

		// Initialize the TV
		osCreateViManager(OS_PRIORITY_VIMGR);
		display_set(-1);
		display_off();

		// Set the target framerate and register the event callbacks
		osViSetEvent(&scheduler.event_queue, (OSMesg)SC_MSG_VSYNC, RETRACE_COUNT);
		#ifdef ENABLE_AUDIO
		osSetEventMesg(OS_EVENT_AI, &scheduler.event_queue, (OSMesg)SC_MSG_AUDIO);
		#endif
		osSetEventMesg(OS_EVENT_DP, &scheduler.event_queue, (OSMesg)SC_MSG_DP);
		osSetEventMesg(OS_EVENT_SP, &scheduler.event_queue, (OSMesg)SC_MSG_SP);
		osSetEventMesg(OS_EVENT_PRENMI, &scheduler.event_queue, (OSMesg)SC_MSG_PRENMI);

		// Start the scheduler thread
		osCreateThread(&scheduler_thread, ID_SCHEDULER, scheduler_threadfunc, NULL, REAL_STACK(SCHEDULER), PR_SCHEDULER);
		osStartThread(&scheduler_thread);

		// Return the scheduler object
		scheduler.initialized = TRUE;
	}

	return &scheduler;
}

static void scheduler_threadfunc(void *arg)
{
	OSMesg msg;

	while (1)
	{
		osRecvMesg(&scheduler.event_queue, (OSMesg *)&msg, OS_MESG_BLOCK);

		// Ignore if the framebuffer resolution is being changed
		if (scheduler.is_changing_res)
		{
			debug_printf("[Scheduler] Waiting until display resolution change is completed\n");
			while (scheduler.is_changing_res) { ; }
		}

		scheduler.current_status = (int)msg;

		switch (scheduler.current_status)
		{
			case SC_MSG_DP:
				break;

			case SC_MSG_VSYNC:
				if (scheduler.gfx_notify != NULL)
				{
					osSendMesg(scheduler.gfx_notify, &msg, OS_MESG_BLOCK);
					scheduler.gfx_notify = NULL;
				}

				scheduler_video_pop();

				if (scheduler.audio_notify != NULL)
				{
					osSendMesg(scheduler.audio_notify, &msg, OS_MESG_BLOCK);
					scheduler.audio_notify = NULL;
				}
				break;

			#ifdef ENABLE_AUDIO
			case SC_MSG_AUDIO:
				if (scheduler.audio_notify != NULL)
				{
					osSendMesg(scheduler.audio_notify, &msg, OS_MESG_BLOCK);
					scheduler.audio_notify = NULL;
				}
				break;
			#endif

			case SC_MSG_PRENMI:
				scheduler_prenmi();
				break;

			case SC_MSG_RCPDEAD:
				display_set(0);
				my_assert(FALSE, /* "RCP hang" */ "RCP is HUNG UP!!\nOh! MY GOD!!");
				break;

			default:
				break;
		}
	}
}

static void render_reset_screen()
{
	// Draw "restarting" screen
	extern void clear_screen_raw();

	if (scheduler.crash == TRUE)
		return;

	osViBlack(FALSE);
	debug_printf("[Scheduler] Rendering Pre-NMI screen to CPU\n");

	if (console_set_fb())
	{
		clear_screen_raw();
		console_clear();
		console_puts("%s", strings[0]);
		console_draw_raw();
	}
}

static void scheduler_video_pop()
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
	// audio_close();
	osSpTaskYield();
	osAfterPreNMI();
	scheduler.reset = TRUE;
}