#ifndef __GFX_H__
#define __GFX_H__

#include <ultra64.h>
#include "config.h"
#include "assets/textures/alex.h"

static Gfx sprite_dl[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsSPClearGeometryMode(G_SHADE | G_SHADING_SMOOTH),
    gsSPTexture(0x2000, 0x2000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetCombineMode(G_CC_DECALRGB, G_CC_DECALRGB),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPLoadTextureBlock(sprite, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 32,
                         0, G_TX_NOMIRROR, G_TX_NOMIRROR, 0, 0,
                         G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(40 << 2, 10 << 2, (40 + 16 - 1) << 2,
                         (10 + 32 - 1) << 2, 0, 0, 0, 4 << 10,
                         1 << 10),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};

// Viewport scaling parameters.
static const Vp viewport =
{{
	{SCREEN_W*2, SCREEN_H*2, G_MAXZ/2, 0}, // Scale
	{SCREEN_W*2, SCREEN_H*2, G_MAXZ/2, 0}, // Translation
}};

// Initialize the RSP.
const Gfx DL_initRSP[] = {
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
const Gfx DL_initRDP[] = {
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_W, SCREEN_H),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF),
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};

#endif