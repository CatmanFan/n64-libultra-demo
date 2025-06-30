#include <ultra64.h>

#include "config.h"
#include "types.h"
#include "helpers/gfx.h"

/* ============= PROTOTYPES ============= */

#define MAX_VTX 128
Mtx vtx[MAX_VTX];
int vtx_count;

Mtx projection;
Mtx viewing;

/* ============= FUNCTIONS ============== */

void init_world(simpleObj cam, int x, int y, int z, float fov)
{
	u16 pnorm;

	vecSet(cam.pos, x, y, z);
	vecSet(cam.mov, 0, 0, 0);

	guPerspective(&projection, &pnorm, fov, 1.3333333, 50.0, 5000.0, 0.5);
	gSPPerspNormalize(glistp++, pnorm);

	guLookAt
	(
		&viewing,
		
		// Position coordinates
		cam.pos[0],
		cam.pos[1],
		cam.pos[2],
		
		// Destination coordinates
		cam.mov[0],
		cam.mov[1],
		cam.mov[2],
		
		// Up-facing coordinates
		0,
		1,
		0
	);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&viewing), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
}

void render_object(Gfx* obj_dl, vec3_t* obj_pos, vec3_t* obj_rot, float size)
{
	if (vtx_count >= MAX_VTX) return;

	guPosition
	(
		&vtx[vtx_count],
		(*obj_rot)[0],
		(*obj_rot)[1],
		(*obj_rot)[2],
		size,
		(*obj_pos)[0],
		(*obj_pos)[1],
		(*obj_pos)[2]
	);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&vtx[vtx_count]), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	gSPDisplayList(glistp++, obj_dl);

	vtx_count++;
}