#include <ultra64.h>
#include <PR/sched.h>

#include "debug.h"
#include "config.h"
#include "helpers/gfx.h"
#include "helpers/reader.h"
#include "helpers/text.h"
#include "stages/stages.h"

/* ============= PROTOTYPES ============= */

static void idle(void *);
static void main(void *);
extern void crash(void *);

// Stacks
u64 boot_stack[STACK_SIZE_BOOT / sizeof(u64)];
static u64 idle_stack[STACK_SIZE_IDLE / sizeof(u64)];
static u64 main_stack[STACK_SIZE_MAIN / sizeof(u64)];
static u64 crash_stack[STACK_SIZE_CRASH / sizeof(u64)];
static u64 scheduler_stack[OS_SC_STACKSIZE / sizeof(u64)];

// F3DEX2 matrix stack
static u64 dram_stack[SP_DRAM_STACK_SIZE8] __attribute__((aligned(16)));
// Display list processing results buffer
static u64 fifo_buffer[FIFO_SIZE] __attribute__((aligned(16)));
// Audio buffer
static u64 yield_buffer[OS_YIELD_DATA_SIZE] __attribute__((aligned(16)));

OSThread idle_thread;
OSThread main_thread;
OSThread crash_thread;

/* =============== DEBUG ================ */

#if DEBUG_MODE
// Debug buffer

#define RDB_SEND_BUF_SIZE   2000
u8  rdbSendBuf[RDB_SEND_BUF_SIZE];
#endif

/* ============== MESSAGES ============== */

// Required for hardware
OSMesg msg_gfx[NUM_GFX_MSGS];
OSMesgQueue msgQ_gfx;

// Crash screen
OSMesg msg_crash;
OSMesgQueue msgQ_crash;

/* ============= SCHEDULER ============== */

OSSched scheduler;
OSScClient sched_client; // Used for frame synchronisation
OSScTask sched_task;     // Used for scheduler graphics tasks
OSScMsg sched_send_msg;  // Hold message for rendering being done
OSScMsg *sched_msg; 	 // Received message

void init_scheduler()
{
	// Initialize video mode
	u8 video;
	u8 highres;

	#if (SCREEN_W == 640 && SCREEN_H == 480)
		highres = 1;
	#elif (SCREEN_W == 320 && SCREEN_H == 240)
		highres = 0;
	#else
		#error "Invalid screen resolution"
	#endif

	switch (osTvType) {
		case 0:
			video = highres == 1 ? OS_VI_PAL_HAN1 : OS_VI_PAL_LAN1;
			break;
		default:
		case 1:
			video = highres == 1 ? OS_VI_NTSC_HAN1 : OS_VI_NTSC_LAN1;
			break;
		case 2:
			video = highres == 1 ? OS_VI_MPAL_HAN1 : OS_VI_MPAL_LAN1;
			break;
	}

	// Create scheduler values using pointer to variable and stack
    osCreateScheduler(&scheduler, &scheduler_stack[OS_SC_STACKSIZE / sizeof(u64)], PR_SCHEDULER, video, 1);

	// Init messages
	osCreateMesgQueue(&msgQ_gfx, msg_gfx, NUM_GFX_MSGS);
	osScAddClient(&scheduler, &sched_client, &msgQ_gfx);

	// Specify register done message to OSScMsg
	sched_send_msg.type = OS_SC_DONE_MSG;

	// Set up graphics task for scheduler
	sched_task.list.t.type             = M_GFXTASK;
    sched_task.list.t.flags            = 0;
	// Set boot microcode to rspboot
	sched_task.list.t.ucode_boot       = (u64*)rspbootTextStart;
	sched_task.list.t.ucode_boot_size  = (u32)rspbootTextEnd - (u32)rspbootTextStart;
    sched_task.list.t.ucode            = (u64 *)gspF3DEX2_fifoTextStart;
	// Set microcode to FIFO F3DEX2
    sched_task.list.t.ucode_data       = (u64 *)gspF3DEX2_fifoDataStart;
    sched_task.list.t.ucode_size       = SP_UCODE_SIZE;
    sched_task.list.t.ucode_data_size  = SP_UCODE_DATA_SIZE;
	// DRAM stack
    sched_task.list.t.dram_stack       = (u64*)dram_stack;
    sched_task.list.t.dram_stack_size  = SP_DRAM_STACK_SIZE8;
	// Output buffer
	sched_task.list.t.output_buff      = fifo_buffer;
	sched_task.list.t.output_buff_size = &fifo_buffer[FIFO_SIZE / sizeof(u64)];
	// Yield buffer
	sched_task.list.t.yield_data_ptr   = yield_buffer;
	sched_task.list.t.yield_data_size  = OS_YIELD_DATA_SIZE;
	// Flags
	sched_task.flags = OS_SC_NEEDS_RSP | OS_SC_NEEDS_RDP | OS_SC_LAST_TASK | OS_SC_SWAPBUFFER;
	// Message queue for when task is finished rendering
	sched_task.msg  = (OSMesg)&sched_send_msg;
	sched_task.msgQ = &msgQ_gfx;
}

