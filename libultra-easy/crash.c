#include <ultra64.h>
#include <PR/os_internal_error.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

/* === Configuration === */
#include "config/global.h"
#include "config/usb.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/display.h"
#include "libultra-easy/gfx.h"
#include "libultra-easy/scheduler.h"

/* === Custom libraries === */
#include "strings.h"

/* ============= PROTOTYPES ============= */

extern Scheduler scheduler;
extern OSMesgQueue msgQ_crash;

/* ========== STATIC VARIABLES ========== */

static OSThread *thread;
static __OSThreadContext *tc;
static s16 cause;

static char * custom_desc = "";
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
static bool tinted_screen = FALSE;
static bool update_fb = FALSE;

/* ========== STATIC FUNCTIONS ========== */

void render_crash_screen()
{
	extern void tint_screen_raw();

	if (!tinted_screen)
	{
		debug_printf("[Fatal] Rendering crash screen to CPU\n");
		tint_screen_raw();
		tinted_screen = TRUE;
	}

	console_clear();
	console_puts
	(
		str_error,
		(int)thread->id,
		(int)thread->priority
	);
	console_puts("%s", strlen(custom_desc) > 0 ? custom_desc : gCauseDesc[cause]);
	console_puts("\n(L/R) P%d", (page + 1));

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
}

static void init_crash()
{
	debug_printf("[Fatal] Exception occurred at thread %d: %s\n", (int)thread->id, strlen(custom_desc) > 0 ? custom_desc : gCauseDesc[cause]);
	debug_printf("[GP Registers]\n");
	debug_printf("at: %x\n", (int)tc->at);
	debug_printf("v0: %x\n", (int)tc->v0);
	debug_printf("v1: %x\n", (int)tc->v1);
	debug_printf("a0: %x\n", (int)tc->a0);
	debug_printf("a1: %x\n", (int)tc->a1);
	debug_printf("a2: %x\n", (int)tc->a2);
	debug_printf("a3: %x\n", (int)tc->a3);
	debug_printf("t0: %x\n", (int)tc->t0);
	debug_printf("t1: %x\n", (int)tc->t1);
	debug_printf("t2: %x\n", (int)tc->t2);
	debug_printf("t3: %x\n", (int)tc->t3);
	debug_printf("t4: %x\n", (int)tc->t4);
	debug_printf("t5: %x\n", (int)tc->t5);
	debug_printf("t6: %x\n", (int)tc->t6);
	debug_printf("t7: %x\n", (int)tc->t7);
	debug_printf("s0: %x\n", (int)tc->s0);
	debug_printf("s1: %x\n", (int)tc->s1);
	debug_printf("s2: %x\n", (int)tc->s2);
	debug_printf("s3: %x\n", (int)tc->s3);
	debug_printf("s4: %x\n", (int)tc->s4);
	debug_printf("s5: %x\n", (int)tc->s5);
	debug_printf("s6: %x\n", (int)tc->s6);
	debug_printf("s7: %x\n", (int)tc->s7);
	debug_printf("t8: %x\n", (int)tc->t8);
	debug_printf("t9: %x\n", (int)tc->t9);
	debug_printf("gp: %x\n", (int)tc->gp);
	debug_printf("sp: %x\n", (int)tc->sp);
	debug_printf("s8: %x\n", (int)tc->s8);
	debug_printf("ra: %x\n", (int)tc->ra);
	debug_printf("pc: %x\n", (int)tc->pc);
	debug_printf("badvaddr: %x\n", (int)tc->badvaddr);

	reset_controller();

	scheduler.crash = TRUE;
	update_fb = TRUE;

	// Halt everything
	for (;;)
	{
		if (update_fb)
		{
			render_crash_screen();
			osWritebackDCacheAll();
			osViBlack(FALSE);
			osViSwapBuffer(osViGetCurrentFramebuffer());
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

void crash()
{
	// TLB exception on load/instruction fetch
	long e1;
	e1 = *(long *)1;
	
	// TLB exception on store
	*(long *)2 = 2;
}

void crash_msg(char *msg)
{
	if (msg != NULL || strlen(msg) > 0)
		custom_desc = msg;

	crash();
}

void crash_loop(void *arg)
{
	OSMesg msg;
	s32 eventFlags;

    osSetEventMesg(OS_EVENT_CPU_BREAK, &msgQ_crash, (OSMesg)0x0A);
    // osSetEventMesg(OS_EVENT_SP_BREAK, &msgQ_crash, (OSMesg)0x0B);
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
				tc = &thread->context;
				cause = (tc->cause >> 2) & 0x1f;
				if (cause == 23) // EXC_WATCH
					cause = 16;
				if (cause == 31) // EXC_VCED
					cause = 17;

				init_crash();
				return;
			}
		}
	}
}