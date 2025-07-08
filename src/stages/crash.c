#include <ultra64.h>
#include <PR/os_internal_error.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

/* === Configuration === */
#include "config/global.h"

/* === Default libraries === */
#include "lib/types.h"
#include "lib/controller.h"
#include "lib/gfx.h"

/* === Custom libraries === */
#include "console.h"
#include "strings.h"

/* ============= PROTOTYPES ============= */

extern OSMesgQueue msgQ_crash;

static bool update_fb = FALSE;
OSThread *thread;

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

static int page;

/* ========== STATIC FUNCTIONS ========== */

static void init_crash()
{
	__OSThreadContext *tc = &thread->context;

	s16 cause = (tc->cause >> 2) & 0x1f;
	if (cause == 23) // EXC_WATCH
		cause = 16;
	if (cause == 31) // EXC_VCED
		cause = 17;

	my_osViBlack(TRUE);
	// osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	reset_controller();

	update_fb = TRUE;

	// Halt everything
	for (;;)
	{
		if (update_fb)
		{
			my_osViBlack(FALSE);

			console_clear();
			console_puts
			(
				str_error,
				(int)thread->id,
				(int)thread->priority,
				gCauseDesc[cause],
				(page + 1)
			);

			switch (page)
			{
				case 0:
					console_puts("at       0x%8x", (int)tc->at);
					console_puts("v0       0x%8x", (int)tc->v0);
					console_puts("v1       0x%8x", (int)tc->v1);
					console_puts("a0       0x%8x", (int)tc->a0);
					console_puts("a1       0x%8x", (int)tc->a1);
					console_puts("a2       0x%8x", (int)tc->a2);
					console_puts("a3       0x%8x", (int)tc->a3);
					console_puts("..       ..........\n..       ..........");
					break;

				case 1:
					console_puts("t0       0x%8x", (int)tc->t0);
					console_puts("t1       0x%8x", (int)tc->t1);
					console_puts("t2       0x%8x", (int)tc->t2);
					console_puts("t3       0x%8x", (int)tc->t3);
					console_puts("t4       0x%8x", (int)tc->t4);
					console_puts("t5       0x%8x", (int)tc->t5);
					console_puts("t6       0x%8x", (int)tc->t6);
					console_puts("t7       0x%8x", (int)tc->t7);
					console_puts("..       ..........");
					break;

				case 2:
					console_puts("s0       0x%8x", (int)tc->s0);
					console_puts("s1       0x%8x", (int)tc->s1);
					console_puts("s2       0x%8x", (int)tc->s2);
					console_puts("s3       0x%8x", (int)tc->s3);
					console_puts("s4       0x%8x", (int)tc->s4);
					console_puts("s5       0x%8x", (int)tc->s5);
					console_puts("s6       0x%8x", (int)tc->s6);
					console_puts("s7       0x%8x", (int)tc->s7);
					console_puts("..       ..........");
					break;

				case 3:
					console_puts("t8       0x%8x", (int)tc->t8);
					console_puts("t9       0x%8x", (int)tc->t9);
					console_puts("gp       0x%8x", (int)tc->gp);
					console_puts("sp       0x%8x", (int)tc->sp);
					console_puts("s8       0x%8x", (int)tc->s8);
					console_puts("ra       0x%8x", (int)tc->ra);
					console_puts("..       ..........");
					console_puts("pc       0x%8x", (int)tc->pc);
					console_puts("badvaddr 0x%8x", (int)tc->badvaddr);
					break;
			}
			console_draw_raw();

			update_fb = FALSE;
		}
		
		read_controller();

		if (controller[0].button == R_TRIG)
		{
			page += 1;
			update_fb = TRUE;
		}

		if (controller[0].button == L_TRIG)
		{
			page -= 1;
			update_fb = TRUE;
		}

		if (page < 0) { page = 3; }
		if (page > 3) { page = 0; }
	}
}

/* ========== GLOBAL FUNCTIONS ========== */

void crash(void *arg)
{
	OSMesg msg;
	s32 eventFlags;

    osSetEventMesg(OS_EVENT_CPU_BREAK, &msgQ_crash, (OSMesg)0x0A);
    osSetEventMesg(OS_EVENT_SP_BREAK, &msgQ_crash, (OSMesg)0x0B);
    osSetEventMesg(OS_EVENT_FAULT, &msgQ_crash, (OSMesg)0x0C);

    thread = (OSThread *)NULL;
	while (1)
	{
		if (thread == NULL)
		{
			// Wait until fault message is received
			(void) osRecvMesg(&msgQ_crash, (OSMesg *)&msg, OS_MESG_BLOCK);
			eventFlags |= (s32)msg;

			// Identify faulted thread
			thread = (OSThread *)__osGetCurrFaultedThread();

			// Initiate crash screen process if fault is detected
			if (thread && ((eventFlags & 0x0A) || (eventFlags & 0x0B) || (eventFlags & 0x0C)))
			{
				init_crash();
				return;
			}
		}
	}
}