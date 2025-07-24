/* ***************************************************** */

#include <ultra64.h>
#include <stdarg.h>

/* === Configuration === */
#include "config/global.h"
#include "config/video.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
// #include "libultra-easy/audio.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/fs.h"
#include "libultra-easy/display.h"
#include "libultra-easy/fault.h"
#include "libultra-easy/rcp.h"
#include "libultra-easy/gfx_2d.h"
// #include "libultra-easy/gfx_3d.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "strings.h"
#include "stages.h"

#include "assets/sprites/white.h"

/* ========== STATIC VARIABLES ========== */

static int fade_opacity;
static int selected_stage;
static int offset = 1;

static sprite_t fade;

/* ========== GLOBAL FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void test_menu_init()
{
	time_reset();
	fade_opacity = 255;

	display_set(0);

	selected_stage = offset;

	sprite_create_tlut(&fade, white_tex, white_tlut, 32, 32, 1, G_IM_SIZ_4b);

	fade.scale_x = 600.0;
	fade.scale_y = 400.0;
	fade.r = 0;
	fade.g = 0;
	fade.b = 0;

	reset_controller();
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void test_menu_update()
{
	int max_stages = 5;

	if (controller[0].button == R_JPAD || controller[0].stick_x >= 10)
		selected_stage += 1;
	if (controller[0].button == L_JPAD || controller[0].stick_x <= -10)
		selected_stage -= 1;
	if (selected_stage < 0)
		selected_stage = max_stages - 1;
	if (selected_stage >= max_stages)
		selected_stage = 0 + offset;

	if (controller[0].button == R_TRIG)
		language += 1;
	if (controller[0].button == L_TRIG)
		language -= 1;

	change_language();

	if (controller[0].button == A_BUTTON)
		target_stage = stages[selected_stage].id;
}

/* ==============================
 * Renders frame.
 * ============================== */
#include "assets/fonts/terminus.h"
void test_menu_render()
{
	clear_zfb();
	clear_cfb(0, 0, 0);

	draw_gradient(0, 0, display_width(), display_height(), RGBA(255,0,0,255), RGBA(64,0,0,255), FALSE);

	console_clear();

	if (fade_opacity <= 0)
	{
		console_puts(str_01, stages[selected_stage].id);
		console_puts("\n");
	}
	else
	{
		fade.a = fade_opacity;

		sprite_init();
		sprite_draw(&fade);
		sprite_finish();

		fade_opacity-=6;
	}

	console_puts("FPS: %2d / delta: %0.3f", fps(), time_delta());
	console_puts("Time: %0.3f", time_current());
	console_puts("V: %d (0: PAL/1: NTSC/2: MPAL)", VIDEO_TYPE);
	console_puts("%dx%d", display_width(), display_height());
	console_draw_dl();
}