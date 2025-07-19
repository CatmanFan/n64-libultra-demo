/**
 * @file libdragon.h
 * @brief Linker for LibUltra wrappers
 * @ingroup libultra-easy
 */
#ifndef __LIBULTRA_EASY_H__
#define __LIBULTRA_EASY_H__

	/**
	 * @defgroup libultra-easy libultra-easy
	 * @brief Wrapper for LibUltra SDK functions
	 */

	#include "libultra-easy/types.h"

	/* General */
	#include "libultra-easy/audio.h"
	#include "libultra-easy/console.h"
	#include "libultra-easy/controller.h"
	#include "libultra-easy/display.h"
	#include "libultra-easy/gfx.h"
	#include "libultra-easy/gfx_2d.h"
	#include "libultra-easy/gfx_3d.h"

	/* Hardware */
	#include "libultra-easy/fs.h"
	#include "libultra-easy/stack.h"
	#include "libultra-easy/scheduler.h"
	#include "libultra-easy/time.h"

	/* Crash screen */
	#include "libultra-easy/crash.h"

	/* User-defined configuration */
	#include "config/global.h"
	#include "config/audio.h"
	#include "config/video.h"
	#include "config/usb.h"

#endif