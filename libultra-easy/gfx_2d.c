#include <ultra64.h>
#include <stdlib.h>
#include <math.h>
#include <PR/gs2dex.h>
#include <PR/gu.h>

#include "config/global.h"
#include "config/video.h"
#include "config/usb.h"

#include "libultra-easy/types.h"
#include "libultra-easy/fault.h"
#include "libultra-easy/fs.h"
#include "libultra-easy/rcp.h"

/* =========== STATIC HELPERS =========== */

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
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsSPEndDisplayList(),
};

static int find_nearest_multiple(int input)
{
	int multiple_array[] = { 2, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 64, 72, 76, 100, 108, 128, 144, 152, 164, 200, 216, 228, 256, 304, 328, 432, 456, 512, 684, 820, 912 };
	int multiple_i;

	while (multiple_array[multiple_i] < input && multiple_i < 32)
		multiple_i++;

	return multiple_array[multiple_i];
}

static void copy_img(void *img, int img_w, void *dest, int dest_w, int w, int h)
{
	int dest_x, dest_y;
    bzero(dest, sizeof(dest));

	for (dest_y = 0; dest_y < h; dest_y++)
	{
		// memcpy(dest + (dest_y * dest_w), img + (dest_y * img_w), w);
	    int *ptr = dest + (dest_w * dest_y);
		for (dest_x = 0; dest_x < w; dest_x++)
		{
		    ptr = img + (dest_y * img_w) + dest_x;
		    ptr++;
		}
	}
}

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
	rcp_init_rdp();
	rcp_init_rsp();
}

/* void sprite_draw_bg(uObjBg *bg, int x, int y, float scale_x, float scale_y, u8 r, u8 g, u8 b, u8 a)
{
	gSPBgRect1Cyc(glistp++, &bg);
} */

void setup_mtx(uObjMtx *buf, int x, int y, int scale)
{
	buf->m.A = 0x10000;
	buf->m.D = 0x10000;

	buf->m.X = x << 2;
	buf->m.Y = y << 2;

	buf->m.BaseScaleX = scale << 10;
	buf->m.BaseScaleY = scale << 10;
}

void sprite_create_tlut(sprite_t *spr, void *img, u16 *tlut, int w, int h, int frames, u8 img_siz)
{
	// Set texture entry properties
	spr->data.tex = img;

	// Set TLUT properties
	spr->data.tlut = tlut;
	spr->data.tlut_fmt = G_TT_RGBA16;

	// Assign properties to pointers
	spr->data.w = w;
	spr->data.h = h;
	spr->data.frames = frames;
	spr->data.img_fmt = G_IM_FMT_CI;
	spr->data.img_siz = img_siz;

	// Set properties
	spr->r = 255;
	spr->g = 255;
	spr->b = 255;
	spr->a = 255;
	spr->scale_x = 1;
	spr->scale_y = 1;
}

void sprite_create(sprite_t *spr, void *img, int w, int h, int frames, u8 fmt, u8 siz)
{
	// Set texture entry properties
	spr->data.tex = img;

	// Determine that this sprite does not use a TLUT palette
	spr->data.tlut = NULL;

	// Assign properties to pointers
	spr->data.w = w;
	spr->data.h = h;
	spr->data.frames = frames;
	spr->data.img_fmt = fmt;
	spr->data.img_siz = siz;

	// Set properties
	spr->r = 255;
	spr->g = 255;
	spr->b = 255;
	spr->a = 255;
	spr->scale_x = 1;
	spr->scale_y = 1;
}

void sprite_destroy(sprite_t *spr)
{
	// free(spr);
}

