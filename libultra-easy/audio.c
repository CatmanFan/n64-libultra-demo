#include <ultra64.h>
#include <PR/libaudio.h>
#include "libultra-easy.h"

#ifdef ENABLE_AUDIO

/* =================================================== *
 *                        MACROS                       *
 * =================================================== */

#define ENABLE_DMA

#define AUDIO_BUFF_COUNT	3
#define AUDIO_BUFF_SIZE		(2 * 1024) // Number of frames in an audio buffer.
#define AUDIO_MSG_COUNT		8

#define AUDIO_CL_COUNT		2
#define AUDIO_CL_MAX		4096

#define DMA_BUFF_COUNT		16
#define DMA_BUFF_SIZE		2048

#define MAX_VOICES			32
#define MAX_UPDATES			128
#define MAX_EVENTS			128

#define EXTRA_SAMPLES		96

/* =================================================== *
 *                       STRUCTS                       *
 * =================================================== */

/* =================================================== *
 *                 FUNCTION PROTOTYPES                 *
 * =================================================== */

static void audio_threadfunc(void *arg) __attribute__ ((noreturn));
static void music_init();
static void sound_init();
static void _bnkfPatchSound(ALSound* s, s32 offset, s32 table);
static void _bnkfPatchWaveTable(ALWaveTable* w, s32 offset, s32 table);

/* =================================================== *
 *                     PROTOTYPES                      *
 * =================================================== */

// Global parameters
static int				audio_heap_free;
static bool				audio_initialized = FALSE;
static Scheduler*		scheduler;
static ALGlobals		globals;
static ALHeap			heap;

// Global buffers
static Acmd 			audio_cl[AUDIO_CL_COUNT][AUDIO_CL_MAX];
static s32				audio_buffers[AUDIO_BUFF_COUNT][AUDIO_BUFF_SIZE] __attribute__((aligned(64)));

// Used to calculate samples per frame
static int				frame_size;
static int				frame_min;
static int				frame_max;
static int				samples_size;
static int				samples_remaining;

// Players
static ALSeqPlayer		music_player;
static ALSndPlayer		sound_player;

// Pointers to current address
static char*			music_bank_current;
static char*			midi_current;

// Dynamically-allocated buffers
static Sound			active_sounds[MAX_SOUNDS];

static OSThread			audio_thread;
static OSMesg			msg_ai[AUDIO_MSG_COUNT];
static OSMesgQueue		msg_queue_ai;
static OSMesg			msg_trigger[AUDIO_MSG_COUNT];
static OSMesgQueue		msg_queue_trigger;

/* =================================================== *
 *                         DMA                         *
 * =================================================== */

