#include <ultra64.h>
#include <assert.h>

/* === Configuration === */
#include "config/video.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
#include "libultra-easy/audio.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/display.h"
#include "libultra-easy/fault.h"
#include "libultra-easy/fs.h"
#include "libultra-easy/rcp.h"
// #include "libultra-easy/gfx_2d.h"
#include "libultra-easy/gfx_3d.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "strings.h"

/* [ASSETS]
========================================= */
extern Gfx globe_mesh[];
#include "assets/models/n64_logo.h"

/* [SEGMENTS]
========================================= */

/* [VARIABLES]
========================================= */
// Camera
static float fov;

// Object
static float scale;
// static int obj_type;
// static int prev_obj_type;

// Message timer
static f64 msg_time_marker;
static f64 msg_timer;
static bool msg_alt;

/* [3D MODELS]
========================================= */
static vec3 camera;
static Lights3 light = gdSPDefLights3
(
	35, 35, 35,		// Ambient light

	245, 255, 255,	// Diffuse light #1
	-127, 0, 80,	// Direction toward diffuse light #1

	200, 0, 255,	// Diffuse light #2
	127, -127, -79,	// Direction toward diffuse light #2

	255, 255, 0,	// Diffuse light #3
	0, 127, 80		// Direction toward diffuse light #3
);

static simpleObj obj = { .dl = n64_logo_logo_mesh, .scale = 1.0 };
static simpleObj globe = { .dl = globe_mesh, .scale = 0.333 };

/* [STATIC FUNCTIONS]
========================================= */

/* [MAIN FUNCTIONS]
========================================= */

/* ==============================
 * Initializes stage.
 * ============================== */
void test_00_init()
{
	fov = 40;
	scale = 1.0;

	vec3_set(camera, 0, 10, 250);

	// obj_type = 0;

	reset_controller();

	console_clear();

	// sound_set_bank("sample");
	// The sound effects bank is now the default bank

	// Now that I played the song, the sound effects bank has been reverted back to default
	// (which in this case is the sound effects bank). So now I can play a sound without
	// the need for MusPtrBankSetSingle.

	time_reset();
	msg_timer = 10.0;
	msg_time_marker = 10.0;
	msg_alt = FALSE;
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void test_00_update()
{
	camera.x += joypad_stick(0).x;
	camera.y -= joypad_stick(0).y;
	if (joypad_button(A, 0))
		camera.z-=8;
	else if (joypad_button(B, 0))
		camera.z+=8;

	/*if (joypad_button(C_UP, 0))
		fov+=0.2;
	else if (joypad_button(C_DOWN, 0))
		fov-=0.2;*/

	if (joypad_button(C_LEFT, 0))
		globe.pos.z-=8;
	else if (joypad_button(C_RIGHT, 0))
		globe.pos.z+=8;
	if (joypad_button(C_UP, 0))
		globe.pos.y+=8;
	else if (joypad_button(C_DOWN, 0))
		globe.pos.y-=8;

	if (joypad_button(DPAD_UP, 0))
		obj.rot.x-=2;
	else if (joypad_button(DPAD_DOWN, 0))
		obj.rot.x+=2;
	if (joypad_button(DPAD_LEFT, 0))
		obj.rot.y-=2;
	else if (joypad_button(DPAD_RIGHT, 0))
		obj.rot.y+=2;

	/*if (joypad_button(L, 0))
		obj_type -= 1;
	else if (joypad_button(R, 0))
		obj_type += 1;
	if (obj_type > 1) obj_type = 0;
	if (obj_type < 0) obj_type = 1;
	prev_obj_type = obj_type;*/

	if (joypad_button(START, 0))
	{
		// sound_set_bank("sample");
		// sound_play(0);
		crash();
	}

	// ==================================================

	globe.rot.y+=1;

	msg_timer = msg_time_marker - time_current();
	if (msg_timer < 0.0)
	{
		msg_alt = !msg_alt;
		while (msg_timer < 0.0)
		{
			msg_time_marker += 10.0;
			msg_timer += 10.0;
		}
	}
}

/* ==============================
 * Renders frame.
 * ============================== */
void test_00_render()
{
	clear_zfb();
	clear_cfb(52, 52, 52);
	draw_gradient(0, display_height() * 0.666, display_width(), display_height() * 0.333, RGBA32(82,82,82,0), RGBA32(92,92,92,255), FALSE);

	// ==========================
	// 3D rendering
	// ==========================
	init_camera_3d(camera, globe.pos, fov);
	gSPSetLights3(glistp++, light);

	render_object(&obj);
	render_object(&globe);
	gDPPipeSync(glistp++);

	// ==========================
	// 2D rendering
	// ==========================
	console_clear();
	console_puts("FPS: %d\n", fps());
	if (msg_alt)
	{
		console_puts(strings[8]);
	}
	else
	{
		console_puts(strings[5], 'X', obj.rot.x);
		console_puts(strings[5], 'Y', obj.rot.y);
		// console_puts(strings[5], 'Z', obj.rot.z);
		console_puts(strings[3], 'X', camera.x);
		console_puts(strings[3], 'Y', camera.y);
		console_puts(strings[3], 'Z', camera.z);
		// console_puts(strings[4], fov);
	}
	console_draw_dl();
}