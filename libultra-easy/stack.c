#include <ultra64.h>
#include <PR/sched.h>
#include "config/global.h"

// Check that all stack sizes are valid before declaring the stacks
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
#if STACK_SIZE_CRASH < OS_MIN_STACKSIZE
	#error Fault stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_SCHEDULER < OS_MIN_STACKSIZE
	#error Scheduler stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_GFX < OS_MIN_STACKSIZE
	#error Graphics stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_AUDIO < OS_MIN_STACKSIZE
	#error Audio stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_RDPFIFO < 0x100
	#error RDP FIFO stack size is smaller than 256 bytes
#endif

volatile u64 idle_stack[STACK_SIZE_IDLE / sizeof(u64)] __attribute__((aligned(16)));
volatile u64 main_stack[STACK_SIZE_MAIN / sizeof(u64)] __attribute__((aligned(16)));
volatile u64 crash_stack[STACK_SIZE_CRASH / sizeof(u64)] __attribute__((aligned(16)));
volatile u64 scheduler_stack[OS_SC_STACKSIZE / sizeof(u64)] __attribute__((aligned(16)));
volatile u64 gfx_stack[STACK_SIZE_GFX / sizeof(u64)] __attribute__((aligned(16)));
volatile u64 audio_stack[STACK_SIZE_AUDIO / sizeof(u64)] __attribute__((aligned(16)));

// Display list processing results buffer
volatile u64 fifo_buffer[STACK_SIZE_RDPFIFO] __attribute__((aligned(16)));
// Microcode matrix stack
volatile u64 dram_stack[SP_DRAM_STACK_SIZE8] __attribute__((aligned(16)));
// Audio buffer
volatile u64 yield_buffer[OS_YIELD_DATA_SIZE] __attribute__((aligned(16)));