void sprite_draw(sprite_t *spr)
{
	bool is_4b = spr->data.img_siz == G_IM_SIZ_4b;
	int texels_per_pixel = spr->data.img_siz == G_IM_SIZ_32b ? 1024
						 : spr->data.img_siz == G_IM_SIZ_16b ? 2048
						 : spr->data.img_siz == G_IM_SIZ_8b ? spr->data.img_fmt == G_IM_FMT_CI ? 2048 : 4096
						 : spr->data.img_fmt == G_IM_FMT_CI ? 4096 : 8192;

    s32 sx = (int) ((1<<10) / spr->scale_x + 0.5F);
    s32 sy = (int) ((1<<10) / spr->scale_y + 0.5F);

	// ******************************************
	gDPSetCombineMode(glistp++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
	gDPSetRenderMode(glistp++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF);
	gSPObjRenderMode(glistp++, G_OBJRM_XLU | G_OBJRM_BILERP);
	gDPSetPrimColor(glistp++, 0, 0, spr->r, spr->g, spr->b, spr->a);

	if (spr->data.tlut != NULL)
	{
		gDPSetTextureLUT(glistp++, G_TT_RGBA16);
		gDPLoadTLUT_pal256(glistp++, spr->data.tlut);
	}

	if (spr->data.w * spr->data.h > texels_per_pixel)
	{
		int i, j;
		int slice_width = spr->data.w;
		int slice_height = 1;
		my_assert(slice_width <= texels_per_pixel, "Sprite is too large");

		for (i = 0; i < spr->data.h; i += slice_height)
		{
			if (slice_height > spr->data.h - i)
				slice_height = spr->data.h - i;

			for (j = 0; j < spr->data.w; j += slice_width)
			{
				if (slice_width > spr->data.w - j)
					slice_width = spr->data.w - j;

				// Load texture as tile
				if (is_4b)
				{
					gDPLoadMultiTile_4b(
						glistp++,
						spr->data.frames > 1 ? (((u32 **)spr->data.tex)[spr->frame])
											 : ((u32 *)spr->data.tex),	// timg - Our sprite array
						0,												// tmem - Address to store in TMEM
						G_TX_RENDERTILE,								// rt - Tile descriptor

						// fmt - Our image format
						spr->data.img_fmt == G_IM_FMT_RGBA ? G_IM_FMT_RGBA :
						spr->data.img_fmt == G_IM_FMT_YUV ? G_IM_FMT_YUV :
						spr->data.img_fmt == G_IM_FMT_CI ? G_IM_FMT_CI :
						spr->data.img_fmt == G_IM_FMT_IA ? G_IM_FMT_IA :
						spr->data.img_fmt == G_IM_FMT_I ? G_IM_FMT_I :
						0,

						spr->data.w, spr->data.h,						// width, height of the full image

						// Top left corner of the rectangle
						j,
						i,

						// Bottom right corner
						j + slice_width - 1,
						i + slice_height - 1,

						0,												// Palette to use (always 0)
						G_TX_WRAP, G_TX_WRAP,							// cms, cmt
						0, 0,											// masks, maskt
						G_TX_NOLOD, G_TX_NOLOD							// shifts, shiftt
					);
				}

				else
				{
					gDPLoadMultiTile(
						glistp++,
						spr->data.frames > 1 ? (((u32 **)spr->data.tex)[spr->frame])
											 : ((u32 *)spr->data.tex),	// timg - Our sprite array
						0,												// tmem - Address to store in TMEM
						G_TX_RENDERTILE,								// rt - Tile descriptor

						// fmt - Our image format
						spr->data.img_fmt == G_IM_FMT_RGBA ? G_IM_FMT_RGBA :
						spr->data.img_fmt == G_IM_FMT_YUV ? G_IM_FMT_YUV :
						spr->data.img_fmt == G_IM_FMT_CI ? G_IM_FMT_CI :
						spr->data.img_fmt == G_IM_FMT_IA ? G_IM_FMT_IA :
						spr->data.img_fmt == G_IM_FMT_I ? G_IM_FMT_I :
						0,

						// size - Pixel size
						spr->data.img_siz == G_IM_SIZ_32b ? G_IM_SIZ_32b :
						spr->data.img_siz == G_IM_SIZ_16b ? G_IM_SIZ_16b :
						G_IM_SIZ_8b,

						spr->data.w, spr->data.h,						// width, height of the full image

						// Top left corner of the rectangle
						j,
						i,

						// Bottom right corner
						j + slice_width - 1,
						i + slice_height - 1,

						0,												// Palette to use (always 0)
						G_TX_WRAP, G_TX_WRAP,							// cms, cmt
						0, 0,											// masks, maskt
						G_TX_NOLOD, G_TX_NOLOD							// shifts, shiftt
					);
				}

				// Draw a rectangle
				gSPTextureRectangle(glistp++, 
					(spr->x + round(j * spr->scale_x))<<2,
					(spr->y + round(i * spr->scale_y))<<2,
					(spr->x + round((j+slice_width) * spr->scale_x))<<2,
					(spr->y + round((i+slice_height) * spr->scale_y))<<2,
					G_TX_RENDERTILE,          // Tile descriptor
					j << 5, i << 5,           // Starting S T Coordinate
					sx, sy                    // S T Increment
				);
			}
		}
	}
	else
	{
		// Load texture as block
		if (is_4b)
		{
			gDPLoadTextureBlock_4b(
				glistp++,
				spr->data.frames > 1 ? (((u32 **)spr->data.tex)[spr->frame])
									 : ((u32 *)spr->data.tex),	// timg - Our sprite array

				// fmt - Our image format
				spr->data.img_fmt == G_IM_FMT_RGBA ? G_IM_FMT_RGBA :
				spr->data.img_fmt == G_IM_FMT_YUV ? G_IM_FMT_YUV :
				spr->data.img_fmt == G_IM_FMT_CI ? G_IM_FMT_CI :
				spr->data.img_fmt == G_IM_FMT_IA ? G_IM_FMT_IA :
				spr->data.img_fmt == G_IM_FMT_I ? G_IM_FMT_I :
				0,

				spr->data.w, spr->data.h,						// width, height of the full image
				0,												// Palette to use (always 0)
				G_TX_CLAMP, G_TX_CLAMP,							// cms, cmt
				G_TX_NOMASK, G_TX_NOMASK,						// masks, maskt
				G_TX_NOLOD, G_TX_NOLOD							// shifts, shiftt
			);
		}

		else
		{
			gDPLoadTextureBlock(
				glistp++,
				spr->data.frames > 1 ? (((u32 **)spr->data.tex)[spr->frame])
									 : ((u32 *)spr->data.tex),	// timg - Our sprite array

				// fmt - Our image format
				spr->data.img_fmt == G_IM_FMT_RGBA ? G_IM_FMT_RGBA :
				spr->data.img_fmt == G_IM_FMT_YUV ? G_IM_FMT_YUV :
				spr->data.img_fmt == G_IM_FMT_CI ? G_IM_FMT_CI :
				spr->data.img_fmt == G_IM_FMT_IA ? G_IM_FMT_IA :
				spr->data.img_fmt == G_IM_FMT_I ? G_IM_FMT_I :
				0,

				// size - Pixel size
				spr->data.img_siz == G_IM_SIZ_32b ? G_IM_SIZ_32b :
				spr->data.img_siz == G_IM_SIZ_16b ? G_IM_SIZ_16b :
				G_IM_SIZ_8b,

				spr->data.w, spr->data.h,						// width, height of the full image
				0,												// Palette to use (always 0)
				G_TX_CLAMP, G_TX_CLAMP,							// cms, cmt
				G_TX_NOMASK, G_TX_NOMASK,						// masks, maskt
				G_TX_NOLOD, G_TX_NOLOD							// shifts, shiftt
			);
		}

		// Draw a rectangle
		gSPTextureRectangle(glistp++, 
			(spr->x)<<2,
			(spr->y)<<2,
			(spr->x + round(spr->data.w * spr->scale_x))<<2,
			(spr->y + round(spr->data.h * spr->scale_y))<<2,
			G_TX_RENDERTILE,		// Tile descriptor
			0 << 5, 0 << 5,			// Starting S T Coordinate
			sx, sy					// S T Increment
		);
	}

	gDPPipeSync(glistp++);
	if (spr->data.tlut != NULL)
	{
		gDPSetTextureLUT(glistp++, G_TT_NONE);
	}
}