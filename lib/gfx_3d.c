#include <ultra64.h>

#include "config/video.h"

#include "lib/types.h"
#include "lib/gfx.h"

/* ============= PROTOTYPES ============= */

#define MAX_VTX 128
Mtx vtx[MAX_VTX];
int vtx_count;

Mtx projection;
Mtx viewing;

/* ============= FUNCTIONS ============== */

void init_camera_2d()
{
	guOrtho(&projection, -SCREEN_W / 2.0F, SCREEN_W / 2.0F, -SCREEN_H / 2.0F, SCREEN_H / 2.0F, 1.0F, 10.0F, 1.0F);
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
}

void init_camera_3d(simpleObj cam, float src_x, float src_y, float src_z, float dest_x, float dest_y, float dest_z, float fov)
{
	u16 pnorm;

	vec3_t src, dest;
	vecSet(src, src_x, src_y, src_z);
	vecSet(dest, dest_x, dest_y, dest_z);

	vecSet(cam.pos, src[0], src[1], src[2]);

	guPerspective(&projection, &pnorm, fov, 1.3333333, 50.0, 5000.0, 0.5);
	guLookAt
	(
		&viewing,
		
		// Position coordinates
		cam.pos[0],
		cam.pos[1],
		cam.pos[2],
		
		// Destination coordinates
		dest[0],
		dest[1],
		dest[2],
		
		// Up-facing coordinates
		0,
		1,
		0
	);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&viewing), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);

	gSPPerspNormalize(glistp++, pnorm);
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