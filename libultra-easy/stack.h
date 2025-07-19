#ifndef __STACK_H__
#define __STACK_H__

	extern volatile u64 idle_stack[] __attribute__((aligned(16)));
	extern volatile u64 main_stack[] __attribute__((aligned(16)));
	extern volatile u64 crash_stack[] __attribute__((aligned(16)));
	extern volatile u64 scheduler_stack[] __attribute__((aligned(16)));
	extern volatile u64 gfx_stack[] __attribute__((aligned(16)));
	extern volatile u64 audio_stack[] __attribute__((aligned(16)));
	extern volatile u64 fifo_buffer[] __attribute__((aligned(16)));
	extern volatile u64 dram_stack[] __attribute__((aligned(16)));
	extern volatile u64 yield_buffer[] __attribute__((aligned(16)));

#endif