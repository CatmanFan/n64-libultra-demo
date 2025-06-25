#include <ultra64.h>
#include <stdarg.h>

#include "config.h"
#include "helpers/types.h"
#include "helpers/gfx.h"
#include "helpers/gfx_3d.h"
#include "helpers/controller.h"
#include "helpers/text.h"

/* =============== ASSETS =============== */

#include "assets/models/n64_logo.h"
#include "assets/models/psx_logo.h"

/* ============= 3D OBJECTS ============= */

static simpleObj camera;
static Lights1 light = gdSPDefLights1
(
	25, 25, 25,  // Ambient light
	
	255, 255, 255,  // Diffuse light #1
	-127, 127, 80 // Direction toward diffuse light #1
);

static simpleObj obj;

/* ========== STATIC VARIABLES ========== */

static int x;
static int y;
static int z;
static float fov;

static int obj_type;

static int frame;

/* ========== STATIC FUNCTIONS ========== */

#include <os_internal.h>
static void die()
{
	// TLB exception on load/instruction fetch
	long e1;
	e1 = *(long *)1;

	// TLB exception on store
	*(long *)2 = 2;
}

/* ========== COMMON FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void stage00_init()
{
	x = 0;
	y = 600;
	z = 1600;
	fov = 40;

	obj_type = 0;
	frame = 0;

	init_controller();
	reset_controller();
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void stage00_update()
{
	read_controller();
	
	if (controller[0].stick_x > 0)
		x+=3;
	else if (controller[0].stick_x < 0)
		x-=3;

	if (controller[0].stick_y > 0)
		y-=3;
	else if (controller[0].stick_y < 0)
		y+=3;

	if (controller[0].button == A_BUTTON)
		z-=3;
	else if (controller[0].button == B_BUTTON)
		z+=3;

	if (controller[0].button == U_JPAD)
		fov+=0.2;
	else if (controller[0].button == D_JPAD)
		fov-=0.2;

	if (controller[0].button == L_JPAD)
		obj.rot[0]-=2;
	else if (controller[0].button == R_JPAD)
		obj.rot[0]+=2;

	if (controller[0].button == L_TRIG)
		obj_type -= 1;
	else if (controller[0].button == R_TRIG)
		obj_type += 1;
	
	if (obj_type > 1) obj_type = 0;
	if (obj_type < 0) obj_type = 1;

	if (controller[0].button == START_BUTTON)
		die();
}

/* ==============================
 * Renders frame.
 * ============================== */
void stage00_render()
{
	init_gfx();
	clear_zfb();
	clear_cfb(72, 72, 72);

	init_world(camera, x, y, z, fov);
	gSPSetLights1(glistp++, light);

	vecSet(obj.rot, 0, 0, -0.75);
	if (obj_type == 0)
		{ obj.dl = n64_logo_logo_mesh; obj.rot[1] = frame; }
	if (obj_type == 1)
		obj.dl = psx_logo_pslogo_mesh;

	render_object(obj.dl, &obj.pos, &obj.rot, 1.0);

	gDPPipeSync(glistp++);

	// print("Frame: %d", &frame);

	finish_gfx();

	frame++;
}