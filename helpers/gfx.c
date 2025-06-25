#include <ultra64.h>
#include <assert.h>
#include <PR/sched.h>
#include <PR/sp.h>

#include "config.h"
#include "helpers/types.h"

/* ============= PROTOTYPES ============ */

Gfx glist[GL_SIZE];
Gfx *glistp;

int cfb_current = 0;
void *cfb[2] = { (void *)CFB1_ADDR, (void *)CFB2_ADDR };
u16 zbuffer[SCREEN_W * SCREEN_H];

extern OSSched scheduler;
extern OSScTask sched_task;
extern OSScMsg *sched_msg;
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
    gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_W, 0),
	gsDPSetCycleType(G_CYC_FILL),
	gsDPSetFillColor(0),
    gsDPFillRectangle(0, 0, SCREEN_W - 1, SCREEN_H - 1),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};


/* ============= FUNCTIONS ============== */

void init_gfx()
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
	gSPDisplayList(glistp++, dl_init_rdp);
	gSPDisplayList(glistp++, dl_init_rsp);
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

	// Swap framebuffer
	cfb_current ^= 1;
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


/* ============== SPRITES =============== */

void init_sprite() // Gfx **dlp
{
    // Gfx* dl;
    // dl = *dlp;
    // spInit(&dl);
    spInit(&glistp);
}

void draw_sprite(Sprite *sp)
{
	gSPDisplayList(glistp++, spDraw(sp));
}

void finish_sprite()
{
    spFinish(&glistp);
    glistp -= 1;
}

/* =========== CPU RENDERING ============ */

// GPACK_RGBA5551(r, g, b, 1)
void draw_pixel(int x, int y, unsigned int color)
{
	// Set beginning CFB pointer
	#if (SCREEN_W == 640 && SCREEN_H == 480)
	u32 *ptr = (u32 *)
	#else
	u16 *ptr = (u16 *)
	#endif
	osViGetCurrentFramebuffer() + (y * SCREEN_W) + x;
	
	// Write color directly
    *ptr = color;
}

void draw_rectangle(int x, int y, int w, int h)
{
	// Set pointer X,Y coordinates
	int ptr_x, ptr_y;
	
	// Set beginning CFB pointer
	#if (SCREEN_W == 640 && SCREEN_H == 480)
	u32 *ptr = (u32 *)
	#else
	u16 *ptr = (u16 *)
	#endif
	osViGetCurrentFramebuffer() + (y * SCREEN_W) + x;

	// Write color directly
	for (ptr_y = y; ptr_y < y + h; ptr_y++)
	{
		for (ptr_x = x; ptr_x < x + w; ptr_x++)
		{
            // 0xe738 = 0b1110011100111000
            *ptr = ((*ptr & 0xe738) >> 2) | 1;
            ptr++;
		}

		ptr += SCREEN_W - w;
	}
}