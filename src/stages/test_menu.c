/* ***************************************************** */

/* [LIBRARIES]
========================================= */
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

/* [ASSETS]
========================================= */
#include "assets/sprites/white.h"

/* [VARIABLES]
========================================= */
static int fade_opacity;
static int selected_stage = -1;
static int offset = 1;

static int bg_color;
static int bg_r;
static int bg_g;
static int bg_b;

static sprite_t fade;

/* [STATIC FUNCTIONS]
========================================= */

/* [MAIN FUNCTIONS]
========================================= */

/* ==============================
 * Initializes stage.
 * ============================== */
void test_menu_init()
{
	extern sound_test();

	display_set(-1);
	if (selected_stage < 0)
		selected_stage = offset;

	sprite_create_tlut(&fade, white_tex, white_tlut, 32, 32, 1, G_IM_SIZ_4b);
	fade.scale_x = 600.0;
	fade.scale_y = 400.0;
	fade.r = 0;
	fade.g = 0;
	fade.b = 0;
	fade_opacity = 255;
	bg_color = 0;
	bg_r = 255;
	bg_g = 0;
	bg_b = 0;

	reset_controller();
	time_reset();
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void test_menu_update()
{
	int max_stages = 7;

	if (joypad_button(DPAD_RIGHT, 0) || joypad_stick(0).x >= 10)
		selected_stage += 1;
	if (joypad_button(DPAD_LEFT, 0) || joypad_stick(0).x <= -10)
		selected_stage -= 1;
	if (selected_stage < 0)
		selected_stage = max_stages - 1;
	if (selected_stage >= max_stages)
		selected_stage = 0 + offset;

	if (joypad_button(L, 0))
		language -= 1;
	if (joypad_button(R, 0))
		language += 1;

	change_language();

	if (joypad_button(A, 0))
		request_stage_change(stages[selected_stage].name);

	if (fade_opacity <= 0)
	{
		switch (bg_color)
		{
			/* RGBMCYB : */
			/*default:
			case 0:
				bg_r--;
				bg_g++;
				if (bg_g >= 255)
				{
					bg_r = 0;
					bg_g = 255;
					bg_color = 1;
				}
				break;

			case 1:
				bg_g--;
				bg_b++;
				if (bg_b >= 255)
				{
					bg_g = 0;
					bg_b = 255;
					bg_color = 2;
				}
				break;

			case 2:
				bg_r++;
				if (bg_r >= 255)
				{
					bg_r = 255;
					bg_color = 3;
				}
				break;

			case 3:
				bg_r--;
				bg_g++;
				if (bg_g >= 255)
				{
					bg_r = 0;
					bg_g = 255;
					bg_color = 4;
				}
				break;

			case 4:
				bg_b--;
				bg_r++;
				if (bg_r >= 255)
				{
					bg_b = 0;
					bg_r = 255;
					bg_color = 5;
				}
				break;

			case 5:
				bg_b++;
				if (bg_b >= 255)
				{
					bg_b = 255;
					bg_color = 6;
				}
				break;

			case 6:
				bg_g--;
				bg_b--;
				if (bg_g <= 0)
				{
					bg_g = 0;
					bg_b = 0;
					bg_color = 0;
				}
				break;*/

			/* RAINBOW : */
			default:
			case 0:
				bg_g++;
				if (bg_g >= 255)
				{
					bg_g = 255;
					bg_color = 1;
				}
				break;

			case 1:
				bg_r--;
				if (bg_r <= 0)
				{
					bg_r = 0;
					bg_color = 2;
				}
				break;

			case 2:
				bg_b++;
				if (bg_b >= 255)
				{
					bg_b = 255;
					bg_color = 3;
				}
				break;

			case 3:
				bg_g--;
				if (bg_g <= 0)
				{
					bg_g = 0;
					bg_color = 4;
				}
				break;

			case 4:
				bg_r++;
				if (bg_r >= 255)
				{
					bg_r = 255;
					bg_color = 5;
				}
				break;

			case 5:
				bg_b--;
				if (bg_b <= 0)
				{
					bg_b = 0;
					bg_color = 0;
				}
				break;
		}
	}
}

/* ==============================
 * Renders frame.
 * ============================== */
void test_menu_render()
{
	clear_zfb();
	clear_cfb(0, 0, 0);

	draw_gradient(0, 0, display_width(), display_height(), RGBA32(bg_r,bg_g,bg_b,255), RGBA32(round(bg_r * 0.25),round(bg_g * 0.25),round(bg_b * 0.25),255), FALSE);

	console_clear();

	if (fade_opacity <= 0)
	{
		console_puts(strings[2]);
		console_puts("(L_DPAD/R_DPAD) %s", stages[selected_stage].name);
	}
	else
	{
		console_puts(strings[1]);

		fade.a = fade_opacity;

		sprite_init();
		sprite_draw(&fade);
		sprite_finish();

		fade_opacity-=6;
	}

	console_puts("\n");
	console_puts(strings[7], time_current_frame(), time_current(), time_system(), fps());

	console_draw_dl();
}