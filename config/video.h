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
#define VIDEO_TYPE osTvType

/**
 * Enables full-resolution PAL mode (320x288 / 640x576).
 */
#define VIDEO_FPAL

/**
 * Permanently sets the screen depth to 32-bit (instead of 16-bit).
 * WARNING: Enabling this breaks dynamic screen resolution!
 */
// #define VIDEO_32BIT

/**
 * Sets the default resolution to high-res dimensions.
 */
// #define VIDEO_HIGHRES

/**
 * Screen resolution dimensions.
 */
// [High-resolution]
#define SCREEN_W_HD 640
#define SCREEN_H_HD 480
// [Standard]
#define SCREEN_W_SD 320
#define SCREEN_H_SD 240

/**
 * Configure color framebuffer count and size.
 */
#define CFB_COUNT 2

/**
 * Define the size of dynamic display lists.
 */
#define GDL_SIZE 0x1000

#endif