#include <ultra64.h>
#include <math.h>

#include "config/global.h"
#include "config/video.h"
#include "config/usb.h"

#include "libultra-easy/types.h"
#include "libultra-easy/crash.h"
#include "libultra-easy/display.h"
#include "libultra-easy/gfx.h"
#include "libultra-easy/rcp.h"
#include "libultra-easy/scheduler.h"
#include "libultra-easy/stack.h"
#include "libultra-easy/time.h"

/* =================================================== *
 *                       MACROS                        *
 * =================================================== */

#ifdef VIDEO_32BIT
	#define FB_DEPTH u32
#else
	#define FB_DEPTH u16
#endif

#if (CFB_COUNT > 3 || CFB_COUNT <= 1)
	#error Invalid framebuffer count
#endif

#define MSG_SIZE (CFB_COUNT + 1)

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

static OSThread		gfx_thread;
static OSTimer      rcp_timer;
static OSMesg		msg_gfx[CFB_COUNT];
static OSMesgQueue	msg_queue_gfx;
static OSMesg		msg_vsync;
static OSMesgQueue	msg_queue_vsync;
static OSMesg		msg_rdp;
static OSMesgQueue	msg_queue_rdp;

static void gfx_loop(void *arg);
static void gfx_render(FrameBuffer *fb, void (*func)());
static void gfx_create_fb(int index, void *address);

static volatile Scheduler *scheduler;

/* =================================================== *
 *              GRAPHICS TASK FUNCTIONING              *
 * =================================================== */

void init_gfx(volatile Scheduler *sc)
{
	// Create framebuffers
	gfx_create_fb(0, (FB_DEPTH*)(RAMBANK_6 + RAMBANK_SIZE - (SCREEN_W_HD*SCREEN_H_HD*sizeof(FB_DEPTH))));
	if (CFB_COUNT == 2)
		gfx_create_fb(1, (FB_DEPTH*)(RAMBANK_7 + RAMBANK_SIZE - (SCREEN_W_HD*SCREEN_H_HD*sizeof(FB_DEPTH))));
	if (CFB_COUNT == 3)
		gfx_create_fb(2, (FB_DEPTH*)(RAMBANK_8 + RAMBANK_SIZE - (SCREEN_W_HD*SCREEN_H_HD*sizeof(FB_DEPTH))));

	// Set scheduler
	scheduler = sc;

	// Start the graphics thread
	osCreateThread(&gfx_thread, ID_GFX, gfx_loop, NULL, &gfx_stack[STACK_SIZE_GFX / sizeof(u64)], PR_GFX);
	osStartThread(&gfx_thread);
}

static void gfx_loop(void *arg)
{
	fb_target = NULL;
	fb_last = NULL;

	osCreateMesgQueue(&msg_queue_gfx, msg_gfx, CFB_COUNT);
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
			while (scheduler->is_changing_res) { ; }
		}

		// We received a message, find an available framebuffer if we don't have one yet
		if (fb_target == NULL)
		{
			int i = 0;
			for (i = 0; i < CFB_COUNT; i++)
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
		// l_msgp->used = TRUE;
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
				l_msgp->func();
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
		// l_msgp->used = FALSE;
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
		l_task.func();
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
	osSpTaskStart(l_task.task);
}

void gfx_request_render(void (*func)(), bool usecpu, bool swapbuffer)
{
	/* if (MSG_SIZE > 1)
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
	} */

	if (scheduler->is_changing_res == FALSE)
	{
		RenderMessage* l_msgp = &msg_buffer[msg_index];
		l_msgp->func = func;
		l_msgp->usecpu = usecpu;
		l_msgp->swapbuffer = swapbuffer;

		debug_printf("[GFX] Requesting render task at slot %d\n", msg_index);
		osSendMesg(&msg_queue_gfx, (OSMesg)l_msgp, OS_MESG_BLOCK);
		msg_index++;
		if (msg_index >= MSG_SIZE) { msg_index = 0; }
	}
}

static void gfx_create_fb(int index, void *address)
{
	framebuffers[index].id = index;
	framebuffers[index].address = address;
	framebuffers[index].dl = glist[index];
	framebuffers[index].status = FB_FREE;
}


/* =================================================== *
 *              INTERACTION WITH SCHEDULER             *
 * =================================================== */

bool is_framebuffer_ready()
{
	return fb_last != NULL;
}

FrameBuffer* return_ready_framebuffer()
{
	int i;
	FrameBuffer* fb_cons = fb_last;

	// Mark the consumed framebuffer as displaying
	fb_cons->status = FB_SHOWING;

	// See if we have another framebuffer marked as ready
	fb_last = NULL;
	for (i = 0; i < CFB_COUNT; i++)
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

FrameBuffer* current_framebuffer()
{
	return fb_current != NULL ? fb_current : fb_last;
}