#ifndef __PSX2N64_CONFIG_H__
#define __PSX2N64_CONFIG_H__ 

#define REGION 3

/**
 * Determines whether to replace the licensed text with a custom string.
 */
// #define USE_CUSTOM_TEXT

/**
 * Custom string to be displayed on the PS logo screen.
 */
#define CUSTOM_LINE1 "Licensed by Nintendo"
#define CUSTOM_LINE2 "No copyright infringement intended!!"
#define CUSTOM_ABBR  "ABCD"

/**
 * Causes the system to crash on loading the PS logo screen, resulting in the so-called "Personified Fear".
 * This mimics the original behaviour of the PlayStation when attempting to read an invalid disc without the usual asset checks.
 */
// #define PFEAR

#define BIOS_VER 4.9

#define VERBOSE

#endif