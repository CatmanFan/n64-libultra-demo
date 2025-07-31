#ifndef __TIME_H__
#define __TIME_H__

#define CURRENT_TIME	     osGetTime()

// Time <-> time conversion
#define SEC_TO_MSEC(a)       (((f64)a)*1000.0F)
#define SEC_TO_USEC(a)       (((f64)a)*1000000.0F)
#define MSEC_TO_USEC(a)      (((f64)a)*1000.0F)
#define USEC_TO_MSEC(a)      (((f64)a)/1000.0F)
#define USEC_TO_SEC(a)       (((f64)a)/1000000.0F)

// Time <-> OS cycles conversion
/* #if VIDEO_TYPE == OS_TV_PAL
	#define SECONDS_PER_CYCLE    0.000000092F
#else
	#define SECONDS_PER_CYCLE    0.00000002133F
#endif */
#define SECONDS_PER_CYCLE    0.00000002133F
#define MSEC_TO_CYCLES(a)    (OS_USEC_TO_CYCLES(SEC_TO_MSEC(a)))
#define SEC_TO_CYCLES(a)     (OS_USEC_TO_CYCLES(SEC_TO_USEC(a)))
#define CYCLES_TO_SEC(a)     ((s32)(a) * SECONDS_PER_CYCLE)

/**
 * @brief Initializes the timer subsystem.
 */
void time_init();

/**
 * @brief Updates the current time and advances by the specified delta frame.
 */
void time_update();

/**
 * @brief Resets time counter.
 */
void time_reset();

/**
 * @brief Returns the current time count based on number of incremented frames
 * divided by the frames per second value.
 * 
 * @return The time counter
 */
f64 time_current();

f64 time_system();

f64 time_delta();

/**
 * @brief Returns the global framerate setting.
 * (This can be either 60, 50, 30 or 25 depending on the TV and game configuration.)
 * 
 * @return A static framerate value
 */
int time_framerate();

int time_current_frame();

int fps();

/**
 * @brief Holds the CPU for a specified amount of time.
 *
 * @param[in] secs
 *            A float number specifing the amount in seconds.
 */
void wait(float secs);

/**
 * @brief Holds the CPU for a specified amount of time.
 *
 * @param[in] cycles
 *            A float number specifing the amount in cycles.
 */
void wait_cycles(u64 cycles);

/**
 * @brief Test lag function.
 */
void lag(int count);

#endif