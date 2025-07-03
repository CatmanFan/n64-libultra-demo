#ifndef __CONFIG_DEBUG_H__
#define __CONFIG_DEBUG_H__

#if DEBUG_MODE
	#define LOG(x)			osSyncPrintf(x)
#else
	#define LOG(x)			if (FALSE) { ; }
#endif

#endif