#include <ultra64.h>

/* === Configuration === */
#include "config/video.h"

/* === Default libraries === */
#include "lib/types.h"
#include "lib/audio.h"
#include "lib/controller.h"
#include "lib/fs.h"
#include "lib/gfx.h"
#include "lib/gfx_2d.h"
#include "lib/gfx_3d.h"
#include "lib/time.h"

/* === Custom libraries === */
#include "console.h"
#include "strings.h"

/* =============== ASSETS =============== */

#include "assets/models/globe.h"
#include "assets/models/n64_logo.h"
#include "assets/models/psx_logo.h"

/* ============== SEGMENTS ============== */

ROM_SEGMENT(pbank_inst1)
ROM_SEGMENT(wbank_inst1)
ROM_SEGMENT(pbank_sfx1)
ROM_SEGMENT(wbank_sfx1)
ROM_SEGMENT(bgm1)
ROM_SEGMENT(sfx1)

/* ========== STATIC VARIABLES ========== */

// Camera
static float fov;

// Object
static float scale;
static int obj_type;
static int prev_obj_type;

/* ============= 3D OBJECTS ============= */

static simpleObj camera;
static Lights1 light = gdSPDefLights1
(
	25, 25, 25,		// Ambient light

	255, 255, 255,	// Diffuse light #1
	-127, 127, 80	// Direction toward diffuse light #1
);

static simpleObj obj;
static simpleObj globe = { .dl = globe_mesh };

/* =============== SPRITES ============== */

#include "assets/sprites/sce0.h"

static SpriteData sony_0, sony_1, sony_2, sony_3, sony_4, sony_5;

/* ========== STATIC FUNCTIONS ========== */

static void die()
{
	// TLB exception on load/instruction fetch
	long e1;
	e1 = *(long *)1;

	// TLB exception on store
	*(long *)2 = 2;
}

/* ========== GLOBAL FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void stage00_init()
{
	fov = 40;
	scale = 1.0;

	vecSet(camera.pos, 0, 600, 1600);
	vecSet(globe.pos, 0, 0, 0);
	vecSet(obj.rot, 0, 0, 0);
	obj_type = 0;

	load_sounds
	(
		ROM_START(pbank_sfx1),
		ROM_END(pbank_sfx1),
		ROM_START(wbank_sfx1),
		ROM_START(sfx1),
		ROM_END(sfx1)
	);
	// The sound effects bank is now the default bank

	load_inst
	(
		ROM_START(pbank_inst1),
		ROM_END(pbank_inst1),
		ROM_START(wbank_inst1)
	);
	play_bgm
	(
		ROM_START(bgm1),
		ROM_END(bgm1)
	);
	// Now that I played the song, the sound effects bank has been reverted back to default
	// (which in this case is the sound effects bank). So now I can play a sound without
	// the need for MusPtrBankSetSingle.

	reset_controller();

	console_clear();

	sony_0 = sprite_create(&sony_obj_0, &sony_tex_0, NULL);
	sony_1 = sprite_create(&sony_obj_1, &sony_tex_1, NULL);
	sony_2 = sprite_create(&sony_obj_2, &sony_tex_2, NULL);
	sony_3 = sprite_create(&sony_obj_3, &sony_tex_3, NULL);
	sony_4 = sprite_create(&sony_obj_4, &sony_tex_4, NULL);
	sony_5 = sprite_create(&sony_obj_5, &sony_tex_5, NULL);
	sony_0.x = 100;
	sony_1.x = sony_1.x + (40 * 1);
	sony_2.x = sony_1.x + (40 * 2);
	sony_3.x = sony_1.x + (40 * 3);
	sony_4.x = sony_1.x + (40 * 4);
	sony_5.x = sony_1.x + (40 * 5);
	sony_0.y =
	sony_1.y =
	sony_2.y =
	sony_3.y =
	sony_4.y =
	sony_5.y = 36;
	sony_0.a =
	sony_1.a =
	sony_2.a =
	sony_3.a =
	sony_4.a =
	sony_5.a = 0;
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void stage00_update()
{
	read_controller();

	camera.pos[0] += controller[0].stick_x;
	camera.pos[1] -= controller[0].stick_y;
	if (controller[0].button == A_BUTTON)
		camera.pos[2]-=8;
	else if (controller[0].button == B_BUTTON)
		camera.pos[2]+=8;

	if (controller[0].button == U_CBUTTONS)
		fov+=0.2;
	else if (controller[0].button == D_CBUTTONS)
		fov-=0.2;

	if (controller[0].button == L_CBUTTONS)
		globe.pos[2]-=8;
	else if (controller[0].button == R_CBUTTONS)
		globe.pos[2]+=8;

	if (controller[0].button == U_JPAD)
		obj.rot[0]-=2;
	else if (controller[0].button == D_JPAD)
		obj.rot[0]+=2;
	if (controller[0].button == L_JPAD)
		obj.rot[1]-=2;
	else if (controller[0].button == R_JPAD)
		obj.rot[1]+=2;

	if (controller[0].button == L_TRIG)
		obj_type -= 1;
	else if (controller[0].button == R_TRIG)
		obj_type += 1;

	if (obj_type > 2) obj_type = 0;
	if (obj_type < 0) obj_type = 2;

	if (controller[0].button == START_BUTTON)
		die();

	// ==================================================

	if (obj_type == 0)
	{
		obj.dl = n64_logo_logo_mesh;
		obj.rot[1]++;
		obj.rot[2] = 0;
	}
	if (obj_type == 1)
	{
		obj.dl = psx_logo_pslogo_mesh;

		if (prev_obj_type != obj_type)
		{
			obj.rot[1] = 0;
			obj.rot[2] = -0.75;
		}
	}
	prev_obj_type = obj_type;

	globe.rot[1]+=1;

	sony_0.a += 1;
	if (sony_0.a > 255) sony_0.a = 0;
	sony_1.a =
	sony_2.a =
	sony_3.a =
	sony_4.a =
	sony_5.a = sony_0.a;
}

/* ==============================
 * Renders frame.
 * ============================== */
void stage00_render()
{
	gfx_init();
	clear_zfb();
	clear_cfb(72, 72, 72);

	// ==========================
	// 3D rendering
	// ==========================
	init_camera_3d
	(
		camera,
		camera.pos[0], camera.pos[1], camera.pos[2],
		globe.pos[0], globe.pos[1], globe.pos[2],
		fov
	);
	gSPSetGeometryMode(glistp++, G_LIGHTING);
	gSPSetLights1(glistp++, light);

	render_object(obj.dl, &obj.pos, &obj.rot, scale);
	render_object(globe.dl, &globe.pos, &globe.rot, 0.25);
	gDPPipeSync(glistp++);

	// ==========================
	// 2D rendering
	// ==========================
	sprite_init();
	sprite_draw(&sony_0);
	sprite_draw(&sony_1);
	sprite_draw(&sony_2);
	sprite_draw(&sony_3);
	sprite_draw(&sony_4);
	sprite_draw(&sony_5);
	sprite_finish();

	console_clear();
	console_puts(str_03, 'X', camera.pos[0]);
	console_puts(str_03, 'Y', camera.pos[1]);
	console_puts(str_03, 'Z', camera.pos[2]);
	console_puts(str_04, fov);
	console_puts(str_05, 'X', obj.rot[0]);
	console_puts(str_05, 'Y', obj.rot[1]);
	console_puts(str_05, 'Z', obj.rot[2]);
	console_draw_dl();

	// ==========================
	// Finish
	// ==========================
	finish_gfx();
}