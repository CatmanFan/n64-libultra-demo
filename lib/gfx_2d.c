#include <ultra64.h>
#include <stdlib.h>
#include <PR/gs2dex.h>
#include <PR/gu.h>

#include "config/global.h"
#include "config/video.h"
#include "config/usb.h"

#include "lib/types.h"
#include "lib/fs.h"
#include "lib/gfx.h"

// ===============================================================================================
// /!\ IMPORTANT NOTICE /!\
// Large portions of this code have been borrowed and improvised from gamemasterplc's S2D library
// with his credit. The original code can be found here:
// https://github.com/gamemasterplc/64noid/blob/master/sprite.c
// ===============================================================================================

static Gfx dl_sprite_init[] =
{
	gsDPPipeSync(),
	gsDPSetTexturePersp(G_TP_NONE),
	gsDPSetTextureLOD(G_TL_TILE),
	gsDPSetTextureLUT(G_TT_NONE),
	gsDPSetTextureConvert(G_TC_FILT),
	gsDPSetAlphaCompare(G_AC_THRESHOLD),
	gsDPSetBlendColor(0, 0, 0, 0x01),
	gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
	gsSPEndDisplayList(),
};

/* ============= FUNCTIONS ============== */

void sprite_init()
{
	gSPLoadUcodeL(glistp++, gspS2DEX2_fifo);
	gSPDisplayList(glistp++, dl_sprite_init);
}

void sprite_finish()
{
	gSPLoadUcodeL(glistp++, gspF3DEX2_fifo);

	// Execute RSP and RDP initialization
	gfx_init_rdp();
	gfx_init_rsp();
}

void sprite_draw_bg(uObjBg *bg, int x, int y, float scale_x, float scale_y, u8 r, u8 g, u8 b, u8 a)
{
	gSPBgRect1Cyc(glistp++, &bg);
}

/*static u8 fmt_tile_bytes(SpriteData *spr)
{
	return spr->pxl > 2 ? 2 : spr->pxl;
}

static int get_slice_height(SpriteData *spr)
{
	int bytes = fmt_tile_bytes(spr);
	int count = spr->fmt == G_IM_FMT_CI ? 256 : 512;

	int stride;
	if (bytes != 0)
	{
		if (spr->fmt == G_IM_FMT_RGBA && spr->pxl == G_IM_SIZ_32b)
			stride = (((spr->width) * bytes) + 7) >> 2;
		else
			stride = (((spr->width) * bytes) + 7) >> 3;
	}
	else
	{
		stride = (((spr->width) >> 1) + 7) >> 3;
	}

	return count / stride;
}

static void s2d_load(SpriteData *spr, u16 uls, u16 ult, u16 lrs, u16 lrt)
{
	gDPLoadTextureTile
	(
		glistp++,
		(u32)&(spr->data),

		spr->fmt == G_IM_FMT_I   ? G_IM_FMT_I
	  : spr->fmt == G_IM_FMT_IA  ? G_IM_FMT_IA
	  : spr->fmt == G_IM_FMT_CI  ? G_IM_FMT_CI
	  : spr->fmt == G_IM_FMT_YUV ? G_IM_FMT_YUV
	  : G_IM_FMT_RGBA,

		spr->pxl == G_IM_SIZ_DD  ? G_IM_SIZ_DD
	  : spr->pxl == G_IM_SIZ_8b  ? G_IM_SIZ_8b
	  : spr->pxl == G_IM_SIZ_16b ? G_IM_SIZ_16b
	  : G_IM_SIZ_32b,

		spr->width, spr->height,
		uls, ult, lrs, lrt,
		0,
		G_TX_CLAMP, G_TX_CLAMP,
		G_TX_NOMASK, G_TX_NOMASK,
		G_TX_NOLOD, G_TX_NOLOD
	);
}

static void s2d_create(SpriteData *spr, uObjSprite *obj, u16 spr_index, u16 y, u16 h)
{
	u8 tile_bytes = fmt_tile_bytes(spr);

	obj->s.objX			= 0 << 2;
	obj->s.objY			= y << 2;
	obj->s.imageW		= spr->width << 5;
	obj->s.imageH		= h << 5;

	obj->s.imageFmt		= spr->fmt;
	obj->s.imageSiz		= spr->pxl;

	if (tile_bytes != 0)
		obj->s.imageStride = ((spr->width * tile_bytes) + 7) >> 3;
	else 
		obj->s.imageStride = (((spr->width) >> 1) + 7) >> 3;
}*/

