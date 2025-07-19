#include <ultra64.h>
#include <PR/ramrom.h>
#include "libultra-easy.h"
#include "strings.h"
#include "stages.h"

/* ============= PROTOTYPES ============= */

// Thread functions
static void idle(void *);
static void main(void *);
extern void crash_loop(void *);

// Stacks
u64 boot_stack[STACK_SIZE_BOOT / sizeof(u64)];

OSThread idle_thread;
OSThread main_thread;
OSThread crash_thread;

volatile Scheduler* scheduler;

/* ============== MESSAGES ============== */

// Crash screen
OSMesg msg_crash;
OSMesgQueue msgQ_crash;

/* =========== MAIN FUNCTIONS =========== */

// Main N64 entry point.
void boot(void* arg)
{
	osInitialize();
	osAiSetFrequency(AUDIO_BITRATE);
	osCreateThread(&idle_thread, ID_IDLE, idle, arg, &idle_stack[STACK_SIZE_IDLE / sizeof(u64)], PR_IDLE);
	osStartThread(&idle_thread);
}

static void idle(void *arg)
{
	// Initialize main thread
	osCreateThread(&main_thread, ID_MAIN, main, arg, &main_stack[STACK_SIZE_MAIN / sizeof(u64)], PR_MAIN);
	osStartThread(&main_thread);

	// Initialize crash screen queue and thread
    osCreateMesgQueue(&msgQ_crash, &msg_crash, 1);
	osCreateThread(&crash_thread, ID_CRASH, crash_loop, NULL, &crash_stack[STACK_SIZE_CRASH / sizeof(u64)], PR_CRASH);
	osStartThread(&crash_thread);

	// Relinquish CPU
	osSetThreadPri(NULL, 0);

	// Enter a permanent loop, so as to remain the only thread running if all others are absent.
	for (;;) { ; }
}

static void main(void *arg)
{
	// Initialize Pi Manager/DMA
	init_reader();
	load_all_segments();

	// Initialize debugger
	debug_init();

	// Initialize the remainder of the libraries
	// ======================================================
	debug_printf("[Boot] Initializing scheduler and display\n");
	scheduler = init_scheduler();	// Scheduler and video
	debug_printf("[Boot] Initializing time counter\n");
	time_init();					// Time engine
	debug_printf("[Boot] Setting default language\n");
	language = 0;
	change_language();				// Language

	debug_printf("[Boot] Initializing graphics\n");
	init_gfx(scheduler);			// Graphics
	debug_printf("[Boot] Initializing audio\n");
	init_audio();					// Audio player
	debug_printf("[Boot] Initializing controller\n");
	init_controller();				// Controller/SI
	// ======================================================

	// Initialize boot stage
	debug_printf("[Boot] Initializing game engine\n");
	change_stage(-1);

	// Start permanent loop
	while (1)
	{
		while (!change_stage_needed())
		{
			read_controller();
			stages[current_stage_index].update();
			gfx_request_render(stages[current_stage_index].render, FALSE, TRUE);
			time_update();
		}

		debug_printf("[Engine] Stage changed by internal function\n");
		change_stage(target_stage);
	}
}