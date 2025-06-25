#include <ultra64.h>

#include "config.h"
#include "helpers/types.h"
#include "helpers/gfx.h"
#include "helpers/controller.h"
#include "helpers/text.h"

static int frame;

/* ==============================
 * Initializes stage.
 * ============================== */
void stage01_init()
{
	frame = 0;
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void stage01_update()
{
	frame++;
}

/* ==============================
 * Renders frame.
 * ============================== */
void stage01_render()
{
	init_gfx();
	clear_zfb();
	clear_cfb(0, 245, 250);

	gDPSetFillColor(glistp++, 1);
	gDPFillRectangle(glistp++, SCREEN_W*0.3, SCREEN_H*0.3, SCREEN_W*0.7, SCREEN_H*0.7);
	gDPPipeSync(glistp++);

	finish_gfx();
}