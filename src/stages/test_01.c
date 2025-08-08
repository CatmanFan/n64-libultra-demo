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
// #include "libultra-easy/fs.h"
#include "libultra-easy/rcp.h"
// #include "libultra-easy/gfx_2d.h"
#include "libultra-easy/gfx_3d.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "stages.h"
#include "strings.h"

/* =============== ASSETS =============== */
// The vertex coords
Vtx test_vertex_vtx[4] =
{
	// Top-left
	{{ {-64, 64, -5},		0, {0, 0},	{0,		0xFF,	0,		0xFF} }},
	// Top-right
	{{ {64, 64, -5},		0, {0, 0},	{0,		0,		0,		0xFF} }},
	// Bottom-right
	{{ {64, -64, -5},		0, {0, 0},	{0,		0,		0xFF,	0xFF} }},
	// Bottom-left
	{{ {-64, -64, -5},		0, {0, 0},	{0xFF,	0,		0,		0xFF} }},
};

// Draw a colorful square using our vertex parameters.
Gfx test_vertex_mesh[] =
{
	gsDPPipeSync(),
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsDPSetRenderMode(G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2),
    // gsDPSetColorDither(G_CD_MAGICSQ),
	gsSPClearGeometryMode(0xFFFFFFFF),
	gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),

	gsSPVertex(&(test_vertex_vtx[0]), 4, 0),
	gsSP1Triangle(0, 1, 2, 0),
	gsSP1Triangle(0, 2, 3, 0),
	gsSPEndDisplayList(),
};

/* ========== STATIC VARIABLES ========== */

static simpleObj Vertex = { .dl = test_vertex_mesh, .scale = 1.0 };

static f64 second_count;
static s64 seconds;
static int dither_mode;

/* ========== STATIC FUNCTIONS ========== */

/* ========== GLOBAL FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void test_01_init()
{
	time_reset();
	second_count = 0;
	seconds = 0;
	dither_mode = 0;
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void test_01_update()
{
	second_count = time_current() - seconds;
	if (second_count >= 1.0)
	{
		seconds++;
		second_count = 0;
	}

	Vertex.rot.z += 0.1;

	if (joypad_button(A, 0))
		dither_mode = (dither_mode + 1) % 4;

	if (joypad_button(B, 0))
		request_stage_change("test_menu");
}

/* ==============================
 * Renders frame.
 * ============================== */
void test_01_render()
{
	clear_zfb();
	if (dither_mode == 1)		{ gDPSetColorDither(glistp++, G_CD_BAYER); }
	else if (dither_mode == 2)	{ gDPSetColorDither(glistp++, G_CD_NOISE); }
	else if (dither_mode == 3)	{ gDPSetColorDither(glistp++, G_CD_DISABLE); }
	else						{ gDPSetColorDither(glistp++, G_CD_MAGICSQ); }
	clear_cfb(0, 245, 250);

	init_camera_2d();
	render_object(&Vertex);
	gDPPipeSync(glistp++);

	draw_gradient(0, 10, display_width(), 20, RGBA32(0,0,0,64), RGBA32(0,255,255,0), FALSE);

	draw_gradient(0, 0, display_width(), 10, RGBA32(0,0,0,255), RGBA32(64,64,64,255), FALSE);
	draw_rectangle(0, 0, display_width() * second_count, 10, RGBA32(0,255,0,255));
}