#ifdef ENABLE_DMA

	/* Structs
	   --------------------------------------------------- */

	typedef struct dma_metadata
	{
		u32 age;
		u32 offset;
	} DMAMetadata;

	/* Prototypes
	   --------------------------------------------------- */

	// DMA buffer metadata.
	static DMAMetadata dma_table[DMA_BUFF_COUNT];

	// DMA buffer contents.
	static u8 dma_buffer[DMA_BUFF_COUNT][DMA_BUFF_SIZE] __attribute__((aligned(16)));

	/* Callbacks and other functions
	   --------------------------------------------------- */

	static OSIoMesg msg_io_dma_audio[DMA_BUFF_COUNT];
	static OSMesg msg_dma_audio[DMA_BUFF_COUNT];
	static OSMesgQueue msg_queue_dma_audio;

	static u16 dmaNext;
	static u16 dmaCurrent;

	// Taken from fs.c
	extern OSPiHandle* rom_handle;

	static s32 audio_dma_callback(s32 addr, s32 len, void *state)
	{
		int i;
		int dma_oldest = 0;
		u32 dmaAge_oldest = 0;

		// Start and end address of requested data (cartridge address).
		u32 data_start = addr, data_end = addr + len;

		(void)state;

		// If these samples are already buffered, return the buffer.
		for (i = 0; i < DMA_BUFF_COUNT; i++)
		{
			u32 dma_start = dma_table[i].offset;
			u32 dma_end = dma_table[i].offset + DMA_BUFF_SIZE;

			if (dma_table[i].age > dmaAge_oldest)
			{
				dma_oldest = i;
				dmaAge_oldest = dma_table[i].age;
			}

			if (dma_start <= data_start && data_end <= dma_end)
			{
				u32 offset = data_start - dma_start;
				dma_table[i].age = 0;
				return K0_TO_PHYS(dma_buffer[i] + offset);
			}
		}

		// Otherwise, use the oldest buffer to start a new DMA.
		if (dmaAge_oldest == 0 || dmaCurrent >= DMA_BUFF_COUNT)
		{
			// If the buffer is in use, don't bother.
			return K0_TO_PHYS(dma_buffer[dma_oldest]);
		}
		else
		{
			// Use these variables only if the oldest buffer is to be found.
			u32 dma_src = data_start & ~1u;
			u8* dma_dest = (u8 *)dma_buffer[dma_oldest];
			u32 dma_size = (u32)DMA_BUFF_SIZE;

			// osWritebackDCache(dma_dest, dma_size);
			// osInvalDCache(dma_dest, dma_size);
			// osInvalICache(dma_dest, dma_size);

			msg_io_dma_audio[dmaNext] = (OSIoMesg)
			{
				.hdr = { .pri = OS_MESG_PRI_NORMAL, .retQueue = &msg_queue_dma_audio },
				.dramAddr = dma_dest,
				.devAddr  = dma_src,
				.size     = dma_size,
			};

			osEPiStartDma(rom_handle, &msg_io_dma_audio[dmaNext], OS_READ);

			// osWritebackDCache(dma_dest, dma_size);
			// osInvalDCache(dma_dest, dma_size);
			// osInvalICache(dma_dest, dma_size);

			dma_table[dma_oldest] = (DMAMetadata)
			{
				.age = 0,
				.offset = dma_src,
			};

			dmaNext = (dmaNext + 1) % DMA_BUFF_COUNT;
			dmaCurrent++;
			return K0_TO_PHYS(dma_buffer[dma_oldest] + (data_start & 1u));
		}
	}

	static ALDMAproc audio_dma_new(void *arg)
	{
		(void)arg;
		return audio_dma_callback;
	}

	static void audio_init_dma()
	{
		// Mark all DMA buffers as "old" so they get used.
		int i;
		for (i = 0; i < DMA_BUFF_COUNT; i++)
			dma_table[i].age = 1;

		osCreateMesgQueue(&msg_queue_dma_audio, msg_dma_audio, DMA_BUFF_COUNT);
	}

	static void audio_clear_dma()
	{
		int i;
		int current = dmaCurrent;

		while (1)
		{
			OSMesg dummy;
			int r = osRecvMesg(&msg_queue_dma_audio, &dummy, OS_MESG_NOBLOCK);
			if (r == -1)
				break;
			current--;
		}

		dmaCurrent = current;

		/*for (i = 0; i < dmaNext; i++)
		{
			OSIoMesg dummy;
			if (osRecvMesg(&dma_msg_queue, (OSMesg *)&dummy, OS_MESG_NOBLOCK) == -1)
				return;
		}*/

		for (i = 0; i < DMA_BUFF_COUNT; i++)
			dma_table[i].age++;
	}

#endif

/* =================================================== *
 *                       HELPERS                       *
 * =================================================== */

/*static s32 get_bank_index(const char *name)
{
	s32 index;
	for (index = 0; index < array_size(banks); index++)
		if (strcmp(banks[index].name, name) == 0)
			return index;

	return -1;
}*/

/* =================================================== *
 *                 AUDIO TASK HANDLERS                 *
 * =================================================== */

