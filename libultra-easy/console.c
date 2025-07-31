#include <ultra64.h>
#include <stdarg.h>

#include "config/global.h"
#include "config/video.h"

#include "libultra-easy/types.h"
#include "libultra-easy/display.h"
#include "libultra-easy/fault.h"
#include "libultra-easy/gfx.h"
#include "libultra-easy/rcp.h"

/* =============== ASSETS =============== */

#include "assets/fonts/terminus.h"
#include "assets/fonts/rm2003_ja.h"

/* ============= PROTOTYPES ============= */

extern int language;

/* ============= FUNCTIONS ============== */

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

static char buf[4096];

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

void console_clear()
{
	bzero(&buf, sizeof(buf));
}

void console_puts(const char *txt, ...)
{
	va_list args;
	va_start(args, txt);

	_Printf(&printf_handler, buf + strlen(buf), txt, args);

	va_end(args);

	if (strlen(txt) == 1 && txt[0] == '\n')
		return;

	buf[strlen(buf)] = '\n';
	buf[strlen(buf) + 1] = '\0';
}

/* =========== DRAWING (RAW) ============ */

void clear_screen_raw(FrameBuffer *framebuffer)
{
	if (framebuffer == NULL)
	{
		int fb;

		for (fb = 0; fb < CFB_COUNT; fb++)
		{
			bzero(framebuffers[fb].address, framebuffers[fb].size);
		}
	}

	else
	{
		bzero(framebuffer->address, framebuffer->size);
	}
}

void tint_screen_raw(FrameBuffer *framebuffer)
{
	if (framebuffer == NULL)
	{
		int i, fb;

		for (fb = 0; fb < CFB_COUNT; fb++)
		{
			// Set beginning CFB pointer
			#ifdef VIDEO_32BIT
			u32 *ptr = (u32 *)
			#else
			u16 *ptr = (u16 *) // "u32 *ptr = (u32 *)" increases the spacing of each pixel, so not recommended
			#endif
			framebuffers[fb].address;

			// Write color directly
			for (i = 0; i < framebuffers[fb].size; i++)
			{
			#ifdef VIDEO_32BIT
				*ptr = 0x00000000;
			#else
				*ptr = (((*ptr & 0xe738) >> 2) | 1);
			#endif
				ptr++;
			}
		}
	}

	else
	{
		int i;

		// Set beginning CFB pointer
		#ifdef VIDEO_32BIT
		u32 *ptr = (u32 *)
		#else
		u16 *ptr = (u16 *) // "u32 *ptr = (u32 *)" increases the spacing of each pixel, so not recommended
		#endif
		framebuffer->address;

		// Write color directly
		for (i = 0; i < display_width() * display_height(); i++)
		{
			#ifdef VIDEO_32BIT
			*ptr = 0x00000000;
			#else
			*ptr = (((*ptr & 0xe738) >> 2) | 1);
			#endif
			ptr++;
		}
	}
}

