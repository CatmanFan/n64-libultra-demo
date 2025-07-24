#include <ultra64.h>
#include "libultra-easy.h"

typedef struct
{
	int X;
	int Y;
	int W;
	int H;
	int tile_X;
	int tile_Y;
	int glyph;
} Character;

static u32 utf8_to_cp(const char **str)
{
	u8 *s = (u8 *)*str;
	u32 c = *s++;

	if (c < 0x80)
	{
		*str = (const char*)s;
		return c;
	}

	if (c < 0xC0)
	{
		*str = (const char*)s;
		return 0xFFFD;
	}

	if (c < 0xE0)
	{
		c = ((c & 0x1F) << 6) | (*s++ & 0x3F);
		*str = (const char*)s;
		return c;
	}

	if (c < 0xF0)
	{
		c = ((c & 0x0F) << 12); c |= ((*s++ & 0x3F) << 6); c |= (*s++ & 0x3F);
		*str = (const char*)s;
		return c;
	}

	if (c < 0xF8)
	{
		c = ((c & 0x07) << 18); c |= ((*s++ & 0x3F) << 12); c |= ((*s++ & 0x3F) << 6); c |= (*s++ & 0x3F);
		*str = (const char*)s;
		return c;
	}

	*str = (const char*)s;
	return 0xFFFD;
}

/*static int calculate_length(const char *txt)
{
	int i;
	for (i = 0; i < strlen(txt); i++)
		if (txt[i] == '\0')
			break;

	return i;
}*/

static int line_length(const char *txt)
{
	int count = 1;
	int i;
	for (i = 0; i < strlen(txt); i++)
		if (txt[i] == '\n')
			count++;

	return count;
}

