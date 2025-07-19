#include <ultra64.h>
#include <math.h>

#include "config/global.h"
#include "config/video.h"
#include "config/usb.h"

#include "libultra-easy/types.h"
#include "libultra-easy/display.h"
#include "libultra-easy/gfx.h"
#include "libultra-easy/rcp.h"
#include "libultra-easy/stack.h"
#include "libultra-easy/crash.h"

Gfx glist[CFB_COUNT][GDL_SIZE]; // Dynamic global DL size
Gfx *glistp;

static RenderTask *cur_task = NULL;
static OSTask rcp_task = 
{{
	.type             = M_GFXTASK,
	.flags            = OS_TASK_DP_WAIT | OS_TASK_LOADABLE,
	.ucode_size       = SP_UCODE_SIZE,
	.ucode_data_size  = SP_UCODE_DATA_SIZE,
	.dram_stack       = (u64*)&dram_stack,
	.dram_stack_size  = SP_DRAM_STACK_SIZE8,
	.output_buff      = (u64*)&fifo_buffer,
	.output_buff_size = &fifo_buffer[STACK_SIZE_RDPFIFO / sizeof(u64)],
	.yield_data_ptr   = (u64*)&yield_buffer,
	.yield_data_size  = OS_YIELD_DATA_SIZE,
}};

/* =================================================== *
 *                         RCP                         *
 * =================================================== */

// Viewport scaling parameters. The dimensions are defined automatically by gfx_init_rsp().
static Vp viewport =
{{
	{0, 0, G_MAXZ/2, 0}, // Scale
	{0, 0, G_MAXZ/2, 0}, // Translation
}};

// Initialize the RDP.
void rcp_init_rdp()
{
	gDPSetCycleType(glistp++, G_CYC_1CYCLE);
	gDPPipelineMode(glistp++, G_PM_1PRIMITIVE);
	gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, display_width(), display_height());

	gDPSetTextureLOD(glistp++, G_TL_TILE);
	gDPSetTextureLUT(glistp++, G_TT_NONE);
	gDPSetTextureDetail(glistp++, G_TD_CLAMP);
	gDPSetTexturePersp(glistp++, G_TP_PERSP);
	gDPSetTextureFilter(glistp++, G_TF_BILERP);
	gDPSetTextureConvert(glistp++, G_TC_FILT);
	gDPSetCombineMode(glistp++, G_CC_SHADE, G_CC_SHADE);

	gDPSetCombineKey(glistp++, G_CK_NONE);
	gDPSetAlphaCompare(glistp++, G_AC_NONE);
	gDPSetRenderMode(glistp++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
	gDPSetColorDither(glistp++, G_CD_ENABLE);
	gDPPipeSync(glistp++);
}

// Initialize the RSP.
void rcp_init_rsp()
{
	// Automatically change viewport dimensions
	viewport.vp.vscale[0] = display_width() * 2;
	viewport.vp.vtrans[0] = display_width() * 2;
	viewport.vp.vscale[1] = display_height() * 2;
	viewport.vp.vtrans[1] = display_height() * 2;

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

void rcp_start(RenderTask* task)
{
	extern int vtx_count;

	// Init display list
	glistp = task->dl;

	// Set the segment register to segment #0
	// The addresses passed to the RCP must be in segment addresses
	// The CPU uses virtual addresses, this will convert accordingly
	gSPSegment(glistp++, 0, 0);

	// Execute RSP and RDP initialization
	rcp_init_rdp();
	rcp_init_rsp();

	// Reset 3D objects counter
	vtx_count = 0;

	// Set current task pointer
	cur_task = task;
}

void rcp_finish(RenderTask *task)
{
	// Finish the display list
	gDPFullSync(glistp++);
	gSPEndDisplayList(glistp++);

	// Normally there should be an assert function to check the display list limit here
	my_assert(((glistp - task->dl) < GDL_SIZE), "GFX assertion failed");

	// Writeback cache lines so that the RCP can read the up-to-date data
	osWritebackDCacheAll();

	// Setup the RCP task
	rcp_task.t.ucode_boot       = (u64*)rspbootTextStart;
	rcp_task.t.ucode_boot_size  = ((u32)rspbootTextEnd-(u32)rspbootTextStart);
	rcp_task.t.ucode            = (u64*)gspF3DEX2_fifoTextStart;
	rcp_task.t.ucode_data       = (u64*)gspF3DEX2_fifoDataStart;
	rcp_task.t.data_ptr         = (u64*)task->dl;
	rcp_task.t.data_size        = ((u32)(glistp - task->dl)*sizeof(Gfx));
	task->task = &rcp_task;
}

/* =================================================== *
 *                 DISPLAY LIST HELPERS                *
 * =================================================== */

void clear_zfb()
{
	gDPSetDepthImage(glistp++, OS_K0_TO_PHYSICAL(zbuffer));
	gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, display_width(), OS_K0_TO_PHYSICAL(zbuffer));
	gDPSetCycleType(glistp++, G_CYC_FILL);
	// gDPSetRenderMode(glistp++, G_RM_NOOP, G_RM_NOOP2);
	gDPSetFillColor(glistp++, GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0));
	gDPFillRectangle(glistp++, 0, 0, display_width() - 1, display_height() - 1);
	gDPPipeSync(glistp++);
}

