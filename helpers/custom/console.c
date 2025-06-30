#include <ultra64.h>
#include <stdarg.h>

#include "config.h"
#include "strings.h"

#include "helpers/types.h"

/* =============== ASSETS =============== */

#include "assets/fonts/terminus.h"
#include "assets/fonts/rm2003_ja.h"

/* ============= PROTOTYPES ============= */

extern void *cfb[];
extern int cfb_current;

/* ============= FUNCTIONS ============== */

u32 utf8_to_cp(const char **str)
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

extern int _Printf(void *(*copyfunc)(void *, const char *, size_t), void*, const char*, va_list);

/*==============================
	printf_handler
	Handles printf memory copying
	@param The buffer to copy the partial string to
	@param The string to copy
	@param The length of the string
	@returns The end of the buffer that was written to
==============================*/
static void* printf_handler(void *buf, const char *str, size_t len)
{
	return ((char *) memcpy(buf, str, len) + len);
}

static char buf[0x108];

void console_clear()
{
	bzero(&buf, sizeof(buf));
}

void console_printf(const char *txt, ...)
{
	va_list args;
	va_start(args, txt);

	_Printf(&printf_handler, buf, txt, args);

	va_end(args);
}

/* =========== DRAWING (RAW) ============ */

static void draw_glyph_raw(Glyph *glyphs, int font_size, u8 font[][font_size], int g, int x, int y)
{
	// Set font helper variables
	int w = g < 0 ? 0                     : glyphs[g].width;
	int h = g < 0 ? glyphs[0].height      : glyphs[g].height;
	int i = g < 0 ? glyphs[0].glyph_index : glyphs[g].glyph_index;
	// int scale = 1;

	// Set pointer X,Y coordinates
	int ptr_x, ptr_y;

	// Set index of target CFB and begin loop
	int cfb_index;
	for (cfb_index = 0; cfb_index < 2; cfb_index++)
	{
		// Set beginning CFB pointer
		u16 *ptr = (u16 *) // "u32 *ptr = (u32 *)" increases the spacing of each pixel, so not recommended
		cfb[cfb_index] + (y * SCREEN_W) + x;

		// Write color directly
		for (ptr_y = y; ptr_y < y + h; ptr_y++)
		{
			for (ptr_x = x; ptr_x < x + w; ptr_x++)
			{
				int chr_x = ptr_x - x;
				int chr_y = ptr_y - y;

				*ptr = i >= 0 && font[i][chr_x + (w * chr_y)] ? 0xFFFF
															  : (((*ptr & 0xe738) >> 2) | 1);
				ptr++;
			}

			ptr += SCREEN_W - w;
		}
	}
}

static void draw_text_raw(int x, int y, const char *txt, Glyph* glyphs, int font_size, u8 font[][font_size])
{
	int text_x = x * glyphs[0].width, text_y = y * glyphs[0].height;
	int i = 0;
	const char *chr;

	x = text_x;
	y = text_y;
	chr = txt;

	for (i = 0; i < strlen(txt); i++)
	{
		if (*chr == '\0') { return; }
		else if (*chr == '\n') { x = text_x; y += glyphs[0].height; chr++; }
		else
		{
			bool found = FALSE;
			u32 cp = utf8_to_cp(&chr);

			int g;
			for (g = 0; g < 500; g++)
				if (glyphs[g].chr == cp)
				{
					found = TRUE;
					goto draw;
				}

			// Set default character to draw if none is found.
			// If the language is set to Japanese, preferably no empty space should be drawn
			// to avoid potential drawing spacing issues.
			#if LANGUAGE == JA
			g = -1;
			#else
			g = 0;
			#endif

			draw:
			draw_glyph_raw(glyphs, font_size, font, g, x, y);

			if (found) x += glyphs[g].width;
			if (x > SCREEN_W - text_x) { x = text_x; y += glyphs[g].height; }
		}
	}
}

void console_draw_raw()
{
	draw_text_raw
	(
		language == 7 ? 1 : 2,
		1,
		buf,
		language == 7 ? rm2003_ja_glyphs : terminus_glyphs,
		language == 7 ? 12 * 12 : 6 * 12,
		language == 7 ? rm2003_ja_img : terminus_img
	);

    osWritebackDCacheAll();
	osViSwapBuffer(cfb[0]);
}

/* =========== DRAWING (DLs) ============ */

static Gfx *draw_text_dl(Gfx *dl, int x, int y, int scale, const char *txt, Glyph* glyphs, int font_size, u8 font[][font_size])
{
	int text_x = x * glyphs[0].width, text_y = y * glyphs[0].height;
	int c;
	const char *chr;

	int w, h, i;

	x = text_x;
	y = text_y;
	chr = txt;

	// Initialize display list
	gDPPipeSync(dl++);
	gDPSetCycleType(dl++, G_CYC_COPY);
	gDPSetCombineMode(dl++, G_CC_DECALRGB, G_CC_DECALRGB);
	gDPSetRenderMode(dl++, G_RM_NOOP, G_RM_NOOP2);
	gDPSetTexturePersp(dl++, G_TP_NONE);

	// Set blend color to transparent
	// gDPSetBlendColor(dl++, 0, 0, 0, 1);
	// gDPSetAlphaCompare(dl++, G_AC_THRESHOLD);

	for (c = 0; c < strlen(txt); c++)
	{
		if (*chr == '\0') { goto end; }
		else if (*chr == '\n') { x = text_x; y += glyphs[0].height * scale; chr++; }
		else
		{
			bool found = FALSE;
			u32 cp = utf8_to_cp(&chr);

			int g;
			for (g = 0; g < 500; g++)
				if (glyphs[g].chr == cp)
				{
					found = TRUE;
					goto draw;
				}

			// Set default character to draw if none is found.
			// If the language is set to Japanese, preferably no empty space should be drawn
			// to avoid potential drawing spacing issues.
			#if LANGUAGE == JA
			g = -1;
			#else
			g = 0;
			#endif

			draw:
			// Set font helper variables
			w = g < 0 ? 0                     : glyphs[g].width * scale;
			h = g < 0 ? glyphs[0].height      : glyphs[g].height * scale;
			i = g < 0 ? glyphs[0].glyph_index : glyphs[g].glyph_index;

			gDPSetFillColor(dl++, GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1));
			gDPFillRectangle(dl++, x, y, x + w, y + h);

			if (g >= 0 && glyphs[g].chr != ' ')
			{
				gDPLoadTextureTile(dl++, font[i], G_IM_FMT_I, G_IM_SIZ_8b, w, h, 0, 0, w - 1, h - 1, 0, 0, 0, 0, 0, 0, 0);
				gSPTextureRectangle
				(
					dl++,
					x << 2,
					y << 2,
					(x + w - 1) << 2,
					(y + h - 1) << 2,
					G_TX_RENDERTILE,
					0 << 5, 0 << 5,
					4 << 10, 1 << 10
				);
			}

			if (found) x += w;
			if (x > SCREEN_W - text_x) { x = text_x; y += h; }
		}
	}

	end:
	// Finalize display list
	// gDPSetAlphaCompare(dl++, G_AC_NONE);
	gDPPipeSync(dl++);

	return dl;
}

Gfx *console_draw_dl(Gfx *dl)
{
	return draw_text_dl
	(
		dl,
		language == 7 ? 1 : 2,
		1,
		1,
		buf,
		language == 7 ? rm2003_ja_glyphs : terminus_glyphs,
		language == 7 ? 12 * 12 : 6 * 12,
		language == 7 ? rm2003_ja_img : terminus_img
	);
}