void draw_text(const char *txt, Font *font, int x, int y, int bounds_w, int bounds_h, int align, int line_spacing, bool line_wrap, bool transparent)
{
	int chr;
	int max_length = strlen(txt);
	int globalX, globalY;
	Character chrs[max_length];

	int line_index = 0;
	int maxW[line_length(txt)];
	int maxH = 0;

	globalX = x;
	globalY = y;
	if (bounds_w == 0) { bounds_w = display_width() - x; }
	if (bounds_h == 0) { bounds_h = display_height() - y; }

	// Determine drawing positions of each character
	for (chr = 0; chr < max_length; chr++)
	{
		const char *p = txt + chr;
		bool is_new_line = p[0] == '\n';
		bool found = FALSE;

		if (p[0] == '\0') { break; }
		else
		{
			int index;
			u32 cp = utf8_to_cp(&p);

			for (index = 0; index < font->glyph_count; index++)
				if (font->glyphs[index].chr == cp)
				{
					found = TRUE;
					goto set_pos;
				}

			// Set default character to draw if none is found.
			// If the language is set to Japanese, preferably no empty space should be drawn
			// to avoid potential drawing spacing issues.
			#if LANGUAGE == JA
			index = -1;
			#else
			index = 0;
			#endif

			set_pos:
			chrs[chr].X = globalX;
			chrs[chr].Y = globalY;
			chrs[chr].W = index < 0 ? 0 : font->glyphs[index].width;
			chrs[chr].H = index < 0 ? 0 : font->glyphs[index].height;
			if (chrs[chr].H > maxH)
				{ maxH = chrs[chr].H; }

			if (!font->multi_bmp)
			{
				chrs[chr].glyph = index;
				chrs[chr].tile_X = index < 0 ? 0 : font->glyphs[index].x;
				chrs[chr].tile_Y = index < 0 ? 0 : font->glyphs[index].y;
			}
			else
			{
				chrs[chr].glyph = index < 0 ? -1 : font->glyphs[index].glyph_index;
				chrs[chr].tile_X = 0;
				chrs[chr].tile_Y = 0;
			}
		}

		if (found)
			{ globalX += chrs[chr].W; }

		if ((line_wrap && globalX >= x + bounds_w) || is_new_line)
		{
			if (globalX > maxW[line_index])
				{ maxW[line_index] = globalX - x; }
			line_index++;

			globalX = x;
			globalY += line_spacing;
			globalY += maxH > 0 ? maxH : chrs[0].H;
			maxH = 0;
		}

		if (line_wrap && globalY >= y + bounds_h)
		{
			globalY = y;
			maxH = 0;
		}
	}

	// Determine drawing positions of each character
	if (align == 1)
	{
		line_index = -1;

		for (chr = 0; chr < max_length; chr++)
		{
			if (chrs[chr].X == x)
				line_index++;
			chrs[chr].X = round((chrs[chr].X - ((f64)maxW[line_index] / 2.0F)) + ((f64)bounds_w / 2.0F));
		}
	}

	if (align == 2)
	{
		line_index = -1;

		for (chr = 0; chr < max_length; chr++)
		{
			if (chrs[chr].X == x)
				line_index++;
			chrs[chr].X = round(chrs[chr].X + (f64)bounds_w - ((f64)maxW[line_index] / 2.0F));
		}
	}

	// Initialize display list
	gDPPipeSync(glistp++);
	gDPSetCycleType(glistp++, G_CYC_1CYCLE);

	// G_CC_MODULATERGBA_PRIM is required for tinting, G_RM_XLU_SURF and G_CC_DECALRGBA required for transparency
	if (transparent)
	{
		gDPSetCombineMode(glistp++, G_CC_DECALRGBA, G_CC_DECALRGBA);
		gDPSetRenderMode(glistp++, G_RM_XLU_SURF, G_RM_XLU_SURF);
	}
	else
	{
		gDPSetCombineMode(glistp++, G_CC_DECALRGB, G_CC_DECALRGB);
		gDPSetRenderMode(glistp++, G_RM_NOOP, G_RM_NOOP);
	}
	gDPSetTexturePersp(glistp++, G_TP_NONE);

	if (transparent)
	{
		// Set blend color to transparent
		gDPSetBlendColor(glistp++, 0, 0, 0, 1);
		gDPSetAlphaCompare(glistp++, G_AC_THRESHOLD);
	}

	// Draw each character
	for (chr = 0; chr < max_length; chr++)
	{
		if (chrs[chr].glyph >= 0 && txt[chr] != ' ')
		{
			if (font->tlut != NULL)
			{
				gDPSetTextureLUT(glistp++, G_TT_RGBA16);
				gDPLoadTLUT_pal256(glistp++, font->tlut);
			}

			gDPLoadMultiTile
			(
				glistp++,
				font->multi_bmp ? font->bmp + chrs[chr].glyph * (font->glyphs[0].width * font->glyphs[0].height) : font->bmp,
				0,
				G_TX_RENDERTILE,
				G_IM_FMT_CI,
				G_IM_SIZ_8b,
				font->bmp_width,
				font->bmp_height,
				chrs[chr].tile_X,
				chrs[chr].tile_Y,
				chrs[chr].tile_X + chrs[chr].W,
				chrs[chr].tile_Y + chrs[chr].H,
				0,
				0,
				0,
				0,
				0,
				0,
				0
			);

			gSPTextureRectangle
			(
				glistp++,
				chrs[chr].X << 2,
				chrs[chr].Y << 2,
				(chrs[chr].X + chrs[chr].W) << 2,
				(chrs[chr].Y + chrs[chr].H) << 2,
				G_TX_RENDERTILE,
				chrs[chr].tile_X << 5, chrs[chr].tile_Y << 5,
				1 << 10, 1 << 10
			);

			gDPPipeSync(glistp++);
			if (font->tlut != NULL) { gDPSetTextureLUT(glistp++, G_TT_NONE); }
		}
		else if (!transparent)
		{
			gDPSetFillColor(glistp++, GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1));
			gDPFillRectangle(glistp++, chrs[chr].X, chrs[chr].Y, chrs[chr].X + chrs[chr].W, chrs[chr].Y + chrs[chr].H);
		}
	}


	// Finalize display list
	if (transparent) { gDPSetAlphaCompare(glistp++, G_AC_NONE); }
	gDPPipeSync(glistp++);
}