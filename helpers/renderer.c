#include <ultra64.h>
#include "config.h"

#include "helpers/gfx.h"

int vtx_count;

#define MAX_OBJ 128
static Mtx vtx[MAX_OBJ];
static Mtx projection;
static Mtx viewing;

void init_world(simpleObj camera, int x, int y, int z, float fov)
{
	u16 pnorm;

	vecSet(camera.pos, x, y, z);
	vecSet(camera.mov, 0, 0, 0);

	guPerspective(&projection, &pnorm, fov, 1.3333333, 50.0, 5000.0, 0.5);
	gSPPerspNormalize(glistp++, pnorm);

	guLookAt
	(
		&viewing,
		
		// Position coordinates
		camera.pos[0],
		camera.pos[1],
		camera.pos[2],
		
		// Destination coordinates
		camera.mov[0],
		camera.mov[1],
		camera.mov[2],
		
		// Up
		0, 0, 1
	);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&viewing), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
}

void render_object(Gfx* obj_dl, vec3_t* obj_pos, vec3_t* obj_rot, float size)
{
	if (vtx_count >= MAX_OBJ) return;
	
	guPosition
	(
		&vtx[vtx_count],
		90+(*obj_rot)[0],
		(*obj_rot)[1],
		(*obj_rot)[2]-90+270,
		size,
		(*obj_pos)[0],
		(*obj_pos)[1],
		(*obj_pos)[2]
	);
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&vtx[vtx_count]), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
	
	gSPDisplayList(glistp++, obj_dl);
	
	vtx_count++;
}