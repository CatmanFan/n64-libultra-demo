#ifndef __CONFIG_VIDEO_H__
#define __CONFIG_VIDEO_H__

/**
 * Default video mode to use.
 *
 * osTvType:   Automatic
 * OS_TV_NTSC: Force NTSC      (Americas/Japan)
 * OS_TV_PAL:  Force PAL mode  (Europe/Australia)
 * OS_TV_MPAL: Force MPAL mode (Brazil)
 */
#define VIDEO_TYPE			OS_TV_NTSC

/**
 * Enables full-resolution PAL mode (320x288 / 640x576).
 */
#define VIDEO_FPAL

/**
 * Enables 32-bit screen depth (instead of 16-bit).
 */
// #define VIDEO_32BIT

/**
 * Renders in high-resolution mode.
 */
#define VIDEO_HIGHRES

/**
 * Screen resolution dimensions.
 */
#ifdef VIDEO_HIGHRES
	#define SCREEN_W		640
	#define SCREEN_H		480
#else
	#define SCREEN_W		320
	#define SCREEN_H		240
#endif

#endif