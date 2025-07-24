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
// #include "libultra-easy/gfx_2d.h"
#include "libultra-easy/gfx_3d.h"
// #include "libultra-easy/fs.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "strings.h"

/* =============== ASSETS =============== */

#include "assets/models/test_vertex.h"

/* ========== STATIC VARIABLES ========== */

static simpleObj Vertex = { .dl = test_vertex_mesh, .scale = 1.0 };

static f64 second_count;
static s64 seconds;

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
}

/* ==============================
 * Renders frame.
 * ============================== */
void test_01_render()
{
	clear_zfb();
	clear_cfb(0, 245, 250);

	draw_gradient(display_width() * 0.25, display_height() * 0.25, display_width() * 0.5, display_height() * 0.5, RGBA(255,0,255,255), RGBA(0,255,255,0), FALSE);

	init_camera_2d();
	render_object(&Vertex);
	gDPPipeSync(glistp++);

	draw_rectangle(0, 0, display_width(), 10, RGBA(0,0,0,255));
	draw_rectangle(0, 0, display_width() * second_count, 10, RGBA(0,255,0,255));
}