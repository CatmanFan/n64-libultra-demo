#include <ultra64.h>
#include <stdarg.h>
#include "helpers/types.h"
#include "config.h"

extern void *cfb[];
extern int cfb_current;

#include "assets/fonts/terminus.h"
#include "assets/fonts/rm2003.h"

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

static void draw_glyph(Glyph glyphs[], int g, int n, u8 img[][n], int x, int y)
{
	// Set font helper variables
	int w = glyphs[g].width;
	int h = glyphs[g].height;
	int i = glyphs[g].glyph_index;
	int chr_x = 0, chr_y = 0;

	// Set pointer X,Y coordinates
	int ptr_x, ptr_y;

	// Set beginning CFB pointer
	u16 *ptr = (u16 *) // "u32 *ptr = (u32 *)" increases the spacing of each pixel, so not recommended
	cfb[cfb_current] + (y * SCREEN_W) + x;

	// Write color directly
	for (ptr_y = y; ptr_y < y + h; ptr_y++)
	{
		for (ptr_x = x; ptr_x < x + w; ptr_x++)
		{
            *ptr = i >= 0 && img[i][chr_x + (w * chr_y)] ? 0xFFFF : (((*ptr & 0xe738) >> 2) | 1);
            ptr++;
			chr_x++;
		}

		ptr += SCREEN_W - w;

		chr_y++;
		chr_x = 0;
	}
}

void draw_text(const char *txt, int x, int y)
{
	int text_x = x * terminus_glyphs[' '].width, text_y = y * terminus_glyphs[' '].height;
	int i = 0;
	const char *chr;

	x = text_x;
	y = text_y;
	chr = txt;

	for (i = 0; i < strlen(txt); i++)
	{
		if (*chr == '\0') { return; }
		else if (*chr == '\n') { x = text_x; y += terminus_glyphs[0].height; chr++; }
		else
		{
			u32 cp = utf8_to_cp(&chr);

			int g = 0;
			for (g = 0; g < 105; g++)
				if (terminus_glyphs[g].chr == cp)
					goto draw;
			g = ' ';

			draw:
			draw_glyph(terminus_glyphs, g, 72, terminus_img, x, y);

			x += terminus_glyphs[g].width;
			if (x > SCREEN_W - text_x) { x = text_x; y += terminus_glyphs[g].height; }
		}
	}
}