#include <ultra64.h>
#include <PR/os_internal_error.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "config.h"
#include "helpers/gfx.h"

#include "assets/textures/terminus.h"

extern OSMesgQueue msgQ_crash;

extern void *cfb[];
extern int cfb_current;
static u16 crash_w;
static u16 crash_h;

static u8 update_fb = FALSE;
OSThread *thread;

static OSThread *get_crashed_thread(void)
{
	OSThread *t = __osGetCurrFaultedThread();

    while (t->priority != -1) {
        if (t->priority > OS_PRIORITY_IDLE && t->priority < OS_PRIORITY_APPMAX
            && ((t->flags & ((1 << (0)) | (1 << (1)))) != 0)) {
            return t;
        }
        t = t->tlnext;
    }
	
    return NULL;
}

extern u8 rm2003_img[][];

Glyph rm2003_glyphs[] =
{
	{' ', -1, 12, 12 },
	{0x3041, 0 + 0,  12, 12 },
	{0x3042, 0 + 1,  12, 12 },
	{0x3043, 0 + 2,  12, 12 },
	{0x3044, 0 + 3,  12, 12 },
	{0x3045, 0 + 4,  12, 12 },
	{0x3046, 0 + 5,  12, 12 },
	{0x3047, 0 + 6,  12, 12 },
	{0x3048, 0 + 7,  12, 12 },
	{0x3049, 0 + 8,  12, 12 },
	{0x304A, 0 + 9,  12, 12 },
	{0x304B, 0 + 10, 12, 12 },
	{0x304C, 0 + 11, 12, 12 },
	{0x304D, 0 + 12, 12, 12 },
	{0x304E, 0 + 13, 12, 12 },
	{0x304F, 0 + 14, 12, 12 },
	{0x3050, 0 + 15, 12, 12 },
	{0x3051, 0 + 16, 12, 12 },
	{0x3052, 0 + 17, 12, 12 },
	{0x3053, 0 + 18, 12, 12 },
	{0x3054, 0 + 19, 12, 12 },
	{0x3055, 0 + 20, 12, 12 },
};

extern u32 utf8_to_cp(const char **str);

static void draw_glyph(Glyph glyphs[], int g, int n, u8 img[][n], u32 x, u32 y)
{
	// Set font helper variables
	int w = glyphs[g].width;
	int h = glyphs[g].height;
	int i = glyphs[g].glyph_index;
	int chr_x = 0, chr_y = 0;

	// Set pointer X,Y coordinates
	int ptr_x, ptr_y;

	// Set beginning CFB pointer
	#if (SCREEN_W == 640 && SCREEN_H == 480)
	u32 *ptr = (u32 *)
	#else
	u16 *ptr = (u16 *)
	#endif
	cfb[cfb_current] + (y * crash_w) + x;

	// Write color directly
	for (ptr_y = y; ptr_y < y + h; ptr_y++)
	{
		for (ptr_x = x; ptr_x < x + w; ptr_x++)
		{
            *ptr = i >= 0 && img[i][chr_x + (w * chr_y)] ? 0xFFFF : (((*ptr & 0xe738) >> 2) | 1);
            ptr++;
			chr_x++;
		}

		ptr += crash_w - w;

		chr_y++;
		chr_x = 0;
	}
}

// __attribute__((format (printf, 4, 5)))
void draw_text(Glyph glyphs[], int n, u8 img[][n], const char *txt, ...)
{
    va_list args;

	int text_x = 20, text_y = 20;
	int x = text_x, y = text_y;

	int i = 0;
	const char *chr;
	char str[300];

    va_start(args, txt);
	sprintf(str, txt, args);
	chr = str;

	for (i = 0; i < strlen(str); i++)
	{
		if (*chr == '\0') { goto end; }
		else if (*chr == '\n') { x = text_x; y += glyphs[0].height; chr++; }
		else
		{
			u32 cp = utf8_to_cp(&chr);

			int g = 0;
			while (glyphs[g].chr != cp) g++;
			draw_glyph(glyphs, g, n, img, x, y);

			// draw_rectangle(x, y, x + glyphs[g].width, y + glyphs[g].height);

			x += glyphs[g].width;
			if (x > crash_w - text_x) { x = text_x; y += glyphs[g].height; }
		}
	}

	end:
	va_end(args);
}

