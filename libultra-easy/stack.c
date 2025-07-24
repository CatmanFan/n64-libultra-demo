#include <ultra64.h>
#include "config/global.h"

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
#if (STACK_ADDR_GFX%8 != 0)
	#error Graphics stack is not 64-bit aligned
#endif
#if (STACK_ADDR_AUDIO%8 != 0)
	#error Audio stack is not 64-bit aligned
#endif
#if (STACK_ADDR_RDPFIFO%16 != 0)
	#error RDP FIFO stack is not 16-bit aligned
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
#if STACK_SIZE_GFX < OS_MIN_STACKSIZE
	#error Graphics stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_AUDIO < OS_MIN_STACKSIZE
	#error Audio stack size is smaller than OS_MIN_STACKSIZE
#endif
#if STACK_SIZE_RDPFIFO < 0x100
	#error RDP FIFO stack size is smaller than 256 bytes
#endif

// Display list processing results buffer
u64 fifo_buffer[STACK_SIZE_RDPFIFO] __attribute__((aligned(16)));
// Microcode matrix stack
u64 dram_stack[SP_DRAM_STACK_SIZE8] __attribute__((aligned(16)));
// Audio buffer
u64 yield_buffer[OS_YIELD_DATA_SIZE] __attribute__((aligned(16)));