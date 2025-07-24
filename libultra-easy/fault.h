#ifndef __FAULT_H__
#define __FAULT_H__

	void init_fault();
	void crash();
	void crash_msg(char *msg);

	#ifdef NDEBUG
	#define my_assert(EX,M) if (0) { ; }
	#else
	#define my_assert(EX,M) if ((EX) == 0) { crash_msg(M); }
	#endif

#endif