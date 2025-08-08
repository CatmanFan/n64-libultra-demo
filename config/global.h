#ifndef __CONFIG_H__
#define __CONFIG_H__

/**
 * Configure thread IDs.
 *
 * It is HIGHLY recommended to not conflict any of the IDs with those used by the
 * USB+Debug Library (13, 14, 15) or with those used by the target decomp!
 */
#define ID_IDLE             5
#define ID_MAIN             6
#define ID_FAULT            7
#define ID_SCHEDULER        8
#define ID_CONTROLLER       9
#define ID_GFX              10
#define ID_AUDIO            11

/**
 * Configure thread priority values.
 */
#define PR_IDLE             10
#define PR_MAIN             10
#define PR_FAULT            OS_PRIORITY_APPMAX
#define PR_SCHEDULER        80
#define PR_CONTROLLER       15
#define PR_GFX              20
#define PR_AUDIO            75

/**
 * Emulator check.
 */
#define IS_RUNNING_ON_EMULATOR (IO_READ(DPC_PIPEBUSY_REG) == 0)

/**
 * Debug mode check.
 */
#if DEBUG_MODE
	#define osSyncPrintf osSyncPrintf
#else
	#define osSyncPrintf if (0) osSyncPrintf
#endif

#endif