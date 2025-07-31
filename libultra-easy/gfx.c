#include <ultra64.h>
#include <math.h>
#include "libultra-easy.h"

/* =================================================== *
 *                       MACROS                        *
 * =================================================== */

#ifdef VIDEO_32BIT
	#define FB_DEPTH		u32
	#define ZBUFFER_ADDR	RAMBANK_8 + RAMBANK_SIZE - CFB_SIZE
	#define CFB_SIZE		(SCREEN_W_HD*SCREEN_H_HD*4) // HD minimum size
#else
	#define FB_DEPTH 		u16
	#define ZBUFFER_ADDR	RAMBANK_4 + RAMBANK_SIZE - CFB_SIZE
	#define CFB_SIZE		(SCREEN_W_HD*SCREEN_H_HD*2) // HD minimum size
#endif

#if (CFB_COUNT > 3 || CFB_COUNT <= 1)
	#error Invalid framebuffer count
#endif

#define MSG_SIZE (CFB_COUNT + 1)

/* =================================================== *
 *                 FUNCTION PROTOTYPES                 *
 * =================================================== */

static void gfx_threadfunc(void *arg) __attribute__ ((noreturn));
static void gfx_render(FrameBuffer *fb, void (*func)());
static void gfx_create_fb(void *address);

/* =================================================== *
 *                     PROTOTYPES                      *
 * =================================================== */

extern OSTask rcp_task;
extern Gfx glist[CFB_COUNT][GDL_SIZE];

// Render request buffers
static RenderMessage msg_buffer[MSG_SIZE];
static int msg_index;

// Framebuffers
static FrameBuffer *fb_target;
static FrameBuffer *fb_last;
static FrameBuffer *fb_current;
static int active_framebuffers;

static OSThread		gfx_thread;
static OSTimer      rcp_timer;
static OSMesg		msg_gfx[CFB_COUNT];
static OSMesgQueue	msg_queue_gfx;
static OSMesg		msg_vsync;
static OSMesgQueue	msg_queue_vsync;
static OSMesg		msg_rdp;
static OSMesgQueue	msg_queue_rdp;

static Scheduler *scheduler;

#define CFB1_ADDR		ZBUFFER_ADDR - (CFB_SIZE*1)
#define CFB2_ADDR		ZBUFFER_ADDR - (CFB_SIZE*2)
#define CFB3_ADDR		ZBUFFER_ADDR - (CFB_SIZE*3)

/* =================================================== *
 *              GRAPHICS TASK FUNCTIONING              *
 * =================================================== */

void init_gfx(Scheduler *sc)
{
	active_framebuffers = 0;

	// Create framebuffers
	zbuffer = (u16 *)(ZBUFFER_ADDR);
	if (CFB_COUNT >= 1) { gfx_create_fb((FB_DEPTH*)(CFB1_ADDR)); }
	if (CFB_COUNT >= 2) { gfx_create_fb((FB_DEPTH*)(CFB2_ADDR)); }
	if (CFB_COUNT >= 3) { gfx_create_fb((FB_DEPTH*)(CFB3_ADDR)); }

	// Clear Z-buffer
	bzero(zbuffer, CFB_SIZE);

	// Set scheduler
	scheduler = sc;

	// Start the graphics thread
	osCreateThread(&gfx_thread, ID_GFX, gfx_threadfunc, NULL, REAL_STACK(GFX), PR_GFX);
	osStartThread(&gfx_thread);
}

void gfx_close()
{
	osStopTimer(&rcp_timer);
	osStopThread(&gfx_thread);
}

/**
 * @brief Self-destruct function, triggered if the scheduler has detected a fault.
 *
 * The main purpose of this is to avoid issues with any new frames being sent and
 * subsequently overwriting that currently being used by the fault screen.
 *
 * This is especially useful during audio thread testing, and will be removed or
 * at least reduced once audio is eventually in a working state.
 */
/*static bool gfx_fault_handler()
{
	if (scheduler->crash)
	{
		int i;
		osYieldThread();
		gfx_close();
		for (i = 0; i < MSG_SIZE; i++)
		{
			msg_buffer[i].func = NULL;
			msg_buffer[i].usecpu = TRUE;
			msg_buffer[i].swapbuffer = FALSE;
			msg_buffer[i].used = TRUE;
		}
		for (;;) {;}
		return TRUE;
	}

	return FALSE;
}*/

