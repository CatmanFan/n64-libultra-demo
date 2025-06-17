#ifndef __CONFIG_H__
#define __CONFIG_H__

	#define SCREEN_W            320
	#define SCREEN_H            240

	/* [Stack sizes] */
	#define STACK_SIZE          0x2000
	#define STACK_SIZE_BOOT     0x800
	#define STACK_SIZE_IDLE     0x800
	#define STACK_SIZE_MAIN     STACK_SIZE

	/* [Threads] */
	#define ID_IDLE             5
	#define ID_MAIN             6
	#define ID_SCHEDULER        7
	#define ID_AUDIO            8
	
	#define PR_IDLE             10
	#define PR_MAIN             10
	#define PR_SCHEDULER        120
	#define PR_AUDIO            70

	/* [Messages] */
	#define NUM_PI_MSGS         16
	#define NUM_GFX_MSGS        8
	#define NUM_SI_MSGS         4

	/* [Graphics] */
	#define MAX_OBJECTS         128     // Maximum objects drawn at once
	#define GL_SIZE             2048    // Dynamic display list size
	#define FIFO_SIZE           8192    // RDP stack size

	/* [Stack addresses] */
	#define START_ADDR          0x80400000
	
	#define AUDIO_SIZE          0x60000
	#define AUDIO_ADDR          (CFB1_ADDR-AUDIO_SIZE)
	
	// Place framebuffers at end of RAM
	#define CFB_SIZE            SCREEN_W*SCREEN_H*2
	#define CFB1_ADDR           (START_ADDR-(CFB_SIZE*2))
	#define CFB2_ADDR           (START_ADDR-(CFB_SIZE))

	// #define F3DEX_GBI_2          // Microcode. See <gbi.h>

#endif