static OSTask audio_task = 
{{
	.type             = M_AUDTASK,
	.flags            = OS_TASK_DP_WAIT,
	.ucode            = (u64*)aspMainTextStart,
	.ucode_data       = (u64*)aspMainDataStart,
	.ucode_size       = SP_UCODE_SIZE,
	.ucode_data_size  = SP_UCODE_DATA_SIZE,
	.dram_stack       = NULL,
	.dram_stack_size  = 0,
}};

static Acmd *cl_start, *cl_end;
static int buffer_index, cl_index;
static s32 cl_length;

int audio_sample_xxx;

static void audio_push()
{
	// Set sample size
	samples_size = (16 + (frame_size - samples_remaining + EXTRA_SAMPLES)) & ~0xf;
	if (samples_size < frame_min) samples_size = frame_min;
	if (samples_size > frame_max) samples_size = frame_max;
	audio_sample_xxx = samples_size;

	// Create the command list.
	cl_length = 0;
	cl_index = (cl_index + 1) % AUDIO_CL_COUNT;
	cl_start = audio_cl[cl_index];
	cl_end = alAudioFrame
	(
		cl_start,
		&cl_length,
		(s16 *)osVirtualToPhysical(audio_buffers[buffer_index]),
		samples_size
	);

	// Check that the Acmd length is valid.
	my_assert(cl_length <= AUDIO_CL_MAX, "Audio assertion failed");
	if (cl_length == 0)
		return;

	// Create and submit the task.
	if (audio_task.t.ucode_boot_size == 0)
	{
		audio_task.t.ucode_boot       = (u64*)rspbootTextStart;
		audio_task.t.ucode_boot_size  = ((u32)rspbootTextEnd-(u32)rspbootTextStart);
	}
	audio_task.t.data_ptr         = (u64 *)cl_start;
	audio_task.t.data_size        = sizeof(Acmd) * (cl_end - cl_start);
	osWritebackDCache(cl_start, sizeof(Acmd) * (cl_end - cl_start));
	// osWritebackDCacheAll();

	// Do the thing
	debug_printf("[Audio] Beginning audio task.\n");
	scheduler->task_audio = &audio_task;
	osSpTaskStart(scheduler->task_audio);

	// Wait for task to finish
	debug_printf("[Audio] Waiting for AI message queue\n");
	scheduler->audio_notify = &msg_queue_ai;
	(void)osRecvMesg(&msg_queue_ai, NULL, OS_MESG_BLOCK);

	debug_printf("[Audio] Finished audio task.\n");
	scheduler->task_audio = NULL;
}

static void audio_pop()
{
	// On real hardware, it seems that there is some issue with the
	// ordering of the events. So we don't assume that the audio
	// device isn't busy just because this function was called.

	// Just try to push the next buffer, and fail otherwise.
	if (osAiSetNextBuffer(audio_buffers[buffer_index], samples_size << 2) != 0)
	{
		debug_printf("[Audio] Failed to push audio buffer at slot %d\n", buffer_index);
		return;
	}

	buffer_index = (buffer_index + 1) % AUDIO_BUFF_COUNT;
}

#endif

/* =================================================== *
 *               AUDIO TASK FUNCTIONING                *
 * =================================================== */

void audio_close()
{
#ifdef ENABLE_AUDIO
	alClose(&globals);
	osStopThread(&audio_thread);
	audio_initialized = FALSE;
#endif
}

void init_audio(Scheduler *sc)
{
#ifdef ENABLE_AUDIO
	if (!audio_initialized)
	{
		if (scheduler == NULL)
		{
			// Set scheduler
			scheduler = sc;

			// Initialize message queues
			osCreateMesgQueue(&msg_queue_ai, msg_ai, AUDIO_MSG_COUNT);
			osCreateMesgQueue(&msg_queue_trigger, msg_trigger, AUDIO_MSG_COUNT);

			// Create the audio thread
			osCreateThread(&audio_thread, ID_AUDIO, audio_threadfunc, NULL, REAL_STACK(AUDIO), PR_AUDIO);
		}

		// Start the audio thread
		osStartThread(&audio_thread);
		audio_initialized = TRUE;
	}
#endif
}