/* =========== MAIN FUNCTIONS =========== */

// Main N64 entry point.
void boot(void* arg)
{
	osInitialize();
	osAiSetFrequency(AUDIO_BITRATE);
	osCreateThread(&idle_thread, ID_IDLE, idle, arg, &idle_stack[STACK_SIZE_IDLE / sizeof(u64)], PR_IDLE);
	osStartThread(&idle_thread);
}

static void idle(void *arg)
{
	#if DEBUG_MODE
    osInitRdb(rdbSendBuf, RDB_SEND_BUF_SIZE);
	#endif

	// Initialize Pi Manager/DMA
	init_reader();

	// Initialize main thread
	osCreateThread(&main_thread, ID_MAIN, main, arg, &main_stack[STACK_SIZE_MAIN / sizeof(u64)], PR_MAIN);
	osStartThread(&main_thread);

	// Initialize crash screen queue and thread
    osCreateMesgQueue(&msgQ_crash, &msg_crash, 1);
	osCreateThread(&crash_thread, ID_CRASH, crash, NULL, &crash_stack[STACK_SIZE_CRASH / sizeof(u64)], PR_CRASH);
	osStartThread(&crash_thread);

	// Relinquish CPU
	osSetThreadPri(NULL, 0);

	// Enter a permanent loop, so as to remain the only thread running if all others are absent.
	while (1) ;
}

static void main(void *arg)
{
	// Initialize scheduler
	init_scheduler();
	
	read_all_segments();
	
	// Initialize current stage
	target_stage = 0;
	
	// Go into permanent loop, initialize target stage if it is loaded

	while (1)
	{
		switch (target_stage)
		{
			case 0:
				stage00_init();
				break;

			case 1:
				stage01_init();
				break;
		}

		current_stage = target_stage;
		
		loop:
		osRecvMesg(&msgQ_gfx, (OSMesg *)&sched_msg, OS_MESG_BLOCK);

		switch (sched_msg->type)
		{
			case OS_SC_PRE_NMI_MSG:
				target_stage = -1;
				osViSetYScale(1.0);
				osSpTaskYield();
				osViBlack(1);
				osAfterPreNMI();
				break;
				
			case OS_SC_DONE_MSG:
				break;

			case OS_SC_RETRACE_MSG:
				switch (current_stage)
				{
					case -1:
						osViBlack(0);
						init_gfx();
						clear_zfb();
						clear_cfb(0, 0, 0);
						print("Rebooting...");
						finish_gfx();
						break;

					case 0:
						stage00_update();
						stage00_render();
						break;

					case 1:
						stage01_update();
						stage01_render();
						break;
				}
				break;
		}
		
		if (target_stage == current_stage && target_stage >= 0)
			goto loop;
	}
}