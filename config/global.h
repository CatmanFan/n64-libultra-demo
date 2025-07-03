#ifndef __CONFIG_H__
#define __CONFIG_H__

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

/**
 * Configure stack sizes.
 *
 * Stack sizes cannot be smaller than OS_MIN_STACKSIZE, except for RDP/FIFO,
 * which cannot be smaller than 256 bytes (0x100).
 */
#define STACK_SIZE          0x2000
#define STACK_SIZE_BOOT     0x800
#define STACK_SIZE_IDLE     0x800
#define STACK_SIZE_MAIN     STACK_SIZE
#define STACK_SIZE_CRASH    STACK_SIZE
#define STACK_SIZE_RDPFIFO  0x2000
#define STACK_SIZE_AUDIO	0x60000

/**
 * Configure thread IDs.
 */
#define ID_IDLE             5
#define ID_MAIN             6
#define ID_SCHEDULER        7
#define ID_AUDIO            8
#define ID_CRASH            12

/**
 * Configure thread priority values.
 */
#define PR_IDLE             10
#define PR_MAIN             10
#define PR_SCHEDULER        120
#define PR_AUDIO            70
#define PR_CRASH            OS_PRIORITY_APPMAX

/**
 * Configure message size limits.
 */
#define NUM_PI_MSGS         16
#define NUM_GFX_MSGS        8

/**
 * Configure color framebuffer size and addresses.
 */
#define CFB_SIZE            SCREEN_W*SCREEN_H*2
#define CFB1_ADDR           (START_ADDR-(CFB_SIZE*2))
#define CFB2_ADDR           (START_ADDR-(CFB_SIZE))

/**
 * Configure global graphics display list size.
 */
#define GL_SIZE             0x0800    // Dynamic

/**
 * Configure stack addresses.
 */
// [Starting address]
#define START_ADDR          0x80400000
// [Audio stack addresses]
#define AUDIO_ADDR          (CFB1_ADDR-STACK_SIZE_AUDIO)

#endif