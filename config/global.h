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
#define ID_SCHEDULER        7
#define ID_GFX              8
#define ID_AUDIO            9
#define ID_FAULT            12

/**
 * Configure thread priority values.
 */
#define PR_IDLE             10
#define PR_MAIN             10
#define PR_SCHEDULER        120
#define PR_GFX              20
#define PR_AUDIO            25
#define PR_FAULT            OS_PRIORITY_APPMAX

/**
 * Configure stack sizes.
 *
 * Stack sizes cannot be smaller than OS_MIN_STACKSIZE, except for RDP/FIFO,
 * which cannot be smaller than 256 bytes (0x100).
 */
#define STACK_SIZE			 0x2000
#define STACK_SIZE_BOOT		 0x400
#define STACK_SIZE_IDLE		 0x400
#define STACK_SIZE_MAIN		 STACK_SIZE
#define STACK_SIZE_FAULT	 0x1000
#define STACK_SIZE_SCHEDULER 0x2000
#define STACK_SIZE_GFX		 0x2000
#define STACK_SIZE_AUDIO	 0x2000
#define STACK_SIZE_RDPFIFO	 0x2000

/**
 * Configure RAM and stack addresses.
 */
// [RAM bank addresses]
#define RAMBANK_START			0x80000000
#define RAMBANK_SIZE			0x00100000
#define RAMBANK_1				(RAMBANK_START + RAMBANK_SIZE*0)
#define RAMBANK_2				(RAMBANK_START + RAMBANK_SIZE*1)
#define RAMBANK_3				(RAMBANK_START + RAMBANK_SIZE*2)
#define RAMBANK_4				(RAMBANK_START + RAMBANK_SIZE*3)
#define RAMBANK_5				(RAMBANK_START + RAMBANK_SIZE*4)
#define RAMBANK_6				(RAMBANK_START + RAMBANK_SIZE*5)
#define RAMBANK_7				(RAMBANK_START + RAMBANK_SIZE*6)
#define RAMBANK_8				(RAMBANK_START + RAMBANK_SIZE*7)

// [Starting address]
#define REAL_STACK(n)			(u64 *)(STACK_ADDR_##n## + STACK_SIZE_##n##/sizeof(u64))
#define STACK_ADDR_BOOT			RAMBANK_2
#define STACK_ADDR_IDLE			(STACK_ADDR_BOOT + STACK_SIZE_BOOT)
#define STACK_ADDR_MAIN			(STACK_ADDR_IDLE + STACK_SIZE_IDLE)
#define STACK_ADDR_FAULT		(STACK_ADDR_MAIN + STACK_SIZE_MAIN)
#define STACK_ADDR_SCHEDULER	(STACK_ADDR_FAULT + STACK_SIZE_FAULT)
#define STACK_ADDR_GFX			(STACK_ADDR_SCHEDULER + STACK_SIZE_SCHEDULER)
#define STACK_ADDR_AUDIO		(STACK_ADDR_GFX + STACK_SIZE_GFX)
#define STACK_ADDR_DRAM			(STACK_ADDR_AUDIO + STACK_SIZE_AUDIO)
#define STACK_ADDR_DRAM			(STACK_ADDR_AUDIO + STACK_SIZE_AUDIO)
#define AUDIO_START_ADDR		RAMBANK_7

/**
 * Configure message size limits.
 */
#define NUM_PI_MSGS         16

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