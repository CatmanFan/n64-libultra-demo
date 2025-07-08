#ifndef __CONFIG_UNFLOADER_H__
#define __CONFIG_UNFLOADER_H__

/**
 * Debug mode parameters and macros (mainly used for enabling UNFLoader).
 */
// #define UNFLOADER
 
#include "usb/debug.h"
#include "usb/usb.h"

#ifdef UNFLOADER
	#define debug_initialize debug_initialize
	#define debug_printf debug_printf
#else
	#define debug_initialize if (FALSE) debug_initialize
	#define debug_printf if (FALSE) debug_printf
#endif

#endif