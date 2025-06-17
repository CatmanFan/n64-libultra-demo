#include <ultra64.h>

#include "config.h"

#include "helpers/types.h"
#include "helpers/gfx.h"
#include "helpers/renderer.h"
#include "helpers/controller.h"

/* =============== ASSETS =============== */

#include "assets/models/psx_logo.h"

/* ============= 3D OBJECTS ============= */

static simpleObj camera;
static Lights1 light = gdSPDefLights1
(
	60, 60, 60,  // Ambient light
	255, 255, 255,  // Diffuse light
	50, 100, 100 // Direction toward diffuse light
);

static simpleObj logo;

/* ========== STATIC VARIABLES ========== */

static int x = 0;
static int y = 0;
static int z = -500;
static float fov = 50;

/* ==============================
 * Initializes stage.
 * ============================== */
void stage00_init()
{
	x = 0;
	y = 0;
	z = -500;
	fov = 50;
	
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
	if (controller[0].stick_x > 0)      x++;
	else if (controller[0].stick_x < 0) x--;
	if (controller[0].stick_y > 0)      y--;
	else if (controller[0].stick_y < 0) y++;
	if (controller[0].button == A_BUTTON) z++;
	else if (controller[0].button == B_BUTTON) z--;
	if (controller[0].button == L_TRIG) fov += 0.1;
	else if (controller[0].button == R_TRIG) fov -= 0.1;
}

/* ==============================
 * Renders frame.
 * ============================== */
void stage00_render()
{
	init_gfx();
	clear_zfb();
	clear_cfb(128, 128, 128);
	
	{
		init_world(camera, x, y, z, fov);
		gSPSetLights1(glistp++, light);
		
		render_object(psx_logo_pslogo_mesh, &logo.pos, &logo.rot, 1.0);
		
		gDPPipeSync(glistp++);
	}
	
	finish_gfx();
}