#ifdef ENABLE_AUDIO
static void *audio_heap_alloc(int size)
{
	if (size > audio_heap_free)
	{
		crash_msg("Audio heap is full");
		for (;;) { ; }
	}

	audio_heap_free -= size;
	return alHeapAlloc(&heap, 1, size);
}

static void audio_heap_dealloc(void *address, int size)
{
	bzero(address, size);
	audio_heap_free += size;
}

/**
 * @brief Blanks the audio heap and initializes it, alongside any other
 * buffers dependent on the audio heap.
 */
static void audio_heap_reset()
{
	if (audio_initialized) bzero((u8 *)AUDIO_HEAP_ADDR, AUDIO_HEAP_SIZE);
	alHeapInit(&heap, (u8 *)AUDIO_HEAP_ADDR, AUDIO_HEAP_SIZE);
	audio_heap_free = AUDIO_HEAP_SIZE;

	{
		// int i;

		// Audio buffers
		// for (i = 0; i < AUDIO_BUFF_COUNT; i++)
		// {
			// audio_buffers[i].data = audio_heap_alloc(sizeof(s32) * AUDIO_BUFF_SIZE);
			// audio_buffers[i].busy = FALSE;
		// }

		// Audio command lists
		// for (i = 0; i < AUDIO_CL_COUNT; i++)
			// audio_cl[i].data = (Acmd*)audio_heap_alloc(sizeof(Acmd) * AUDIO_CL_MAX);
	}
}

static void audio_threadfunc(void *arg)
{
	bool audio_heap_corrupted = FALSE;
	ALSynConfig audio_cfg;

#ifdef ENABLE_DMA
	// Initialize DMA
	audio_init_dma();
#endif

	// Initialize audio heap
	audio_heap_reset();

	// Calculate audio frame size
	audio_cfg.outputRate = osAiSetFrequency(AUDIO_BITRATE);
	{
		// Formula used by audiomgr.c
		f32 factor = (f32)audio_cfg.outputRate * 2 / 60.0F;
		frame_size = (s32)factor;
		if (frame_size < factor) frame_size++;
		if (frame_size & 0xf) frame_size = (frame_size & ~0xf) + 0x10;

		// Formula used by Thornmarked
		// if (display_tvtype() == OS_TV_PAL) frame_size = (audio_cfg.outputRate + 25) / 50;
		// else frame_size = (audio_cfg.outputRate * 1001 + 30000) / 60000;

		// frame_size = (((audio_cfg.outputRate / 60) + 0xf) & ~0xf);

		// Get min and max sizes
		frame_min = frame_size - 16;
		frame_max = frame_size + EXTRA_SAMPLES + 16;
	}

	// Set audio configuration
	audio_cfg.maxVVoices = MAX_VOICES;
	audio_cfg.maxPVoices = MAX_VOICES;
	audio_cfg.maxUpdates = MAX_UPDATES;
	audio_cfg.heap = &heap;
#ifdef ENABLE_DMA
	audio_cfg.dmaproc = audio_dma_new;
#else
	audio_cfg.dmaproc = NULL;
#endif
	audio_cfg.fxType = AL_FX_CHORUS;	// AL_FX_NONE
										// AL_FX_SMALLROOM
										// AL_FX_BIGROOM
										// AL_FX_ECHO
										// AL_FX_CHORUS
										// AL_FX_FLANGE
										// AL_FX_CUSTOM

	alInit(&globals, &audio_cfg);
	debug_printf("[Audio] Initialized audio driver\n");
	debug_printf("[Audio] Bitrate: %d\n", AUDIO_BITRATE);

	// Set up music and sound players
	music_init();
	sound_init();

	while (1)
	{

		scheduler->audio_notify = &msg_queue_trigger;
		osRecvMesg(&msg_queue_trigger, NULL, OS_MESG_BLOCK);

		samples_remaining = osAiGetLength() >> 2;

		#ifdef ENABLE_DMA
		// Clear and update DMA
		audio_clear_dma();
		#endif

		if (audio_heap_corrupted)
		{
			while (osViGetCurrentFramebuffer() == NULL) { ; }
			crash_msg("Audio heap corrupted, cannot continue");

			continue;
		}

		if (alHeapCheck(&heap) == 1 && !audio_heap_corrupted)
			crash_msg("Audio heap corrupted, cannot continue");
			// audio_heap_corrupted = TRUE;

		audio_push();
		audio_pop();
	}
}
#endif