void clear_cfb(int r, int g, int b)
{
	debug_printf("[RCP] Clearing framebuffer with resolution %dx%d\n", display_width(), display_height());
	debug_printf("[RCP] Color: %x %x %x\n", r, g, b);
#ifdef VIDEO_32BIT
	gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_32b, display_width(), cur_task->fb);
#else
	gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, display_width(), cur_task->fb);
#endif
	gDPSetCycleType(glistp++, G_CYC_FILL);
	gDPSetFillColor(glistp++, GPACK_RGBA5551(r, g, b, 1) << 16 | GPACK_RGBA5551(r, g, b, 1));
	gDPFillRectangle(glistp++, 0, 0, display_width() - 1, display_height() - 1);
	gDPPipeSync(glistp++);
}

void draw_rectangle(int x, int y, int w, int h, u32 color)
{
	u32 r = (color >> 24) & 0xFF;
	u32 g = (color >> 16) & 0xFF;
	u32 b = (color >> 8) & 0xFF;
	u32 a = color & 0xFF;

	gDPSetCycleType(glistp++, G_CYC_1CYCLE);
	gDPSetCombineMode(glistp++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
	gDPSetRenderMode(glistp++, G_RM_XLU_SURF, G_RM_XLU_SURF);

	gDPSetPrimColor(glistp++, 0, 0, r, g, b, a);
	gDPFillRectangle(glistp++, x, y, x + w, y + h);
	gDPPipeSync(glistp++);
}

void draw_gradient(int x, int y, int w, int h, u32 color1, u32 color2, bool horizontal)
{
	int line;
	int end = horizontal ? w : h;

	u32 r1, g1, b1, a1, r2, g2, b2, a2, r, g, b, a;
	int rec_x, rec_y, rec_w, rec_h;

	r1 = ((color1 & 0xFF000000) >> 24);
	g1 = ((color1 & 0x00FF0000) >> 16);
	b1 = ((color1 & 0x0000FF00) >> 8);
	a1 = (color1 & 0x000000FF);

	r2 = ((color2 & 0xFF000000) >> 24);
	g2 = ((color2 & 0x00FF0000) >> 16);
	b2 = ((color2 & 0x0000FF00) >> 8);
	a2 = (color2 & 0x000000FF);

	for (line = 0; line <= end; line++)
	{
		double factor = (double)line / (double)end;

		rec_x = horizontal ? x + line : x;
		rec_y = horizontal ? y        : y + line;
		rec_w = horizontal ? 1        : w;
		rec_h = horizontal ? h        : 1;

		r = (u32)((r1 * (1 - factor)) + (r2 * factor));
		g = (u32)((g1 * (1 - factor)) + (g2 * factor));
		b = (u32)((b1 * (1 - factor)) + (b2 * factor));
		a = (u32)((a1 * (1 - factor)) + (a2 * factor));

		draw_rectangle(rec_x, rec_y, rec_w, rec_h, RGBA(r,g,b,a));
	}
}