static void gfx_threadfunc(void *arg)
{
	fb_target = NULL;
	fb_last = NULL;

	osCreateMesgQueue(&msg_queue_gfx, msg_gfx, active_framebuffers);
	osCreateMesgQueue(&msg_queue_vsync, &msg_vsync, 1);
	osCreateMesgQueue(&msg_queue_rdp, &msg_rdp, 1);
	osSetEventMesg(OS_EVENT_DP, &msg_queue_rdp, NULL);
	msg_index = 0;

	while (1)
	{
		RenderMessage* l_msgp;

		// Wait for a graphics message to arrive
		debug_printf("[GFX] Waiting for render request\n");
		osRecvMesg(&msg_queue_gfx, (OSMesg*)&l_msgp, OS_MESG_BLOCK);

		// Ignore if the framebuffer resolution is being changed
		if (scheduler->is_changing_res)
		{
			debug_printf("[GFX] Waiting until display resolution change is completed\n");
			while (scheduler->is_changing_res) {
		}
		}

		// We received a message, find an available framebuffer if we don't have one yet
		if (fb_target == NULL)
		{
			int i = 0;
			for (i = 0; i < active_framebuffers; i++)
			{
				FrameBuffer* fb_ptr = &framebuffers[i];
				if (fb_ptr->status == FB_FREE || (fb_ptr->status == FB_READY && fb_ptr != fb_last))
				{
					debug_printf("[GFX] Found framebuffer %d (address: %p)\n", fb_ptr->id, fb_ptr->address);
					fb_target = fb_ptr;
					fb_current = fb_target;
					break;
				}
			}

			// If none was found, drop this render request
			if (fb_target == NULL)
			{
				debug_printf("[GFX] No available framebuffer found, skipping...\n");
				continue;
			}
		}

		// Generate the display list for the scene
		l_msgp->used = TRUE;
		if (!l_msgp->usecpu)
		{
			gfx_render(fb_target, l_msgp->func);
			fb_target->status = FB_BUSY;

			// Wait for the render task to finish
			(void)osRecvMesg(&msg_queue_rdp, NULL, OS_MESG_BLOCK);
			debug_printf("[GFX] Finished render task at framebuffer %d\n", fb_target->id);
			scheduler->task_gfx = NULL;

			#if DEBUG_MODE
				osStopTimer(&rcp_timer);
			#endif
		}
		else
		{
			debug_printf("[GFX] Sending render task to CPU\n");
			if (l_msgp->func != NULL)
			{
				l_msgp->func();
			}
			osWritebackDCacheAll();
		}

		// If we're not meant to swap the framebuffer yet, then stop here
		// The next loop should reuse this framebuffer if needed
		if (l_msgp->swapbuffer)
		{
			debug_printf("[GFX] Framebuffer %d ready, will be swapped\n", fb_target->id);
			fb_target->status = FB_READY;
			fb_last = fb_target;
			fb_target = NULL;
		}
		l_msgp->used = FALSE;
	}
}

static void gfx_render(FrameBuffer *fb, void (*func)())
{
	RenderTask l_task;

	// Initialize the render task
	l_task.fb = fb->address;
	l_task.dl = fb->dl;
	l_task.func = func;

	// Build the display list
	rcp_start(&l_task);
	if (l_task.func != NULL)
	{
		l_task.func();
	}
	rcp_finish(&l_task);

	// If the framebuffer is still in use by the VI (the switch takes time), then wait for it to become available
	while (osViGetCurrentFramebuffer() == l_task.fb)
	{
		debug_printf("[GFX] Waiting for Vsync pending display task\n");
		scheduler->gfx_notify = &msg_queue_vsync;
		osRecvMesg(&msg_queue_vsync, NULL, OS_MESG_BLOCK);
	}

	// Let the scheduler know the RCP is going to be busy
	scheduler->task_gfx = l_task.task;

	#if DEBUG_MODE
		osSetTimer(&rcp_timer, OS_USEC_TO_CYCLES(SEC_TO_USEC(3)), 0, &scheduler->queue, (OSMesg)SC_MSG_RCPDEAD);
	#endif
	debug_printf("[GFX] Beginning render task at framebuffer %d\n", fb->id);
	osSpTaskStart(scheduler->task_gfx);
}

void gfx_request_render(void (*func)(), bool usecpu, bool swapbuffer)
{
	if (MSG_SIZE > 1)
	{
		int starting_index = -1;
		while (msg_buffer[msg_index].used == TRUE)
		{
			if (msg_index == starting_index && starting_index >= 0)
			{
				debug_printf("[GFX] All render task requests full, skipping...\n");
				return;
			}

			debug_printf("[GFX] Render task request slot %d is occupied\n", msg_index);
			if (starting_index < 0)
				starting_index = msg_index;
			msg_index++;
			if (msg_index >= MSG_SIZE) { msg_index = 0; }
		}
	}
	else if (msg_buffer[0].used == TRUE)
	{
		debug_printf("[GFX] Render task request full, skipping...\n");
		return;
	}

	if (scheduler->is_changing_res == FALSE)
	{
		RenderMessage* l_msgp = &msg_buffer[msg_index];
		l_msgp->func = func;
		l_msgp->usecpu = usecpu;
		l_msgp->swapbuffer = swapbuffer;

		debug_printf("[GFX] Requesting render task at slot %d\n", msg_index);
		osSendMesg(&msg_queue_gfx, (OSMesg)l_msgp, OS_MESG_BLOCK);

		msg_index = (msg_index + 1) % MSG_SIZE;
	}
}

static void gfx_create_fb(void *address)
{
	// Clear the buffer
	bzero(address, CFB_SIZE);

	framebuffers[active_framebuffers].id = active_framebuffers;
	framebuffers[active_framebuffers].address = address;
	framebuffers[active_framebuffers].size = CFB_SIZE;
	framebuffers[active_framebuffers].dl = glist[active_framebuffers];
	framebuffers[active_framebuffers].status = FB_FREE;

	active_framebuffers++;
}


/* =================================================== *
 *              INTERACTION WITH SCHEDULER             *
 * =================================================== */

bool is_framebuffer_ready()
{
	return fb_last != NULL;
}

int num_active_framebuffers()
{
	return active_framebuffers;
}

FrameBuffer* return_ready_framebuffer()
{
	int i;
	FrameBuffer* fb_cons = fb_last;

	// Mark the consumed framebuffer as displaying
	fb_cons->status = FB_SHOWING;

	// See if we have another framebuffer marked as ready
	fb_last = NULL;
	for (i = 0; i < active_framebuffers; i++)
	{
		if (framebuffers[i].status == FB_READY)
		{
			fb_last = &framebuffers[i];
			break;
		}
	}

	// Return the consumed framebuffer
	return fb_cons;
}