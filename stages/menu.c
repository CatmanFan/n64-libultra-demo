#include <ultra64.h>
#include <stdarg.h>

/* === Configuration === */
#include "config/global.h"
#include "config/video.h"

/* === Default libraries === */
// #include "libraries/types.h"
// #include "libraries/audio.h"
#include "libraries/controller.h"
#include "libraries/gfx.h"
// #include "libraries/gfx_2d.h"
// #include "libraries/gfx_3d.h"
#include "libraries/reader.h"
// #include "libraries/time.h"

/* === Custom libraries === */
#include "libraries/custom/console.h"
#include "libraries/custom/strings.h"

/* ========== STATIC VARIABLES ========== */

extern int target_stage;
static int selected_stage = 0;
static int max_stages = 2;

/* ========== GLOBAL FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void menu_init()
{
	reset_controller();
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void menu_update()
{
	read_controller();

	if (controller[0].button == R_JPAD)
		selected_stage += 1;
	if (controller[0].button == L_JPAD)
		selected_stage -= 1;

	if (controller[0].button == R_TRIG)
		language += 1;
	if (controller[0].button == L_TRIG)
		language -= 1;

	if (selected_stage < 0) { selected_stage = max_stages - 1; }
	if (selected_stage >= max_stages) { selected_stage = 0; }
	change_language();

	if (controller[0].button == START_BUTTON || controller[0].button == A_BUTTON) { target_stage = selected_stage; }
}

/* ==============================
 * Renders frame.
 * ============================== */
void menu_render()
{
	init_gfx();
	clear_zfb();
	clear_cfb(0, 0, 0);

	console_clear();
	#if DEBUG_MODE
		console_puts("LANG: %d", language);
		console_puts("SCREEN: %dx%d", SCREEN_W, SCREEN_H);
		console_puts("SYSTEM: %d", VIDEO_TYPE);
		console_puts("VIDEO: %s\n", osViGetCurrentMode() == 0 ? "NTSC.LPN (16 bit)"
								 : osViGetCurrentMode() == 1 ? "NTSC.LPF (16 bit)"
								 : osViGetCurrentMode() == 2 ? "NTSC.LAN (16 bit)"
								 : osViGetCurrentMode() == 3 ? "NTSC.LAF (16 bit)"
								 : osViGetCurrentMode() == 4 ? "NTSC.LPN (32 bit)"
								 : osViGetCurrentMode() == 5 ? "NTSC.LPF (32 bit)"
								 : osViGetCurrentMode() == 6 ? "NTSC.LAN (32 bit)"
								 : osViGetCurrentMode() == 7 ? "NTSC.LAF (32 bit)"
								 : osViGetCurrentMode() == 8 ? "NTSC.HPN (16 bit)"
								 : osViGetCurrentMode() == 9 ? "NTSC.HPF (16 bit)"
								 : osViGetCurrentMode() == 10 ? "NTSC.HAN (16 bit)"
								 : osViGetCurrentMode() == 11 ? "NTSC.HAF (16 bit)"
								 : osViGetCurrentMode() == 12 ? "NTSC.HPN (32 bit)"
								 : osViGetCurrentMode() == 13 ? "NTSC.HPF (32 bit)"

								 : osViGetCurrentMode() == 14 ? "PAL.LPN (16 bit)"
								 : osViGetCurrentMode() == 15 ? "PAL.LPF (16 bit)"
								 : osViGetCurrentMode() == 16 ? "PAL.LAN (16 bit)"
								 : osViGetCurrentMode() == 17 ? "PAL.LAF (16 bit)"
								 : osViGetCurrentMode() == 18 ? "PAL.LPN (32 bit)"
								 : osViGetCurrentMode() == 19 ? "PAL.LPF (32 bit)"
								 : osViGetCurrentMode() == 20 ? "PAL.LAN (32 bit)"
								 : osViGetCurrentMode() == 21 ? "PAL.LAF (32 bit)"
								 : osViGetCurrentMode() == 22 ? "PAL.HPN (16 bit)"
								 : osViGetCurrentMode() == 23 ? "PAL.HPF (16 bit)"
								 : osViGetCurrentMode() == 24 ? "PAL.HAN (16 bit)"
								 : osViGetCurrentMode() == 25 ? "PAL.HAF (16 bit)"
								 : osViGetCurrentMode() == 26 ? "PAL.HPN (32 bit)"
								 : osViGetCurrentMode() == 27 ? "PAL.HPF (32 bit)"

								 : osViGetCurrentMode() == 28 ? "MPAL.LPN (16 bit)"
								 : osViGetCurrentMode() == 29 ? "MPAL.LPF (16 bit)"
								 : osViGetCurrentMode() == 30 ? "MPAL.LAN (16 bit)"
								 : osViGetCurrentMode() == 31 ? "MPAL.LAF (16 bit)"
								 : osViGetCurrentMode() == 32 ? "MPAL.LPN (32 bit)"
								 : osViGetCurrentMode() == 33 ? "MPAL.LPF (32 bit)"
								 : osViGetCurrentMode() == 34 ? "MPAL.LAN (32 bit)"
								 : osViGetCurrentMode() == 35 ? "MPAL.LAF (32 bit)"
								 : osViGetCurrentMode() == 36 ? "MPAL.HPN (16 bit)"
								 : osViGetCurrentMode() == 37 ? "MPAL.HPF (16 bit)"
								 : osViGetCurrentMode() == 38 ? "MPAL.HAN (16 bit)"
								 : osViGetCurrentMode() == 39 ? "MPAL.HAF (16 bit)"
								 : osViGetCurrentMode() == 40 ? "MPAL.HPN (32 bit)"
								 : osViGetCurrentMode() == 41 ? "MPAL.HPF (32 bit)"

								 : osViGetCurrentMode() == 42 ? "FPAL.LPN (16 bit)"
								 : osViGetCurrentMode() == 43 ? "FPAL.LPF (16 bit)"
								 : osViGetCurrentMode() == 44 ? "FPAL.LAN (16 bit)"
								 : osViGetCurrentMode() == 45 ? "FPAL.LAF (16 bit)"
								 : osViGetCurrentMode() == 46 ? "FPAL.LPN (32 bit)"
								 : osViGetCurrentMode() == 47 ? "FPAL.LPF (32 bit)"
								 : osViGetCurrentMode() == 48 ? "FPAL.LAN (32 bit)"
								 : osViGetCurrentMode() == 49 ? "FPAL.LAF (32 bit)"
								 : osViGetCurrentMode() == 50 ? "FPAL.HPN (16 bit)"
								 : osViGetCurrentMode() == 51 ? "FPAL.HPF (16 bit)"
								 : osViGetCurrentMode() == 52 ? "FPAL.HAN (16 bit)"
								 : osViGetCurrentMode() == 53 ? "FPAL.HAF (16 bit)"
								 : osViGetCurrentMode() == 54 ? "FPAL.HPN (32 bit)"
								 : osViGetCurrentMode() == 55 ? "FPAL.HPF (32-bit)"

								 : "unknown");
	#endif
	console_puts(str_01, selected_stage);
	glistp = console_draw_dl(glistp);

	finish_gfx();
}