#ifndef __CONFIG_STACK_H__
#define __CONFIG_STACK_H__

#include "video.h"

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

// [Audio heap]
#define AUDIO_HEAP_ADDR			RAMBANK_2		// 0x801A8000: Total size of four framebuffers - audio heap size
												// For reference, stack section ends at 0x80132800
#define AUDIO_HEAP_SIZE			0x80000

// [Starting address]
#define REAL_STACK(n)			(u64 *)(STACK_ADDR_##n## + STACK_SIZE_##n##/sizeof(u64))
#define STACK_ADDR_BOOT			(AUDIO_HEAP_ADDR + AUDIO_HEAP_SIZE)
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

// [RCP framebuffers]
#ifdef VIDEO_32BIT
	#define ZBUFFER_ADDR		RAMBANK_8 + RAMBANK_SIZE - CFB_SIZE
	#define CFB_SIZE			(SCREEN_W_HD*SCREEN_H_HD*4)
#else
	#define ZBUFFER_ADDR		RAMBANK_4 + RAMBANK_SIZE - CFB_SIZE
	#define CFB_SIZE			(SCREEN_W_HD*SCREEN_H_HD*2)
#endif

#define CFB1_ADDR				ZBUFFER_ADDR - (CFB_SIZE*1)
#define CFB2_ADDR				ZBUFFER_ADDR - (CFB_SIZE*2)
#define CFB3_ADDR				ZBUFFER_ADDR - (CFB_SIZE*3)

/* **************************************************** */

// Check that all stack addresses are aligned
#if (STACK_ADDR_BOOT%8 != 0)
	#error Boot stack is not 64-bit aligned
#endif
#if (STACK_ADDR_IDLE%8 != 0)
	#error Idle stack is not 64-bit aligned
#endif
#if (STACK_ADDR_MAIN%8 != 0)
	#error Main stack is not 64-bit aligned
#endif
#if (STACK_ADDR_FAULT%8 != 0)
	#error Fault stack is not 64-bit aligned
#endif
#if (STACK_ADDR_SCHEDULER%8 != 0)
	#error Scheduler stack is not 64-bit aligned
#endif
#if (STACK_ADDR_CONTROLLER%8 != 0)
	#error Controller stack is not 64-bit aligned
#endif
#if (STACK_ADDR_GFX%8 != 0)
	#error GFX stack is not 64-bit aligned
#endif
#if (STACK_ADDR_AUDIO%8 != 0)
	#error Audio stack is not 64-bit aligned
#endif
#if (STACK_ADDR_DRAM%16 != 0)
	#error DRAM stack is not 16-bit aligned
#endif
#if (STACK_ADDR_RDPFIFO%16 != 0)
	#error RDP FIFO stack is not 16-bit aligned
#endif
#if (STACK_ADDR_YIELD%16 != 0)
	#error GFX yield stack is not 16-bit aligned
#endif
#if (AUDIO_HEAP_ADDR%16 != 0)
	#error Audio heap is not 16-bit aligned
#endif

// Check that all stack sizes are valid
#if STACK_SIZE < OS_MIN_STACKSIZE
	#error General stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_BOOT < OS_MIN_STACKSIZE
	#error Boot stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_IDLE < OS_MIN_STACKSIZE
	#error Idle stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_MAIN < OS_MIN_STACKSIZE
	#error Main stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_FAULT < OS_MIN_STACKSIZE
	#error Fault stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_SCHEDULER < OS_MIN_STACKSIZE
	#error Scheduler stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_CONTROLLER < OS_MIN_STACKSIZE
	#error Controller stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_GFX < OS_MIN_STACKSIZE
	#error GFX stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_AUDIO < OS_MIN_STACKSIZE
	#error Audio stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_DRAM < SP_DRAM_STACK_SIZE8
	#error DRAM stack size is smaller than SP_DRAM_STACK_SIZE8
#endif
#if STACK_SIZE_RDPFIFO < 0x100
	#error RDP FIFO stack size is smaller than 256 bytes
#endif
#if STACK_SIZE_YIELD < OS_YIELD_DATA_SIZE
	#error GFX yield stack size is smaller than OS_YIELD_DATA_SIZE
#endif

#endif