uObjTxtr txtr_blank =
{
	.tile =
	{
		G_OBJLT_TXTRTILE,
		0, // image
		0, // tmem
		0, // twidth
		0, // theight
		0, // sid
		-1, // flag
		0xFFFFFFFF, // mask
	},
};

uObjSprite obj_blank =
{{
	0<<2, 1<<10, 0<<5, 0,          /* objX, scaleX, imageW, unused */
	0<<2, 1<<10, 0<<5, 0,          /* objY, scaleY, imageH, unused */
	0, /* imageStride */
	0, /* imageAdrs */
	G_IM_FMT_RGBA, /* imageFmt */
	G_IM_SIZ_16b, /* imageSiz */
	0, /* imagePal */
	0, /* imageFlags */
}};

uObjMtx mtx_blank =
{{
	0x10000,  0,              /* A,B */
	0,        0x10000,        /* C,D */
	50,        50,            /* X,Y */
	1<<10,    1<<10           /* BaseScaleX, BaseScaleY */
}};

int get_nearest_multiple(int input)
{
	int multiple_array[] = { 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 64, 72, 76, 100, 108, 128, 144, 152, 164, 200, 216, 228, 256, 304, 328, 432, 456, 512, 684, 820, 912 };
	int multiple_i;

	while (multiple_array[multiple_i] < input && multiple_array[multiple_i] < 900)
		multiple_i++;

	return multiple_array[multiple_i];
}

void copy_img(void *img, int img_w, void *dest, int dest_w, int w, int h)
{
	int dest_x, dest_y;
    bzero(dest, sizeof(dest));

	for (dest_y = 0; dest_y < h; dest_y++)
	{
	    int *ptr = dest + (dest_w * dest_y);
		// memcpy(dest + (dest_y * dest_w), img + (dest_y * img_w), w);
		for (dest_x = 0; dest_x < w; dest_x++)
		{
		    ptr = img + (dest_y * img_w) + dest_x;
		    ptr++;
		}
	}
}

SpriteData sprite_create_raw(void *img, int w, int h, u8 fmt, u8 siz)
{
	SpriteData spr;

	uObjTxtr *txtr = &txtr_blank;
	uObjMtx *mtx = &mtx_blank;
	uObjSprite *obj = &obj_blank;

	txtr->tile.image = (u64 *)img;
	txtr->tile.tmem = GS_PIX2TMEM(0, siz);
	txtr->tile.twidth = GS_TT_TWIDTH(w, siz);
	txtr->tile.theight = GS_TT_THEIGHT(h, siz);

	obj->s.imageFmt = fmt;

	spr.txtr = txtr;
	spr.mtx = mtx;
	spr.obj = obj;
	spr.x = 0;
	spr.y = 0;
	return spr;
}

SpriteData sprite_create(uObjSprite *obj, uObjTxtr *txtr, u32 *pal)
{
	SpriteData spr;

	uObjMtx *mtx = &mtx_blank;

	spr.txtr = txtr;
	spr.pal = pal;
	spr.mtx = mtx;
	spr.obj = obj;

	if (pal != 0 && pal != NULL)
	{
		spr.txtr->tlut.type  = G_OBJLT_TLUT;
		spr.txtr->tlut.image = (u64 *)&pal;
		spr.txtr->tlut.phead = GS_PAL_HEAD(0);
		spr.txtr->tlut.pnum  = GS_PAL_NUM(sizeof(pal));
		spr.txtr->tlut.zero  = 0;
		spr.txtr->tlut.sid   = 0;
		spr.txtr->tlut.flag  = -1;
		spr.txtr->tlut.mask  = 0;
	}

	spr.x = 0;
	spr.y = 0;

	spr.r = 255;
	spr.g = 255;
	spr.b = 255;
	spr.a = 255;

	return spr;
}

