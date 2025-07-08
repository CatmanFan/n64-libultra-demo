#include <ultra64.h>
#include <assert.h>

#include "config/global.h"
#include "config/video.h"

#include "lib/types.h"
#include "lib/scheduler.h"

/* ============= PROTOTYPES ============= */

Gfx glist[GL_SIZE];
Gfx *glistp;

int cfb_current = 0;
void *cfb[2] = { (void *)CFB1_ADDR, (void *)CFB2_ADDR };
u16 zbuffer[SCREEN_W * SCREEN_H];

extern OSMesgQueue msgQ_gfx;

extern int vtx_count;

/* ======== STATIC DISPLAY LISTS ======== */

// Viewport scaling parameters.
static const Vp viewport =
{{
	{SCREEN_W*2, SCREEN_H*2, G_MAXZ/2, 0}, // Scale
	{SCREEN_W*2, SCREEN_H*2, G_MAXZ/2, 0}, // Translation
}};

// Initialize the RSP.
static const Gfx dl_init_rsp[] = {
	gsSPViewport(&viewport),
	gsSPClearGeometryMode(0xFFFFFFFF),
	gsSPSetGeometryMode
	(
		G_ZBUFFER
	  | G_SHADE
	  // | G_SHADING_SMOOTH
	  // | G_CULL_FRONT
	  // | G_CULL_BACK
	  // | G_CULL_BOTH
	  // | G_FOG
	  | G_LIGHTING
	  // | G_TEXTURE_GEN
	  // | G_TEXTURE_GEN_LINEAR
	  // | G_CLIPPING
	  // | G_LOD
	),
	gsSPTexture(0, 0, 0, 0, G_OFF),
	gsSPEndDisplayList(),
};

// Initialize the RDP.
static const Gfx dl_init_rdp[] = {
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_W, SCREEN_H),
	gsDPSetCombineKey(G_CK_NONE),
	gsDPSetAlphaCompare(G_AC_NONE),
	gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF),
	gsDPSetColorDither(G_CD_DISABLE),
	gsDPPipeSync(),
	gsSPEndDisplayList(),
};

static Gfx dl_clear_zfb[] = {
	gsDPSetDepthImage(OS_K0_TO_PHYSICAL(zbuffer)),
	gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_W, OS_K0_TO_PHYSICAL(zbuffer)),
	gsDPSetCycleType(G_CYC_FILL),
	gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
	gsDPSetFillColor(GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0)),
	gsDPFillRectangle(0, 0, SCREEN_W - 1, SCREEN_H - 1),
	gsDPPipeSync(),
	gsSPEndDisplayList(),
};

static Gfx dl_clear_cfb[] = {
#ifdef VIDEO_32BIT
	gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_32b, SCREEN_W, 0),
#else
	gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_W, 0),
#endif
	gsDPSetCycleType(G_CYC_FILL),
	gsDPSetFillColor(0),
	gsDPFillRectangle(0, 0, SCREEN_W - 1, SCREEN_H - 1),
	gsDPPipeSync(),
	gsSPEndDisplayList(),
};


/* ============= FUNCTIONS ============== */

float y_scale = 1;
void my_osViBlack(bool active)
{
	osViSetYScale(active ? 1 : y_scale);
	osViBlack(active);
}

void gfx_init_rdp()
{
	gDPSetCycleType(glistp++, G_CYC_1CYCLE);
	gDPPipelineMode(glistp++, G_PM_1PRIMITIVE);
	gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, SCREEN_W, SCREEN_H);

	gDPSetTextureLOD(glistp++, G_TL_TILE);
	gDPSetTextureLUT(glistp++, G_TT_NONE);
	gDPSetTextureDetail(glistp++, G_TD_CLAMP);
	gDPSetTexturePersp(glistp++, G_TP_PERSP);
	gDPSetTextureFilter(glistp++, G_TF_BILERP);
	gDPSetTextureConvert(glistp++, G_TC_FILT);
	gDPSetCombineMode(glistp++, G_CC_SHADE, G_CC_SHADE);

	gDPSetCombineKey(glistp++, G_CK_NONE);
	gDPSetAlphaCompare(glistp++, G_AC_NONE);
	gDPSetRenderMode(glistp++, G_RM_OPA_SURF, G_RM_OPA_SURF);
	gDPSetColorDither(glistp++, G_CD_MAGICSQ);
	gDPPipeSync(glistp++);
}

void gfx_init_rsp()
{
	gSPViewport(glistp++, &viewport);
	gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
	gSPSetGeometryMode
	(
		glistp++,

	// Toggle geometry modes here:
	// ---------------------------
		G_ZBUFFER
	  | G_SHADE
	  | G_SHADING_SMOOTH
	  // | G_CULL_FRONT
	  // | G_CULL_BACK
	  // | G_CULL_BOTH
	  // | G_FOG
	  | G_LIGHTING
	  // | G_TEXTURE_GEN
	  // | G_TEXTURE_GEN_LINEAR
	  // | G_CLIPPING
	  // | G_LOD
	);
	gSPTexture(glistp++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
	gSPClipRatio(glistp++, FRUSTRATIO_1);
}

void gfx_init()
{
	// Reset 3D objects counter
	vtx_count = 0;

	// Init display list
	glistp = glist;

	// Set the segment register to segment #0
	// The addresses passed to the RCP must be in segment addresses
	// The CPU uses virtual addresses, this will convert accordingly
	gSPSegment(glistp++, 0, 0);

	// Execute RSP and RDP initialization
	gfx_init_rdp();
	gfx_init_rsp();
}

void finish_gfx()
{
	// Finish the display list
	gDPFullSync(glistp++);
	gSPEndDisplayList(glistp++);

	// Normally there should be an assert function to check the display list limit here
	assert((glistp - glist) < GL_SIZE);

	// Point scheduler task to display list and set framebuffer
	sched_task.list.t.data_ptr = (u64 *)glist;
	sched_task.list.t.data_size = (s32)(glistp - glist) * sizeof(Gfx);
	sched_task.framebuffer = cfb[cfb_current];

	// Writeback cache lines so that the RCP can read the up-to-date data
	osWritebackDCacheAll();

	// Send task
	osSendMesg(osScGetCmdQ(&scheduler), (OSMesg)&sched_task, OS_MESG_BLOCK);

	// Wait for rendering to finish
	do {
	osRecvMesg(&msgQ_gfx, (OSMesg *)&sched_msg, OS_MESG_BLOCK);
	} while (sched_msg->type != OS_SC_DONE_MSG);

	// Swap framebuffer value (the actual CFB has already been swapped automatically)
	cfb_current ^= 1;
}

void swap_cfb(int index)
{
	int outOfRange = index < 0 || index >= 2;
	osViSwapBuffer(outOfRange ? cfb[cfb_current] : cfb[index]);
	if (outOfRange || index == cfb_current) cfb_current ^= 1;
}

void clear_zfb()
{
	gSPDisplayList(glistp++, dl_clear_zfb);
}

void clear_cfb(int r, int g, int b)
{
	dl_clear_cfb[0] = (Gfx)gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_W, OS_K0_TO_PHYSICAL(cfb[cfb_current]));
	dl_clear_cfb[2] = (Gfx)gsDPSetFillColor(GPACK_RGBA5551(r, g, b, 1) << 16 | GPACK_RGBA5551(r, g, b, 1));

	gSPDisplayList(glistp++, dl_clear_cfb);
}