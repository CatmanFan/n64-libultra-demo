#ifndef __CRASH_H__
#define __CRASH_H__

extern void crash();
extern void crash_msg(char *msg);

#ifdef NDEBUG
#define my_assert(EX,M) if (0) { ; }
#else
#define my_assert(EX,M) if ((EX) == 0) { crash_msg(M); }
#endif

#endif