void sprite_draw(SpriteData *spr)
{
	/*int i, slice_y;
	u16 slice_h = get_slice_height(spr);
	u16 slices = spr->height / slice_h;
	u16 slices_ext = spr->height % slice_h;

	// ******************************************

	// Setup sprite array
	uObjMtx *mtx = &mtx_blank;
	uObjSprite *obj = &obj_blank;*/

	// Set up DL
	gDPPipeSync(glistp++);
	gDPSetCycleType(glistp++, G_CYC_1CYCLE);
	gDPSetRenderMode(glistp++, G_RM_XLU_SPRITE, G_RM_XLU_SPRITE2);
	gSPObjRenderMode(glistp++, G_OBJRM_XLU | G_OBJRM_BILERP);

	// ******************************************

	// Init palette
	gDPSetPrimColor(glistp++, 0, 0, spr->r, spr->g, spr->b, spr->a);
	if (spr->pal != 0 && spr->pal != NULL)
	{
		if (spr->obj->s.imageFmt == G_IM_FMT_CI && spr->obj->s.imageSiz == G_IM_SIZ_4b)
			gDPLoadTLUT_pal16(glistp++, 0, spr->pal);
		if (spr->obj->s.imageFmt == G_IM_FMT_CI && spr->obj->s.imageSiz == G_IM_SIZ_8b)
			gDPLoadTLUT_pal256(glistp++, spr->pal);
	}

	// Draw each slice accordingly
	/*slice_y = 0;
	for (i = 0; i < slices; i++)
	{
		s2d_create(spr, obj, i, slice_y, slice_h);
		s2d_load(spr, 0, slice_y, spr->width - 1, slice_y + slice_h - 1);
		osWritebackDCache(obj, sizeof(uObjSprite));
		gSPObjSprite(glistp++, obj);
		gDPPipeSync(glistp++);

		slice_y += slice_h;
	}

	// ...and the remainder
	if (slices_ext)
	{
		s2d_create(spr, obj, i, slice_y, slices_ext);
		s2d_load(spr, 0, slice_y, spr->width - 1, slice_y + slices_ext - 1);
		osWritebackDCache(obj, sizeof(uObjSprite));
		gSPObjSprite(glistp++, obj);
		gDPPipeSync(glistp++);
	}*/

	// ******************************************

	// Translate X/Y
	spr->mtx->m.X = spr->x << 2;
	spr->mtx->m.Y = spr->y << 2;

	// Translate scale
	spr->mtx->m.A *= 1;
	spr->mtx->m.D *= 1;

	spr->mtx->m.BaseScaleX *= 1;
	spr->mtx->m.BaseScaleY *= 1;

	// Translate rotation
	/* spr->mtx->m.A = FTOFIX32(cosf(90));
	spr->mtx->m.B = FTOFIX32(sinf(90));
	spr->mtx->m.C = FTOFIX32(-sinf(90));
	spr->mtx->m.D = FTOFIX32(cosf(90)); */

	// Write matrix to DL
	gSPObjLoadTxtr(glistp++, spr->txtr);
	osWritebackDCache(spr->mtx, sizeof(uObjMtx));
	gSPObjMatrix(glistp++, spr->mtx);
	osWritebackDCache(spr->obj, sizeof(uObjSprite));
	gSPObjSprite(glistp++, spr->obj);
	gDPPipeSync(glistp++);
}

/*Sprite sprite_create
(
	void *img,
	int img_w,
	int img_h,
	int offset_x,
	int offset_y,
	int w,
	int h,
	u8 fmt,
	u8 pxl
)
{
}*/