#include <ultra64.h>

/* === Configuration === */
#include "config/video.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
// #include "libultra-easy/audio.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/crash.h"
#include "libultra-easy/display.h"
#include "libultra-easy/rcp.h"
#include "libultra-easy/gfx_2d.h"
#include "libultra-easy/gfx_3d.h"
// #include "libultra-easy/fs.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "strings.h"

/* =============== ASSETS =============== */

extern Gfx globe_mesh[];
#include "assets/models/psx_logo.h"

/* ========== STATIC VARIABLES ========== */

static simpleObj psx_logo = { .dl = psx_logo_mesh, .scale = 1.0 };
static Light ambient;
static Light direct;

static f64 time_elapsed;
static bool passed_first_stage;
static f64 alpha1;
static f64 alpha2;

static int scale_2d;

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

	// display_set(1);
	scale_2d = display_highres() ? 1 : 2;

	time_elapsed = 0.0;

	vec3_set(psx_logo.pos, 52, 118, -1100);
	vec3_set(psx_logo.rot, 22.34, 2, 1);

	alpha1 = 0;
	alpha2 = 0;

    direct.l.dir[0] = -128;
    direct.l.dir[1] = 127;
    direct.l.dir[2] = 127;
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
}

/* ==============================
 * Renders frame.
 * ============================== */
void psx2n64_01_render()
{
	vec3 cam = {0, 0, 1600};
	vec3 dest = {0, 0, -1};
	int i;

	for (i = 0; i < 3; i++)
	{
		ambient.l.col[i] = round(82 * alpha1);
		ambient.l.colc[i] = round(82 * alpha1);
		direct.l.col[i] = round(255 * alpha1);
		direct.l.colc[i] = round(255 * alpha1);
	}

	clear_zfb();
	clear_cfb(0, 0, 0);

	gSPNumLights(glistp++, NUMLIGHTS_1);
	gSPLight(glistp++, &direct, 1);
	gSPLight(glistp++, &ambient, 2);
	gDPPipeSync(glistp++);

	init_camera_3d(cam, dest, 27.5F);
	render_object(&psx_logo);
	gDPPipeSync(glistp++);
}

void psx2n64_01_destroy()
{
	display_set(-1);
}