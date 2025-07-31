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
#define PR_AUDIO            25

/**
 * Configure stack sizes.
 *
 * Stack sizes cannot be smaller than OS_MIN_STACKSIZE, except for the following:
 * - RDP/FIFO, which cannot be smaller than 256 bytes
 *   (0x100);
 * - DRAM, which cannot be smaller than SP_DRAM_STACK_SIZE8
 *   (1 kB);
 * - Yield buffer, which cannot be smaller than OS_YIELD_DATA_SIZE
 *   (0xC00 or 3 kB if using F3DEX, otherwise 0x900 or 2.25 kB).
 * It is recommended to leave the DRAM and yield stack sizes at their default values.
 */
#define STACK_SIZE				0x2000 // 8 kB
#define STACK_SIZE_BOOT			0x400
#define STACK_SIZE_IDLE			0x400
#define STACK_SIZE_MAIN			STACK_SIZE
#define STACK_SIZE_FAULT		STACK_SIZE
#define STACK_SIZE_SCHEDULER	0x2000
#define STACK_SIZE_CONTROLLER	0x2000
#define STACK_SIZE_GFX			0x2000
#define STACK_SIZE_AUDIO		0x2000
#define STACK_SIZE_DRAM			0x400
#define STACK_SIZE_RDPFIFO		0x1000
#define STACK_SIZE_YIELD		OS_YIELD_DATA_SIZE

/**
 * Configure RAM and stack addresses.
 */
// [RAM bank addresses]
#define RAMBANK_START			0x80000000
#define RAMBANK_SIZE			0x00100000 // 1 MB
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
#define STACK_ADDR_CONTROLLER	(STACK_ADDR_SCHEDULER + STACK_SIZE_SCHEDULER)
#define STACK_ADDR_GFX			(STACK_ADDR_CONTROLLER + STACK_SIZE_CONTROLLER)
#define STACK_ADDR_AUDIO		(STACK_ADDR_GFX + STACK_SIZE_GFX)
#define STACK_ADDR_DRAM			(STACK_ADDR_AUDIO + STACK_SIZE_AUDIO)
#define STACK_ADDR_RDPFIFO		(STACK_ADDR_DRAM + STACK_SIZE_DRAM)
#define STACK_ADDR_YIELD		(STACK_ADDR_RDPFIFO + STACK_SIZE_RDPFIFO)

// [Audio heap]
#define AUDIO_HEAP_ADDR			RAMBANK_3
#define AUDIO_HEAP_SIZE			0x80000

/**
 * Enables audio playback.
 */
#define ENABLE_AUDIO

/**
 * Configure audio bitrate.
 */
#define AUDIO_BITRATE		44100

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