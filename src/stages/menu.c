#include <ultra64.h>
#include <stdarg.h>

/* === Configuration === */
#include "config/global.h"
#include "config/video.h"

/* === Default libraries === */
// #include "lib/types.h"
// #include "lib/audio.h"
#include "lib/controller.h"
#include "lib/fs.h"
#include "lib/gfx.h"
#include "lib/gfx_2d.h"
// #include "lib/gfx_3d.h"
// #include "lib/time.h"

/* === Custom libraries === */
#include "console.h"
#include "strings.h"

/* =============== ASSETS =============== */

// #include "assets/sprites/sce0.h"

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
	gfx_init();
	clear_zfb();
	clear_cfb(0, 0, 0);

	// sprite_init();
	// sprite_draw(&sce0);
	// sprite_finish();

	console_clear();
	console_puts(str_01, selected_stage);
	console_draw_dl();

	finish_gfx();
}