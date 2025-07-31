#include <ultra64.h>
#include "libultra-easy.h"

// Framerate macros
#ifdef VIDEO_DOUBLE_FRAMERATE
	#define FRAMERATE_NTSC 60
	#define FRAMERATE_PAL  50
#else
	#define FRAMERATE_NTSC 30
	#define FRAMERATE_PAL  25
#endif

#if VIDEO_TYPE == OS_TV_PAL
	#define FRAMERATE FRAMERATE_PAL
#else
	#define FRAMERATE FRAMERATE_NTSC
#endif

// Delta time macros
#define TICKRATE 60
#define DELTA (1.0F / (f64)TICKRATE)

// OSTime variables
static OSTime os_delta_time;
static OSTime os_now_system_time;
static OSTime os_prev_time;
static OSTime os_now_time;
static OSTime os_diff_time;
static OSTime os_last_reset;

static int current_frame;

static void wait_cycles_s(u64 cycles, bool use_debug);

/* =================================================== *
 *                   FPS calculation                   *
 * =================================================== */

static int s_fps;
static int fps_frames;
static long double fps_time;

static void fps_update()
{
	fps_time += os_diff_time * SECONDS_PER_CYCLE;
	fps_frames++;

	// If frame time is longer or equal to a second, update FPS counter.
	if (fps_time >= 1.0) 
	{
		s_fps = fps_frames;
		fps_frames = 0;
		fps_time -= 1.0;
	}
}

int fps()
{
	return s_fps;
}

/* =================================================== *
 *                  Time calculation                   *
 * =================================================== */

void time_init()
{
	os_now_system_time = 0;

	os_prev_time = 0;
	os_now_time = 0;
	os_diff_time = 0;
	os_last_reset = 0;

	current_frame = 0;
	os_delta_time = OS_USEC_TO_CYCLES(SEC_TO_USEC(DELTA));

	s_fps = 0;
	fps_frames = 0;
	fps_time = 0;
}

void time_reset()
{
	os_last_reset = osGetTime();
	os_prev_time = 0;
	os_now_time = 0;
	os_diff_time = 0;
}

void time_update()
{
    os_now_system_time = osGetTime();
    os_now_time = os_now_system_time - os_last_reset;
	os_diff_time = os_now_system_time - os_prev_time;
	os_prev_time = os_now_system_time;

	fps_update();

	wait_cycles_s(os_delta_time, FALSE);
	current_frame++;
}

/* =================================================== *
 *                       HELPERS                       *
 * =================================================== */

static bool debug = FALSE;

void wait(float secs)
{
	if (!debug)
	{
		debug_printf("[Engine] Waiting for %0.2f seconds\n", secs);
		debug = TRUE;
	}

	wait_cycles_s(OS_USEC_TO_CYCLES(SEC_TO_USEC(secs)), TRUE);
}

void wait_cycles(u64 cycles)
{
	wait_cycles_s(cycles, TRUE);
}

static void wait_cycles_s(u64 cycles, bool use_debug)
{
	bool use_timer = TRUE;

	if (!debug && use_debug)
	{
		if (cycles == 1)
			debug_printf("[Engine] Waiting for %d cycle\n", cycles);
		else
			debug_printf("[Engine] Waiting for %d cycles\n", cycles);

		debug = TRUE;
	}

	if (use_timer)
	{
		OSTimer timer;
		OSMesg timer_buffer;
		OSMesgQueue timer_queue;

		osCreateMesgQueue(&timer_queue, &timer_buffer, 1);
		osSetTimer(&timer, (OSTime)cycles, 0, &timer_queue, 0);
		(void) osRecvMesg(&timer_queue, NULL, OS_MESG_BLOCK);
	}
	else
	{
		OSTime marker = osGetTime();

		// Temporary halt
		while (osGetTime() < marker + cycles)
			{ ; }
	}

	if (use_debug)
		debug = FALSE;
}

void lag(int count)
{
	int lag_value;
	for (lag_value = 0; lag_value < count * 100000; lag_value++)
		{ ; }
}

// ----------------------------------------------------

f64 time_current()
{
	return USEC_TO_SEC(OS_CYCLES_TO_USEC(os_now_time));
}

f64 time_system()
{
	return USEC_TO_SEC(OS_CYCLES_TO_USEC(os_now_system_time));
}

f64 time_delta()
{
	return USEC_TO_SEC(OS_CYCLES_TO_USEC(os_delta_time));
}

int time_framerate()
{
	return FRAMERATE;
}

int time_current_frame()
{
	return current_frame;
}