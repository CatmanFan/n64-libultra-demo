#include <ultra64.h>

/* === Configuration === */
#include "config/video.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
// #include "libultra-easy/audio.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/display.h"
#include "libultra-easy/fault.h"
#include "libultra-easy/rcp.h"
#include "libultra-easy/gfx_2d.h"
#include "libultra-easy/gfx_3d.h"
// #include "libultra-easy/fs.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "strings.h"

/* =============== ASSETS =============== */

#include "assets/fonts/psx_bios.h"
#include "assets/fonts/terminus.h"
#include "assets/models/psx_logo.h"
#include "assets/sprites/biosA_ps.h"
#include "assets/sprites/biosB_ps.h"

/* ========== STATIC VARIABLES ========== */

static simpleObj psx_logo = { .dl = psx_logo_mesh, .scale = 1.0 };
static Light ambient;
static Light direct;

static sprite_t ps_text;

static f64 time_elapsed;
static bool passed_first_stage;
static f64 alpha1;
static f64 alpha2;

static int scale_2d;

// #define PFEAR
#define BIOS_VER 4.0
static float BIOS = BIOS_VER;

/* ========== STATIC FUNCTIONS ========== */

/* ========== GLOBAL FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void psx2n64_01_init()
{
	time_reset();

	// Play AUDIO

	#ifdef PFEAR
    *(u32*)0x11111111 = 0; // trigger an exception via unaligned memory access
	#endif

	// display_set(1);
	scale_2d = display_highres() ? 1 : 2;

	time_elapsed = 0.0;

	vec3_set(psx_logo.pos, 31, 137, -1100);
	vec3_set(psx_logo.rot, 22.8, 2.5, 1);

	alpha1 = 0;
	alpha2 = 0;

    direct.l.dir[0] = -75;
    direct.l.dir[1] = 60;
    direct.l.dir[2] = 75;

	if (BIOS < 4.3)
	{
		sprite_create_tlut(&ps_text, BIOS1_ps_tex, BIOS1_ps_tlut, 197, 39, 1, G_IM_SIZ_8b);
		ps_text.x = 229 / scale_2d;
		ps_text.y = (271 + 1) / scale_2d;
	}
	else
	{
		sprite_create_tlut(&ps_text, BIOS2_ps_tex, BIOS2_ps_tlut, 199, 40, 1, G_IM_SIZ_4b);
		ps_text.x = 228 / scale_2d;
		ps_text.y = (270 + 1) / scale_2d;
	}
	ps_text.r = 0;
	ps_text.g = 0;
	ps_text.b = 0;
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void psx2n64_01_update()
{
	time_elapsed = time_current();
	passed_first_stage = time_elapsed >= 0.5;

	if (!passed_first_stage)
	{
		alpha1 = time_elapsed * 2;
	}
	else
	{
		alpha1 = 1.0;
		if (time_elapsed < 1.0)
			alpha2 = time_elapsed * 2 - 1.0;
		else
			alpha2 = 1.0;
	}

	ps_text.r = round(255 * alpha2);
	ps_text.g = round(255 * alpha2);
	ps_text.b = round(255 * alpha2);
}

/* ==============================
 * Renders frame.
 * ============================== */
void psx2n64_01_render()
{
	extern void draw_text(const char *txt, Font *font, int x, int y, int bounds_w, int bounds_h, int align, int line_spacing, bool line_wrap, bool transparent);

	vec3 cam = {0, 0, 1600};
	vec3 dest = {0, 0, -1};
	int i;

	for (i = 0; i < 3; i++)
	{
		ambient.l.col[i] = round(70 * alpha1);
		ambient.l.colc[i] = round(70 * alpha1);
		direct.l.col[i] = round(240 * alpha1);
		direct.l.colc[i] = round(240 * alpha1);
	}

	clear_zfb();
	clear_cfb(100, 0, 0);

	gSPNumLights(glistp++, NUMLIGHTS_1);
	gSPLight(glistp++, &direct, 1);
	gSPLight(glistp++, &ambient, 2);
	gDPPipeSync(glistp++);

	init_camera_3d(cam, dest, 27.9F);
	render_object(&psx_logo);
	gDPPipeSync(glistp++);

	if (passed_first_stage)
	{
		sprite_init();
		sprite_draw(&ps_text);
		sprite_finish();

		// draw_text("Hi bitches!", &psx_bios_font, 120, 331, 400, 38, 1, 2, FALSE, TRUE);
		draw_text("abcdefgh", joypad_is_pressed(A_BUTTON, 0) ? &psx_bios_font : &terminus, 20, 20, 0, 0, 0, 0, FALSE, TRUE);
	}
}

void psx2n64_01_destroy()
{
	display_set(-1);
}