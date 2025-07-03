#include <ultra64.h>

/* === Configuration === */
#include "config/global.h"

/* === Default libraries === */
#include "libraries/types.h"
// #include "libraries/audio.h"
#include "libraries/controller.h"
#include "libraries/gfx.h"
// #include "libraries/gfx_2d.h"
#include "libraries/gfx_3d.h"
// #include "libraries/reader.h"
#include "libraries/time.h"

/* === Custom libraries === */
#include "libraries/custom/console.h"
#include "libraries/custom/strings.h"

/* =============== ASSETS =============== */

#include "assets/models/test_vertex.h"

/* ========== STATIC VARIABLES ========== */

static int frame;

static OSTime time_start;
static OSTime time_prev;
static float time;

static int fps;
static int fps_frames;
static float frame_time;

/* ========== STATIC FUNCTIONS ========== */

void calculate_FPS(OSTime diff)
{
	frame_time += diff * SECONDS_PER_CYCLE;
	fps_frames++;
}

/* ========== GLOBAL FUNCTIONS ========== */

/* ==============================
 * Initializes stage.
 * ============================== */
void stage01_init()
{
	time_start = CURRENT_TIME;
	time = CYCLES_TO_SEC(CURRENT_TIME - time_start);

	frame = 0;
	frame_time = 0;
	fps = 0;
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void stage01_update()
{
	OSTime time_new = CURRENT_TIME;

	calculate_FPS(time_new - time_prev);
	// If frame time is longer or equal to a second, update FPS counter.
	if (frame_time >= 1.0f) 
	{
		fps = fps_frames;
		fps_frames = 0;
		frame_time -= 1.0f;
	}
	time_prev = time_new;

	time = CYCLES_TO_SEC(time_new - time_start);

	frame++;
}

/* ==============================
 * Renders frame.
 * ============================== */
void stage01_render()
{
	static simpleObj obj;
	vecSet(obj.pos, 0, 0, 0);
	vecSet(obj.rot, 0, 0, 0);

	init_gfx();
	clear_zfb();
	clear_cfb(0, 245, 250);

	init_camera_2d();
	render_object(test_vertex_mesh, &obj.pos, &obj.rot, 1.0);

	gDPSetFillColor(glistp++, 1);
	gDPFillRectangle(glistp++, SCREEN_W*0.3, SCREEN_H*0.3, SCREEN_W*0.7, SCREEN_H*0.7);
	gDPPipeSync(glistp++);

	console_clear();
	console_puts(str_02, frame, time, fps);
	glistp = console_draw_dl(glistp);

	finish_gfx();
}