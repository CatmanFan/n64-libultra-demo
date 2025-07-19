#include <ultra64.h>

#include "config/video.h"

#include "libultra-easy/types.h"
#include "libultra-easy/gfx_3d.h"
#include "libultra-easy/display.h"
#include "libultra-easy/rcp.h"

/* ============= PROTOTYPES ============= */

#define MAX_VTX 128
Mtx vtx[MAX_VTX];
int vtx_count;

Mtx projection;
Mtx viewing;

/* ============= FUNCTIONS ============== */

void init_camera_2d()
{
	guOrtho(&projection, -(float)display_width() / 2.0F, (float)display_width() / 2.0F, -(float)display_height() / 2.0F, (float)display_height() / 2.0F, 1.0F, 10.0F, 1.0F);
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
}

void init_camera_3d(vec3 src, vec3 dest, float fov)
{
	u16 pnorm;

	guPerspective(&projection, &pnorm, fov, 1.3333333, 50.0, 5000.0, 1.0);
	guLookAt
	(
		&viewing,
		
		// Position coordinates
		src.x,
		src.y,
		src.z,
		
		// Destination coordinates
		dest.x,
		dest.y,
		dest.z,
		
		// Up-facing coordinates
		0,
		1,
		0
	);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&viewing), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);

	gSPPerspNormalize(glistp++, pnorm);
}

void render_object(simpleObj *obj)
{
	if (vtx_count >= MAX_VTX) return;

	guPosition
	(
		&vtx[vtx_count],
		(float)(obj->rot.x),
		(float)(obj->rot.y),
		(float)(obj->rot.z),
		(float)(obj->scale),
		(float)(obj->pos.x),
		(float)(obj->pos.y),
		(float)(obj->pos.z)
	);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&vtx[vtx_count]), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	gSPDisplayList(glistp++, obj->dl);
	gDPPipeSync(glistp++);

	vtx_count++;
}