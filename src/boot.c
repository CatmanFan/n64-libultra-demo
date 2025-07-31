#include <ultra64.h>
#include <PR/ramrom.h>
#include "libultra-easy.h"
#include "strings.h"
#include "stages.h"

/* ============= PROTOTYPES ============= */

// Thread functions
static void idle(void *) __attribute__ ((noreturn));
static void main(void *) __attribute__ ((noreturn));

OSThread idle_thread;
OSThread main_thread;

Scheduler* scheduler;

/* =========== MAIN FUNCTIONS =========== */

// Main N64 entry point.
void boot(void* arg)
{
	osInitialize();
	// osAiSetFrequency(AUDIO_BITRATE);
	osCreateThread(&idle_thread, ID_IDLE, idle, arg, REAL_STACK(IDLE), PR_IDLE);
	osStartThread(&idle_thread);
}

static void idle(void *arg)
{
	// Initialize main thread
	osCreateThread(&main_thread, ID_MAIN, main, arg, REAL_STACK(MAIN), PR_MAIN);
	osStartThread(&main_thread);

	// Relinquish CPU
	osSetThreadPri(NULL, 0);

	// Enter a permanent loop, so as to remain the only thread running if all others are absent.
	for (;;) { ; }
}

static void main(void *arg)
{
	extern void change_stage(s32 id);
	extern bool change_stage_needed();
	extern s32 target_stage_id;
	extern int current_stage_index;

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
	init_fault();

	debug_printf("[Boot] Initializing graphics\n");
	init_gfx(scheduler);			// Graphics
	debug_printf("[Boot] Initializing audio\n");
	init_audio(scheduler);			// Audio player
	debug_printf("[Boot] Initializing controller\n");
	init_controller();				// Controller/SI
	// ======================================================

	// Initialize boot stage
	debug_printf("[Boot] Initializing game engine\n");
	change_stage(0);

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
		change_stage(target_stage_id);
	}
}