void init_crash()
{
    crash_w = SCREEN_W;
    crash_h = SCREEN_H;

	osViSetYScale(1.0);
	// osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	update_fb = TRUE;

	for (;;)
	{
		
		if (update_fb)
		{
			int lang = 0;

			char *str0 =
				lang == 1 ? "Erreur grave\n" :
				lang == 2 ? "Schwerwiegender Fehler\n" :
				lang == 3 ? "Error critical\n" :
				lang == 4 ? "Kritik Hata\n" :
				lang == 5 ? "Критическая ошибка\n" :
				lang == 6 ? "たいへんな　エラー\n" :
				"Fatal error\n";

			char *str1 =
				lang == 1 ? "Le logiciel a été terminé.\n\n" :
				lang == 2 ? "Die Software wurde beendet.\n\n" :
				lang == 3 ? "Se ha cerrado el programa.\n\n" :
				lang == 4 ? "Yazılım kapatıldı.\n\n" :
				lang == 5 ? "Программа закрыта.\n\n" :
				lang == 6 ? "ソフトが しゅうりょう　しました。\n\n" :
				"Software terminated.\n\n";

			char *str2 =
				lang == 1 ? "Adresse :  0x%8x\n" :
				lang == 2 ? "Adresse:   0x%8x\n" :
				lang == 3 ? "Hilo:      0x%8x\n" :
				lang == 4 ? "Adresi:   0x%8x\n" :
				lang == 5 ? "Адрес:         0x%8x\n" :
				lang == 6 ? "アドレス:  0x%8x\n" :
				"Address:  0x%8x\n";

			char *str3 =
				lang == 1 ? "ID :       %ld\n" :
				lang == 2 ? "ID:        %ld\n" :
				lang == 3 ? "ID:        %ld\n" :
				lang == 4 ? "Kimliği:  %ld\n" :
				lang == 5 ? "Идентификатор: %ld\n" :
				lang == 6 ? "ID:    %ld\n" :
				"ID:       %ld\n";

			char *str4 =
				lang == 1 ? "Priorité : %ld\n\n" :
				lang == 2 ? "Priorität: %ld\n\n" :
				lang == 3 ? "Prioridad: %ld\n\n" :
				lang == 4 ? "Önceliği: %ld\n\n" :
				lang == 5 ? "Приоритет:     %ld\n\n" :
				lang == 6 ? "ゆうせんど: %ld\n\n" :
				"Priority: %ld\n\n";

			// draw_rectangle(20, 20, SCREEN_W - 40, SCREEN_H - 40);

			draw_text
			(
				lang == 6 ? rm2003_glyphs : terminus_glyphs,
				lang == 6 ? 100           : 72,
				lang == 6 ? rm2003_img    : terminus_img,
				lang == 6 ? "あいうえお\0" : strcat(strcat(strcat(strcat(str0, str1), str2), str3), str4),
				thread, &thread->id, &thread->priority
			);
			osWritebackDCacheAll();
			osViBlack(0);
			osViSwapBuffer(cfb[cfb_current]);

			update_fb = FALSE;
		}
	}
}

void crash(void *arg)
{
	OSMesg msg;

    osSetEventMesg(OS_EVENT_CPU_BREAK, &msgQ_crash, (OSMesg)1);
    osSetEventMesg(OS_EVENT_FAULT, &msgQ_crash, (OSMesg)2);
	
    thread = (OSThread *)NULL;
	while (1)
	{
		if (thread == NULL)
		{
			// Wait until fault message is received
			(void) osRecvMesg(&msgQ_crash, (OSMesg *)&msg, OS_MESG_BLOCK);

			// Identify faulted thread
			thread = __osGetCurrFaultedThread();

			// Initiate crash screen process if fault is detected
			if (thread)
			{
				init_crash();
				return;
			}
		}
	}
}