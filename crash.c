#include <ultra64.h>
#include <PR/os_internal_error.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "config.h"
#include "helpers/gfx.h"

extern OSMesgQueue msgQ_crash;

extern void *cfb[];
extern int cfb_current;

static u8 update_fb = FALSE;
OSThread *thread;
__OSThreadContext *tc;

const char *const gCauseDesc[18] = {
    "Interrupt",
    "TLB modification",
    "TLB exception on load",
    "TLB exception on store",
    "Address error on load",
    "Address error on store",
    "Bus error on inst.",
    "Bus error on data",
    "System call exception",
    "Breakpoint exception",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap exception",
    "Virtual coherency on inst.",
    "Floating point exception",
    "Watchpoint exception",
    "Virtual coherency on data",
};

#define LANGUAGE 2

#if   LANGUAGE == 1
#define STR_FRAME	"Image n° %d"
#define STR_REBOOT	"Redémarrage en cours..."
#define STR_ERROR	"Erreur grave\nau thread n° %d (priorité fixée à %d)\n"\
					"Message : %s\n\n"\
					"Le logiciel a été terminé."
#elif LANGUAGE == 2
#define STR_FRAME	"Fotograma %d"
#define STR_REBOOT	"Se reiniciará el programa."
#define STR_ERROR	"Error grave\nen el subproceso n° %d (prioridad fijada a %d)\n"\
					"Mensaje: %s\n\n"\
					"Se ha cerrado el programa."
#elif LANGUAGE == 3
#define STR_FRAME	"Bild %d"
#define STR_REBOOT	"Die Software wird neu gestartet."
#define STR_ERROR	"Schwerwiegender Fehler\nam Thread %d (Priorität: %d)\n"\
					"Nachricht: %s\n\n"\
					"Die Software wurde beendet."
#elif LANGUAGE == 4
#define STR_FRAME	"Kare: %d"
#define STR_REBOOT	"Yazılım yeniden başlamıyor."
#define STR_ERROR	"%d. İş Parçacığında Kritik Hata\n"\
					"İş Parçacığı Önceliği: %d\n"\
					"Hata Mesajı: %s\n\n"\
					"Yazılım kapatıldı."
#elif LANGUAGE == 5
#define STR_FRAME	"Кадр: %d"
#define STR_REBOOT	"Перезагрузка"
#define STR_ERROR	"Критическая ошибка\nв треде %d (приоритет: %d)\n"\
					"%s\n\n"\
					"Программа закрыта."
#elif LANGUAGE == 6
#define STR_FRAME	"フレーム %d"
#define STR_REBOOT	"さいきどうしています。\0"
#define STR_ERROR	"大変なエラー\n"\
					"スレッド %d (優先度: %d)\n"\
					"詳しく: %s\n\n"\
					"ソフトが終了しました。\0"
#else
#define STR_FRAME	"Frame %d"
#define STR_REBOOT	"Restarting..."
#define STR_ERROR	"Fatal error\nat thread %d (priority %d)\n"\
					"Message: %s\n\n"\
					"Software terminated."
#endif

extern void draw_text(const char *txt, int x, int y);

void init_crash()
{
	osViSetYScale(1.0);
	// osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	update_fb = TRUE;
	tc = &thread->context;

	for (;;)
	{
		if (update_fb)
		{
			char str[500];

			s16 cause = (tc->cause >> 2) & 0x1f;
			if (cause == 23) // EXC_WATCH
				cause = 16;
			if (cause == 31) // EXC_VCED
				cause = 17;

			sprintf
			(
				str,
				STR_ERROR,
				(int)thread->id,
				(int)thread->priority,
				gCauseDesc[cause]
			);

			draw_text(str, 2, 1);
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