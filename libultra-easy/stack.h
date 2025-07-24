#ifndef __STACK_H__
#define __STACK_H__

	extern u64 fifo_buffer[] __attribute__((aligned(16)));
	extern u64 dram_stack[] __attribute__((aligned(16)));
	extern u64 yield_buffer[] __attribute__((aligned(16)));

#endif