static void draw_glyph_raw(Font *font, int g, int x, int y, FrameBuffer *framebuffer)
{
	// Set font helper variables
	int w = g < 0 ? 0                           : font->glyphs[g].width;
	int h = g < 0 ? font->glyphs[0].height      : font->glyphs[g].height;
	int i = g < 0 ? font->glyphs[0].glyph_index : font->glyphs[g].glyph_index;
	// int scale = 1;

	if (framebuffer == NULL)
	{
		int fb;

		for (fb = 0; fb < CFB_COUNT; fb++)
		{
			// Set pointer X,Y coordinates
			int ptr_x, ptr_y;

			// Set beginning CFB pointer
			#ifdef VIDEO_32BIT
			u32 black = 0x00000000;
			u32 white = 0xFFFFFFFF;
			u32 *ptr = (u32 *)
			#else
			u16 black = 0x0000; // (((*ptr & 0xe738) >> 2) | 1)
			u16 white = 0xFFFF;
			u16 *ptr = (u16 *) // "u32 *ptr = (u32 *)" increases the spacing of each pixel, so not recommended
			#endif
			framebuffers[fb].address + (y * display_width()) + x;

			// Write color directly
			for (ptr_y = y; ptr_y < y + h; ptr_y++)
			{
				for (ptr_x = x; ptr_x < x + w; ptr_x++)
				{
					int chr_x = ptr_x - x;
					int chr_y = ptr_y - y;
					u8 *chr = (u8 *)(font->bmp + i * (w * h));

					*ptr = i >= 0 && chr[chr_x + (w * chr_y)] == 0x01
						 ? white : black;
					ptr++;
				}

				ptr += display_width() - w;
			}
		}
	}
	else
	{
		// Set pointer X,Y coordinates
		int ptr_x, ptr_y;

		// Set beginning CFB pointer
		#ifdef VIDEO_32BIT
		u32 black = 0x00000000;
		u32 white = 0xFFFFFFFF;
		u32 *ptr = (u32 *)
		#else
		u16 black = 0x0000; // (((*ptr & 0xe738) >> 2) | 1)
		u16 white = 0xFFFF;
		u16 *ptr = (u16 *) // "u32 *ptr = (u32 *)" increases the spacing of each pixel, so not recommended
		#endif
		framebuffer->address + (y * display_width()) + x;

		// Write color directly
		for (ptr_y = y; ptr_y < y + h; ptr_y++)
		{
			for (ptr_x = x; ptr_x < x + w; ptr_x++)
			{
				int chr_x = ptr_x - x;
				int chr_y = ptr_y - y;
				u8 *chr = (u8 *)(font->bmp + i * (w * h));

				*ptr = i >= 0 && chr[chr_x + (w * chr_y)] == 0x01
					 ? white : black;
				ptr++;
			}

			ptr += display_width() - w;
		}
	}
}

static void draw_text_raw(int x, int y, const char *txt, Font font, FrameBuffer *fb)
{
	int text_x = x * font.glyphs[0].width, text_y = y * font.glyphs[0].height;
	int i = 0;
	const char *chr;

	x = text_x;
	y = text_y;
	chr = txt;

	for (i = 0; i < strlen(txt); i++)
	{
		if (*chr == '\0') { return; }
		else if (*chr == '\n') { x = text_x; y += font.glyphs[0].height; chr++; }
		else
		{
			bool found = FALSE;
			u32 cp = utf8_to_cp(&chr);

			int g;
			for (g = 0; g < 500; g++)
				if (font.glyphs[g].chr == cp)
				{
					found = TRUE;
					goto draw;
				}

			// Set default character to draw if none is found.
			// If the language is set to Japanese, preferably no empty space should be drawn
			// to avoid potential drawing spacing issues.
			g = language == 6 ? -1 : 0;

			draw:
			draw_glyph_raw(&font, g, x, y, fb);

			if (found) x += font.glyphs[g].width;
			if (x > display_width() - text_x) { x = text_x; y += font.glyphs[g].height; }
			if (y >= display_height() - font.glyphs[0].height - font.glyphs[g].height) { y = text_y; }
		}
	}
}

/* =========== DRAWING (DLs) ============ */

static void draw_text_dl(int x, int y, int scale, const char *txt, Font *font)
{
	extern void draw_text(const char *txt, Font *font, int x, int y, int bounds_w, int bounds_h, int align, int line_spacing, bool line_wrap, bool transparent);
	x *= font->glyphs[0].width;
	y *= font->glyphs[0].height;
	draw_text(txt, font, x, y, display_width() - x * 2, display_height() - y * 2, 0, 0, TRUE, FALSE);
}

/* ============== GLOBAL =============== */

void console_draw_dl()
{
	int font = language == 6 ? 1 : 0;
	draw_text_dl
	(
		font == 1 ? 1 : 2,
		1,
		1,
		buf,
		font == 1 ? &rm2003_ja : &terminus
	);
}

void console_draw_raw(FrameBuffer *fb)
{
	draw_text_raw
	(
		language == 6 ? 1 : 2,
		1,
		buf,
		language == 6 ? rm2003_ja : terminus,
		fb
	);
}