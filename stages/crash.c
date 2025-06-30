#include <ultra64.h>
#include <PR/os_internal_error.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "config.h"
#include "helpers/types.h"

#include "helpers/custom/console.h"
#include "helpers/custom/strings.h"

extern OSMesgQueue msgQ_crash;

static bool update_fb = FALSE;
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
			s16 cause = (tc->cause >> 2) & 0x1f;
			if (cause == 23) // EXC_WATCH
				cause = 16;
			if (cause == 31) // EXC_VCED
				cause = 17;

			osViBlack(0);

			console_clear();
			console_printf
			(
				STR_ERROR,
				(int)thread->id,
				(int)thread->priority,
				gCauseDesc[cause],
				(int)tc->pc,
				(int)tc->badvaddr
			);
			console_draw_raw();

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