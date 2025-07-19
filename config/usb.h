#ifndef __CONFIG_UNFLOADER_H__
#define __CONFIG_UNFLOADER_H__

	/**
	 * Debug mode parameters and macros (mainly used for enabling UNFLoader).
	 */
	// #define UNFLOADER
	 
	#include "usb/debug.h"
	#include "usb/usb.h"

	#ifdef UNFLOADER
		#define debug_init		debug_initialize
		#define debug_printf	debug_printf
	#else
		#define debug_init		if (0) debug_initialize
		#define debug_printf	if (0) debug_printf
	#endif

#endif