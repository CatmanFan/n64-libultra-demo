#ifndef __FAULT_H__
#define __FAULT_H__

	void init_fault();
	void crash();
	void crash_msg(const char *txt, ...);

	/*
	#define crash() { \
						// TLB exception on load/instruction fetch \
						long e1; \
						e1 = *(long *)1; \
						\
						// TLB exception on store \
						*(long *)2 = 2; \
					}

	#define crash_msg(t) { \
							extern void set_crash_msg(char *msg); \
							set_crash_msg(t); \
							crash(); \
						 }
	*/

	#ifdef NDEBUG
	#define my_assert(EX,M) if (0) { ; }
	#else
	#define my_assert(EX,M) if ((EX) == 0) { crash_msg(M); }
	#endif

#endif