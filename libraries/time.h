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
	#define SECONDS_PER_CYCLE    0.00000002133F
	#define MSEC_TO_CYCLES(a)    (OS_USEC_TO_CYCLES(SEC_TO_MSEC(a)))
	#define SEC_TO_CYCLES(a)     (OS_USEC_TO_CYCLES(SEC_TO_USEC(a)))
	#define CYCLES_TO_SEC(a)     ((s32)(a) * SECONDS_PER_CYCLE)

	#define WAIT(a) { \
		OSTime wait_time = osGetTime(); \
		while (osGetTime() < wait_time + OS_MSEC_TO_CYCLES(a)) { ; } \
	}
	
	#define LAG() { \
		int lag_value; \
		for (lag_value = 0; lag_value < 700000; lag_value++) \
			; \
	}

#endif