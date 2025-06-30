#include <ultra64.h>
#include <stdarg.h>

#include "config.h"

/* === Default libraries === */
// #include "helpers/types.h"
// #include "helpers/3d.h"
// #include "helpers/audio.h"
#include "helpers/controller.h"
#include "helpers/gfx.h"
// #include "helpers/reader.h"

/* === Custom libraries === */
#include "helpers/custom/console.h"
#include "helpers/custom/strings.h"

/* ========== STATIC VARIABLES ========== */

extern int target_stage;
static int selected_stage = 0;
static int max_stages = 2;

/* ========== COMMON FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void menu_init()
{
	reset_controller();
	console_clear();
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
	if (language < 0) { language = SHI; }
	if (language > SHI) { language = 0; }
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

	console_printf(STR_STAGE, selected_stage);
	glistp = console_draw_dl(glistp);

	finish_gfx();
}