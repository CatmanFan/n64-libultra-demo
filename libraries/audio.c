#include <ultra64.h>
#include <PR/libmus.h>

#include "config/global.h"
#include "config/audio.h"
#include "libraries/reader.h"
#include "libraries/scheduler.h"

/* ============= PROTOTYPES ============= */

static u8 sfx_buf[SFX_BUF_SIZE] __attribute__((aligned(16)));
static u8 ptr_sfx_buf[PTR_BUF_SIZE] __attribute__((aligned(16)));
static u8 bgm_buf[BGM_BUF_SIZE] __attribute__((aligned(16)));
static u8 ptr_inst_buf[PTR_BUF_SIZE] __attribute__((aligned(16)));

/* ============= FUNCTIONS ============== */

void init_audio()
{
	musConfig config;

	config.control_flag = 0; // Use ROM wavetables
	config.channels = 24; // Decent default for channel count
	config.sched = &scheduler;
	config.thread_priority = PR_AUDIO;
	// Setup audio heap
	config.heap = (u8 *)AUDIO_ADDR;
	config.heap_length = STACK_SIZE_AUDIO;
	// Set FIFO length to minimum
	config.fifo_length = 64;
	// Assign no initial audio or FX bank
	config.ptr = NULL;
	config.wbk = NULL;
	config.default_fxbank = NULL;

	// Set audio default frequency
	config.syn_output_rate = AUDIO_BITRATE;

	// Set synthesizer parameters to sane defaults from nualstl3
	config.syn_updates = 256;
	config.syn_rsp_cmds = 2048;
	config.syn_num_dma_bufs = 64;
	config.syn_dma_buf_size = 1024;
	config.syn_retraceCount = 1; // Must be same as last parameter to osCreateScheduler

	// Initialize libmus
	MusInitialize(&config);
}

void load_inst(char *start, char *end, char *wbank)
{
	// Read and register the instrument bank and its pointers. (instruments ptr file)
	load_binary((void*)start, (void*)ptr_inst_buf, end-start);
	MusPtrBankInitialize(ptr_inst_buf, wbank);
}

void play_bgm(char *start, char *end)
{
	// Read and register the background music (song bin file)
	load_binary((void*)start,(void*)bgm_buf, end-start);

	// Set override for the instrument bank pointers
	MusPtrBankSetSingle(ptr_inst_buf);

	// Play the song utilizing the instruments bank
	MusStartSong(bgm_buf);
}

void load_sounds(char *ptr_start, char *ptr_end, char *wbank, char *start, char *end)
{
	// Read and register the instrument bank and its pointers. (instruments ptr file)
	load_binary((void*)ptr_start, (void*)ptr_sfx_buf, ptr_end-ptr_start);
	MusPtrBankInitialize(ptr_sfx_buf, wbank);

	// Read and register the sound effects. (sound effects bfx file)
	load_binary((void*)start, (void*)sfx_buf, end-start);
	MusFxBankInitialize(sfx_buf);
}

void play_sound(int index)
{
	MusStartEffect(index);
}