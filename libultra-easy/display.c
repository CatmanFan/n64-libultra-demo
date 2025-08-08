#include <ultra64.h>
#include "libultra-easy.h"

typedef struct
{
	volatile int screen_w;
	volatile int screen_h;
	volatile bool highres;
	volatile bool is_32bit;
	volatile float yscale;
} Display;

static volatile Display display;
extern Scheduler scheduler;

float display_yscale()
{
	return display.yscale;
}

int display_width()
{
	return display.highres ? SCREEN_W_HD : SCREEN_W_SD;
}

int display_height()
{
	return display.highres ? SCREEN_H_HD : SCREEN_H_SD;
}

bool display_highres()
{
	return display.highres;
}

void display_off()
{
	osViSetYScale(1);
	osViBlack(TRUE);
	scheduler.display = FALSE;
}

void display_on()
{
	osViBlack(FALSE);
	osViSetYScale(display.yscale);
	scheduler.display = TRUE;
}

int display_framerate()
{
	if (display_tvtype() == 0)
		return 50;
	else
		return 60;
}

int display_tvtype()
{
	#if defined VIDEO_TYPE && (VIDEO_TYPE >= 0 && VIDEO_TYPE <= 2)
		return VIDEO_TYPE;
	#else
		return osTvType;
	#endif
}

void display_set(int highres)
{
	s32 viMode;
	// int i;
	// for (i = 0; i < CFB_COUNT; i++)
	// 	framebuffers[i].status = FB_FREE;
	// osSpTaskYield();

	// Check if changing the display is necessary; if not, then abort
	bool __highres = (highres < 0 || highres > 1) ? 
	#ifdef VIDEO_HIGHRES
		TRUE
	#else
		FALSE
	#endif
	 : highres == 1 ? TRUE : FALSE;
	bool needs_change = display.highres != __highres;
	if (!needs_change && scheduler.initialized)
		return;

	// Set scheduler bool to TRUE to avoid interference
	scheduler.is_changing_res = TRUE;

	// Set internal variables
	display.highres = __highres;
	display.screen_w = display.highres == TRUE ? SCREEN_W_HD : SCREEN_W_SD;
	display.screen_h = display.highres == TRUE ? SCREEN_H_HD : SCREEN_H_SD;
	#ifdef VIDEO_32BIT
	display.is_32bit = TRUE;
	#else
	display.is_32bit = FALSE;
	#endif
	display.yscale = 1;

	// Set initial viMode parameter and Y-scale
	switch (display_tvtype())
	{
		case OS_TV_PAL:
			#ifdef VIDEO_FPAL
			viMode = OS_VI_FPAL_LAN1;
			display.yscale = 0.833;
			#else
			viMode = OS_VI_PAL_LAN1;
			#endif
			break;

		case OS_TV_MPAL:
			viMode = OS_VI_MPAL_LAN1;
			break;

		case OS_TV_NTSC:
			viMode = OS_VI_NTSC_LAN1;
			break;
	}

	// Increment viMode based on highres option
	if (display.highres == TRUE)
		viMode += 8;

	// Increment viMode based on video depth
	if (display.is_32bit == TRUE)
		viMode += display.highres == TRUE ? 2 : 4;

	// viMode += 1; // Interlaced / Deflickered interlaced

	debug_printf("[Display] Setting display resolution.\n");
	debug_printf("[Display] Gamma: OFF\n");
	debug_printf("[Display] Gamma dithering: OFF\n");
	if (display.highres) { debug_printf("[Display] Highres: ON (%dx%d)\n", display_width(), display_height()); }
	else { debug_printf("[Display] Highres: OFF (%dx%d)\n", display_width(), display_height()); }
	if (display.is_32bit) { debug_printf("[Display] Pixel depth: 32-bit\n"); }
	else { debug_printf("[Display] Pixel depth: 16-bit\n"); }

	osViSetMode(&osViModeTable[viMode]);
	osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	osViSetSpecialFeatures(OS_VI_GAMMA_DITHER_OFF);
	// osViSetSpecialFeatures(OS_VI_DIVOT_OFF);
	#ifndef VIDEO_32BIT
		// osViSetSpecialFeatures(OS_VI_DITHER_FILTER_OFF); // set to ON if G_CD_MAGICSQ
	#endif

	display_off();
	wait_cycles(10);

	scheduler.is_changing_res = FALSE;
	debug_printf("[Display] Done.\n");
}