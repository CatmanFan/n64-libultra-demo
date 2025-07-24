#include <ultra64.h>
#include <assert.h>

/* === Configuration === */
#include "config/video.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
// #include "libultra-easy/audio.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/fault.h"
#include "libultra-easy/fs.h"
#include "libultra-easy/rcp.h"
// #include "libultra-easy/gfx_2d.h"
#include "libultra-easy/gfx_3d.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "strings.h"

/* =============== ASSETS =============== */

extern Gfx globe_mesh[];
#include "assets/models/n64_logo.h"

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

static simpleObj obj = { .dl = n64_logo_logo_mesh };
static simpleObj globe = { .dl = globe_mesh, .scale = 0.333 };

/* ========== STATIC FUNCTIONS ========== */

static void die()
{
	crash();
}

/* ========== GLOBAL FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void test_00_init()
{
	fov = 40;
	scale = 1.0;

	vec3_set(camera, 0, 600, 1600);

	obj_type = 0;

	// The sound effects bank is now the default bank

	// Now that I played the song, the sound effects bank has been reverted back to default
	// (which in this case is the sound effects bank). So now I can play a sound without
	// the need for MusPtrBankSetSingle.

	reset_controller();

	console_clear();
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void test_00_update()
{
	camera.x += controller[0].stick_x;
	camera.y -= controller[0].stick_y;
	if (controller[0].button == A_BUTTON)
		camera.z-=8;
	else if (controller[0].button == B_BUTTON)
		camera.z+=8;

	if (controller[0].button == U_CBUTTONS)
		fov+=0.2;
	else if (controller[0].button == D_CBUTTONS)
		fov-=0.2;

	if (controller[0].button == L_CBUTTONS)
		globe.pos.z-=8;
	else if (controller[0].button == R_CBUTTONS)
		globe.pos.z+=8;

	if (controller[0].button == U_JPAD)
		obj.rot.x-=2;
	else if (controller[0].button == D_JPAD)
		obj.rot.x+=2;
	if (controller[0].button == L_JPAD)
		obj.rot.y-=2;
	else if (controller[0].button == R_JPAD)
		obj.rot.y+=2;

	if (controller[0].button == L_TRIG)
		obj_type -= 1;
	else if (controller[0].button == R_TRIG)
		obj_type += 1;

	if (obj_type > 1) obj_type = 0;
	if (obj_type < 0) obj_type = 1;

	if (controller[0].button == START_BUTTON)
		die();

	// ==================================================

	prev_obj_type = obj_type;

	globe.rot.y+=1;
}

/* ==============================
 * Renders frame.
 * ============================== */
void test_00_render()
{
	clear_zfb();
	clear_cfb(72, 72, 72);

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
	console_puts(str_03, 'X', camera.x);
	console_puts(str_03, 'Y', camera.y);
	console_puts(str_03, 'Z', camera.z);
	console_puts(str_04, fov);
	console_puts(str_05, 'X', obj.rot.x);
	console_puts(str_05, 'Y', obj.rot.y);
	console_puts(str_05, 'Z', obj.rot.z);
	console_draw_dl();
}