/* =================================================== *
 *                     SOUND PLAYER                    *
 * =================================================== */

#ifdef ENABLE_AUDIO

static void _bnkfPatchSound(ALSound* s, s32 offset, s32 table)
{
	if (s->flags)
		return;

	s->flags = 1;

	s->envelope = (ALEnvelope*)((u8*)s->envelope + offset);
	s->keyMap = (ALKeyMap*)((u8*)s->keyMap + offset);

	s->wavetable = (ALWaveTable*)((u8*)s->wavetable + offset);

	_bnkfPatchWaveTable(s->wavetable, offset, table);
}

static void _bnkfPatchWaveTable(ALWaveTable* w, s32 offset, s32 table)
{
	if (w->flags)
		return;

	w->flags = 1;

	w->base += table;

	/* sct 2/14/96 - patch wavetable loop info based on type. */
	if (w->type == AL_ADPCM_WAVE)
	{
		w->waveInfo.adpcmWave.book = (ALADPCMBook*)((u8*)w->waveInfo.adpcmWave.book + offset);
		if (w->waveInfo.adpcmWave.loop)
			w->waveInfo.adpcmWave.loop = (ALADPCMloop*)((u8*)w->waveInfo.adpcmWave.loop + offset);
	}

	else if (w->type == AL_RAW16_WAVE)
	{
		if (w->waveInfo.rawWave.loop)
			w->waveInfo.rawWave.loop = (ALRawLoop*)((u8*)w->waveInfo.rawWave.loop + offset);
	}
}

static void sound_init()
{
	ALSndpConfig sound_cfg =
	{
		.maxSounds = MAX_SOUNDS,
		.maxEvents = MAX_EVENTS,
		.heap = &heap,
	};

	// sound_player = audio_heap_alloc(sizeof(ALSndPlayer));
	alSndpNew(&sound_player, &sound_cfg);

	debug_printf("[Audio] Initialized sound player\n");
}

static int sound_get_free_slot()
{
	int i;
	for (i = 0; i < MAX_SOUNDS; i++)
	{
		if (active_sounds[i].snd == NULL)
		{
			return i;
		}
	}

	return -1;
}


static Sound* sound_get(int index)
{
	int i;
	for (i = 0; i < MAX_SOUNDS; i++)
	{
		if (!active_sounds[i].snd == NULL && active_sounds[i].snd_index == index)
		{
			return &active_sounds[i];
		}
	}

	return NULL;
}

#endif

