#include <ultra64.h>
#include <PR/os_internal_error.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

/* === Configuration === */
#include "config/global.h"

/* === Default libraries === */
#include "libraries/types.h"
#include "libraries/gfx.h"

/* === Custom libraries === */
#include "libraries/custom/console.h"
#include "libraries/custom/strings.h"

/* ============= PROTOTYPES ============= */

extern OSMesgQueue msgQ_crash;

static bool update_fb = FALSE;
OSThread *thread;
__OSThreadContext *tc;

/* ========== STATIC VARIABLES ========== */

static const char *const gCauseDesc[18] = {
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

/* ========== STATIC FUNCTIONS ========== */

static void init_crash()
{
	my_osViBlack(TRUE);
	// osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	update_fb = TRUE;
	tc = &thread->context;

	if (update_fb)
	{
		s16 cause = (tc->cause >> 2) & 0x1f;
		if (cause == 23) // EXC_WATCH
			cause = 16;
		if (cause == 31) // EXC_VCED
			cause = 17;

		my_osViBlack(FALSE);

		console_clear();
		console_puts
		(
			str_error,
			(int)thread->id,
			(int)thread->priority,
			gCauseDesc[cause]
		);
		console_puts("a0       0x%x", (int)tc->a0);
		console_puts("a1       0x%x", (int)tc->a1);
		console_puts("a2       0x%x", (int)tc->a2);
		console_puts("a3       0x%x", (int)tc->a3);
		console_puts("pc       0x%x", (int)tc->pc);
		console_puts("badvaddr 0x%x", (int)tc->badvaddr);
		console_draw_raw();

		update_fb = FALSE;
	}

	// Halt everything
	// for (;;) { ; }
}

/* ========== GLOBAL FUNCTIONS ========== */

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