void sound_play(SoundBank *sfx, int snd_index, int snd_slot)
{
#ifdef ENABLE_AUDIO
	if (sfx != NULL)
	{
		// Get first empty sound slot
		if (active_sounds[snd_slot].snd != NULL)
		{
			debug_printf("[Audio] Sound at slot %d is occupied, cancelling...\n", snd_slot);
			return;
		}

		// Get first empty sound slot
		if (snd_slot < 0)
		{
			snd_slot = sound_get_free_slot();
			if (snd_slot < 0)
			{
				debug_printf("[Audio] All sound slots occupied, cancelling...\n");
				return;
			}
		}

		// Allocate ALSndId first, if failed then stop
		active_sounds[snd_slot].id = alSndpAllocate(&sound_player, sfx->file->sounds[snd_index]);
		if (active_sounds[snd_slot].id == -1)
		{
			debug_printf("[Audio] Invalid sound ID, cancelling...\n");
			return;
		}

		// Set slot pointer to sound
		active_sounds[snd_slot].snd = sfx->file->sounds[snd_index];
		active_sounds[snd_slot].snd_index = snd_index;
		active_sounds[snd_slot].playing = TRUE;

		alSndpSetSound(&sound_player, active_sounds[snd_slot].id);
		alSndpSetPitch(&sound_player, 1);
		alSndpSetPan(&sound_player, 64);
		alSndpSetVol(&sound_player, 30000);
		alSndpPlay(&sound_player);

		debug_printf("[Audio] Started sound %d belonging to %s in slot %d\n", snd_index, sfx->name, snd_slot);
	}
#endif
}

void sound_stop(int snd_slot)
{
#ifdef ENABLE_AUDIO
	if (active_sounds[snd_slot].playing)
	{
		active_sounds[snd_slot].playing = FALSE;

		alSndpSetSound(&sound_player, active_sounds[snd_slot].id);
		alSndpStop(&sound_player);

		debug_printf("[Audio] Stopped sound at slot %d\n", snd_slot);
	}

	active_sounds[snd_slot].snd = NULL;
	active_sounds[snd_slot].snd_index = 0;
#endif
}

// void sound_destroy(int index)
// {
// #ifdef ENABLE_AUDIO
	// Sound *target = sound_get(index);
	// if (target != NULL)
	// {
		// alSndpSetSound(&sound_player, target->id);
		// alSndpStop(&sound_player);
		// while (alSndpGetState(&sound_player) != AL_STOPPED) { ; }
		// alSndpDeallocate(&sound_player, target->id);
		// debug_printf("[Audio] Stopped sound %d\n", index);

		// *target = (Sound){0};
		// active_sounds--;
	// }
// #endif
// }

// void sound_stop_all()
// {
// #ifdef ENABLE_AUDIO
	// int i = active_sounds - 1;
	// while (1)
	// {
		// sound_destroy(i);
		// i--;
		// if (i < 0) break;
	// }
// #endif
// }

void sound_init_bank(SoundBank *sfx)
{
#ifdef ENABLE_AUDIO
	if (strlen(sfx->name) <= 0)
	{
		debug_printf("[Audio] Music bank does not appear to have a name attributed to it, cancelling\n");
		return;
	}

	if (!sfx->initialized)
	{
		int length = sfx->ctl_end - sfx->ctl_start;
		sfx->file = audio_heap_alloc(length);
		load_from_rom((char *) sfx->file, sfx->ctl_start, length);

		// Patch each sound in CTL
		{
			int i;
			for (i = 0; i < sfx->file->soundCount; i++)
			{
				sfx->file->sounds[i] = (ALSound*)((u8*)sfx->file->sounds[i] + (u32)sfx->file);
				_bnkfPatchSound(sfx->file->sounds[i], (u32)sfx->file, (u32)sfx->tbl_start);
			}
		}

		sfx->count = sfx->file->soundCount;
		sfx->initialized = TRUE;
		debug_printf("[Audio] Loaded sound bank %s from address %p of size %d bytes\n", sfx->name, sfx->ctl_start, length);
	}
#endif
}

void sound_set_bank(SoundBank *sfx)
{
#ifdef ENABLE_AUDIO
#endif
}

/* =================================================== *
 *                     MUSIC PLAYER                    *
 * =================================================== */

void sound_test()
{
	/*// Times measured in microseconds.
	ALEnvelope sndenv = {
		.attackTime = 0,
		.releaseTime = 13400000,
		.attackVolume = 67,
		.decayVolume = 67,
	};

	// Not sure this does anything.
	ALKeyMap keymap = {
		.velocityMin = 66,
		.velocityMax = 49,
		.keyMin = 0,
		.keyMax = 1,
		.keyBase = 0,
		.detune = 0,
	};

	// Poitner to the PCM data.
	ALWaveTable wtable = {
		.base = (u8 *)138176,
		.len = 71478,
		.type = AL_ADPCM_WAVE,
		.flags = 1,
	};

	// Pointer to the sound itself.
	ALSound snd = {
		.envelope = &sndenv,
		.keyMap = &keymap,
		.wavetable = &wtable,
		.samplePan = 64,
		// .sampleVolume = 127,
		.flags = 1,
	};

	// Allocate and play a sound.
	ALSndId sndid = alSndpAllocate(&sound_player, &snd);
	alSndpSetSound(&sound_player, sndid);
	alSndpSetPitch(&sound_player, 1.0F);
	alSndpSetPan(&sound_player, 64);
	alSndpSetVol(&sound_player, 30000);
	alSndpPlay(&sound_player);*/
}

#ifdef ENABLE_AUDIO
static void music_init()
{
	ALSeqpConfig music_cfg =
	{
		.maxVoices = MAX_VOICES,
		.maxEvents = MAX_EVENTS,
		.maxChannels = 16,
		.heap = &heap,
		#ifdef DEBUG_MODE
		// .debugFlags = NO_VOICE_ERR_MASK | NOTE_OFF_ERR_MASK | NO_SOUND_ERR_MASK,
		#endif
	};

	// music_player = audio_heap_alloc(sizeof(ALSeqPlayer));
	alSeqpNew(&music_player, &music_cfg);

	debug_printf("[Audio] Initialized music player\n");
}
#endif

void music_player_start()
{
#ifdef ENABLE_AUDIO
	if (alSeqpGetState(&music_player) != AL_PLAYING)
	{
		debug_printf("[Audio] Starting music player\n");
		alSeqpPlay(&music_player);
	}
#endif
}

void music_player_stop()
{
#ifdef ENABLE_AUDIO
	if (alSeqpGetState(&music_player) != AL_STOPPED)
	{
		debug_printf("[Audio] Stopping music player\n");
		alSeqpStop(&music_player);
	}
#endif
}

void music_init_bank(InstBank *bank)
{
#ifdef ENABLE_AUDIO
	if (strlen(bank->name) <= 0)
	{
		debug_printf("[Audio] Music bank does not appear to have a name attributed to it, cancelling\n");
		return;
	}

	if (!bank->initialized)
	{
		int length = bank->ctl_end - bank->ctl_start;
		bank->ctl_file = audio_heap_alloc(length);
		load_from_rom((char *) bank->ctl_file, bank->ctl_start, length);

		alBnkfNew(bank->ctl_file, (u8 *) bank->tbl_start);
		bank->bank = bank->ctl_file->bankArray[0];

		bank->initialized = TRUE;
		debug_printf("[Audio] Loaded music bank %s from address %p of size %d bytes\n", bank->name, bank->ctl_start, length);
	}
#endif
}

void music_deinit_bank(InstBank *bank)
{
#ifdef ENABLE_AUDIO
	if (strlen(bank->name) <= 0)
	{
		debug_printf("[Audio] Music bank does not appear to have a name attributed to it, cancelling\n");
		return;
	}

	if (bank->initialized)
	{
		if (music_bank_current == bank->ctl_start)
		{
			alSeqpSetBank(&music_player, 0);
			music_bank_current = 0;
		}

		bank->bank = NULL;
		audio_heap_dealloc(bank->ctl_file, sizeof(bank->ctl_file));
		bank->ctl_file = NULL;

		bank->initialized = FALSE;
		debug_printf("[Audio] Unloaded music bank %s from RAM\n", bank->name);
	}
#endif
}

void music_set_bank(InstBank *bank)
{
#ifdef ENABLE_AUDIO
	if (!bank->initialized) return;

	// Point sequence player's bank to sequence_bank
    alSeqpSetBank(&music_player, bank->ctl_file->bankArray[0]);

	debug_printf("[Audio] Configured current music bank to %s at address %p\n", bank->name, bank->ctl_start);
	music_bank_current = bank->ctl_start;
#endif
}

void midi_set_tempo(MIDI *mid, int tempo)
{
	mid->tempo = tempo;
}

void midi_set_markers(MIDI *mid, int playback_start, int loop_start, int loop_end, int loop_count)
{
	mid->playback_start = playback_start;
	mid->loop_start = loop_start;
	mid->loop_end = loop_end;
	mid->loop_count = loop_count;
}

void midi_send_event(long ticks, u8 status, u8 byte1, u8 byte2)
{
#ifdef ENABLE_AUDIO
	alSeqpSendMidi(&music_player, ticks, status, byte1, byte2);
#endif
}

void midi_init(MIDI *mid)
{
#ifdef ENABLE_AUDIO
	if (strlen(mid->name) <= 0)
	{
		debug_printf("[Audio] MIDI does not appear to have a name attributed to it, cancelling\n");
		return;
	}

	if (!mid->initialized)
	{
		int length = mid->mid_end - mid->mid_start;
		mid->data = audio_heap_alloc(length);
		load_from_rom((char *) mid->data, mid->mid_start, length);

		mid->sequence = audio_heap_alloc(sizeof(ALSeq));
		alSeqNew(mid->sequence, mid->data, length);

		// Set loop markers
		alSeqNewMarker(mid->sequence, &mid->m_loop_start, mid->loop_start);
		alSeqNewMarker(mid->sequence, &mid->m_loop_end, mid->loop_end);

		// Set playback start marker
		alSeqNewMarker(mid->sequence, &mid->m_playback_start, mid->playback_start);
		if (mid->playback_start > 0)
			alSeqSetLoc(mid->sequence, &mid->m_playback_start);

		mid->initialized = TRUE;
		debug_printf("[Audio] Loaded MIDI track %s from address %p of size %d bytes\n", mid->name, mid->mid_start, length);
	}
#endif
}

void music_set_midi(MIDI *mid)
{
#ifdef ENABLE_AUDIO
	if (!mid->initialized) return;

	// Point music player to sequence
    alSeqpSetSeq(&music_player, mid->sequence);

	// Set tempo
	alSeqpSetTempo(&music_player, mid->tempo);

	// Set loop
	alSeqpLoop(&music_player, &mid->m_loop_start, &mid->m_loop_end, mid->loop_count);

	debug_printf("[Audio] Configured current MIDI track to %s (%p)\n", mid->name, mid->mid_start);
	midi_current = mid->mid_start;

	// Play the newly-registered sequence
	// music_player_start();
#endif
}

// #else

// void init_audio(Scheduler *sc)
// { ; }
// void audio_close()
// { ; }
// void audio_heap_reset()
// { ; }

// void music_init_bank(InstBank *bank)
// { ; }
// void music_set_bank(InstBank *bank)
// { ; }
// void midi_set_tempo(MIDI *mid, int tempo)
// { ; }
// void midi_set_markers(MIDI *mid, int playback_start, int loop_start, int loop_end, int loop_count)
// { ; }
// void midi_init(MIDI *mid)
// { ; }
// void music_set_midi(MIDI *mid)
// { ; }
// void music_player_start()
// { ; }
// void music_player_stop()
// { ; }

// void sound_init_bank(SoundBank *sfx)
// { ; }
// void sound_set_bank(SoundBank *sfx)
// { ; }
// void sound_play(SoundBank *sfx, int snd_index, int snd_slot)
// { ; }
// void sound_stop(int snd_slot)
// { ; }
// void sound_destroy(int index)
// { ; }
// void sound_stop_all()
// { ; }

// #endif