#include <ultra64.h>
#include <PR/libaudio.h>
#include "libultra-easy.h"

#ifdef ENABLE_AUDIO
#define ENABLE_DMA

/* =================================================== *
 *                       MACROS                        *
 * =================================================== */

#define DMA_BUFF_COUNT		64
#define DMA_BUFF_SIZE		1024
#define AUDIO_BUFF_COUNT	3
#define AUDIO_BUFF_SIZE		1024 * 2 // Number of frames in an audio buffer.
#define AUDIO_CL_COUNT		3
#define AUDIO_CL_SIZE		4096
#define AUDIO_FRAME_COUNT	2
#define AUDIO_SEQ_COUNT		3
#define AUDIO_SEQ_SIZE		40 * 1024

#define MAX_VOICES			4
#define MAX_UPDATES			256
#define MAX_EVENTS			32
#define MAX_SOUNDS			16
#define MAX_CHANNELS		16

#define EXTRA_SAMPLES		80

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

// Main buffers
static Acmd audio_cl[AUDIO_CL_COUNT][AUDIO_CL_SIZE] __attribute__((aligned(16)));
static bool audio_cl_busy[AUDIO_CL_COUNT];

/*static*/ u8 audio_buffers[AUDIO_BUFF_COUNT][AUDIO_BUFF_SIZE];
/*static*/ bool audio_buffers_busy[AUDIO_CL_COUNT];

/*static*/ int audio_samples_per_frame;

// Players
static ALSndPlayer* sound_player;
static ALSeqPlayer* music_player;

// Bank request buffers
SEGMENT_DECLARE(bgmCtl_playstation)
SEGMENT_DECLARE(bgmTbl_playstation)
SEGMENT_DECLARE(sfx_sample)
SEGMENT_DECLARE(sfxTbl_sample)

AudioBank banks[] =
{
	{
		.name = "playstation",
		.ctl_start = SEGMENT_START(bgmCtl_playstation),
		.ctl_end = SEGMENT_END(bgmCtl_playstation),
		.tbl_start = SEGMENT_START(bgmTbl_playstation),
		.is_sound = FALSE
	},
};

// Sound table
static ALBank*			music_bank;
static SoundArray*		sound_bank;

// Music sequence table
static ALSeq			*sequences[AUDIO_SEQ_COUNT];
static s32				sequenceLen[AUDIO_SEQ_COUNT];
static u8				*sequenceData[AUDIO_SEQ_COUNT];
static ALSeqMarker		sequenceStart[AUDIO_SEQ_COUNT];
static ALSeqMarker		sequenceLoopStart[AUDIO_SEQ_COUNT];
static ALSeqMarker		sequenceEnd[AUDIO_SEQ_COUNT];
static SeqPlayEvent		pendingSeq;
static char*			currentSeq;
static int				targetSeq;

// Parameters
static ALGlobals		globals;
static ALHeap			heap;

static bool				audio_initialized = FALSE;

static OSThread			audio_thread;
static OSMesg			msg_ai;
static OSMesgQueue		msg_queue_ai;
static OSMesg			msg_frame[AUDIO_FRAME_COUNT];
static OSMesgQueue		msg_queue_frame;

static Scheduler *scheduler;

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

	static u16 dmaNext;
	static u16 dmaCurrent;

	static OSMesg dma_msg[DMA_BUFF_COUNT];
	static OSIoMesg dma_msg_io[DMA_BUFF_COUNT];
	static OSMesgQueue dma_msg_queue;

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
			extern OSPiHandle* rom_handle;
			u32 dma_src = data_start & ~1u;
			u8* dma_dest = (u8 *)dma_buffer[dma_oldest];
			u32 dma_size = (u32)DMA_BUFF_SIZE;
			OSMesg dummy;

			/*
			 * Always invalidate cache before dma'ing data into the buffer.
			 * This is to prevent a flush of the cache in the future from 
			 * potentially trashing some data that has just been dma'ed in.
			 * Since you don't care if old data makes it from cache out to 
			 * memory, you can use the cheaper osInvalDCache() instead of one
			 * of the writeback commands
			 */
			osWritebackDCache(dma_dest, dma_size);
			osInvalDCache(dma_dest, dma_size);
			osInvalICache(dma_dest, dma_size);

			dma_msg_io[dmaNext] = (OSIoMesg)
			{
				.hdr = { .pri = OS_MESG_PRI_NORMAL, .retQueue = &dma_msg_queue },
				.dramAddr = dma_dest,
				.devAddr  = dma_src,
				.size     = dma_size,
			};

			osEPiStartDma(rom_handle, &dma_msg_io[dmaNext], OS_READ);
			(void) osRecvMesg(&dma_msg_queue, &dummy, OS_MESG_BLOCK);

			osWritebackDCache(dma_dest, dma_size);
			osInvalDCache(dma_dest, dma_size);
			osInvalICache(dma_dest, dma_size);

			dma_table[dma_oldest].age = 0;
			dma_table[dma_oldest].offset = dma_src;

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

	static void audio_clear_dma()
	{
		int i;
		int current = dmaCurrent;

		while (1)
		{
			OSMesg dummy;
			int r = osRecvMesg(&dma_msg_queue, &dummy, OS_MESG_NOBLOCK);
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

static s32 get_bank_index(const char *name)
{
	s32 index;
	for (index = 0; index < array_size(banks); index++)
		if (strcmp(banks[index].name, name) == 0)
			return index;

	return -1;
}

static void set_frame_size(int output_rate)
{
	if (display_tvtype() == 0) // PAL
	{
		audio_samples_per_frame = (output_rate + 25) / 50;
	}
	else
	{
		audio_samples_per_frame = (output_rate * 1001 + 30000) / 60000;
	}
}

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

static int buffer_index, cl_index;

static void audio_handle_buffer()
{

	osRecvMesg(&msg_queue_frame, NULL, OS_MESG_BLOCK);

	/* =================================================== *
	 *                   BUFFER HANDLING                   *
	 * =================================================== */

	// for (i = 0; i < AUDIO_BUFF_COUNT; i++)
	// {
		// unsigned phase = 0;
		// unsigned rate = ((50 * (i + 4)) << 16) / AUDIO_BITRATE;

		// int j;
		// for (j = 0; j <= AUDIO_BUFF_SIZE - 1; j+=2)
		// {
			// audio_buffers[i][j] = (i * 4) * 0x8000 / 0x200;

			// audio_buffers[i][j] = phase;
			// audio_buffers[i][j + 1] = phase;
			// phase += rate;
		// }
	// }

	// osWritebackDCache(audio_buffers, sizeof(audio_buffers));

#ifdef ENABLE_DMA
	// Clear and update DMA
	audio_clear_dma();
#endif
}

Acmd *cl_start, *cl_end;
s16 *buffer_address;
s32 cl_length = 0;

static void audio_do_task()
{

	// Look for an audio buffer
	for (buffer_index = 0; buffer_index < AUDIO_BUFF_COUNT; buffer_index++)
	{
		if (!audio_buffers_busy[buffer_index])
		{
			debug_printf("[Audio] Found empty audio buffer at slot %d\n", buffer_index);

			// Look for an audio command list slot
			for (cl_index = 0; cl_index < AUDIO_CL_COUNT; cl_index++)
			{
				if (!audio_cl_busy[cl_index])
				{
					debug_printf("[Audio] Found free audio command list at slot %d\n", cl_index);
					goto do_task;
				}
			}

			// Terminate if no command list slot was found
			debug_printf("[Audio] No available audio command list found, skipping...\n");
			return;
		}
	}

	// Terminate if no buffer was found
	debug_printf("[Audio] No available audio buffer found, skipping...\n");
	return;

	do_task:
	audio_buffers_busy[buffer_index] = TRUE;
	buffer_address = (s16 *)osVirtualToPhysical(audio_buffers[buffer_index]);

	// Create the command list.
	audio_cl_busy[cl_index] = TRUE;
	cl_start = audio_cl[cl_index];
	cl_end = alAudioFrame(cl_start, &cl_length, buffer_address, audio_samples_per_frame);
	my_assert(cl_end - cl_start < AUDIO_CL_SIZE, "Audio assertion failed");

	// Create and submit the task.
	audio_task.t.ucode_boot       = (u64*)rspbootTextStart;
	audio_task.t.ucode_boot_size  = ((u32)rspbootTextEnd-(u32)rspbootTextStart);
	audio_task.t.data_ptr         = (u64 *)cl_start;
	audio_task.t.data_size        = sizeof(Acmd) * (cl_end - cl_start);
	// osWritebackDCache(cl_start, sizeof(Acmd) * (cl_end - cl_start));
	osWritebackDCacheAll();

	// Do the thing
	debug_printf("[Audio] Beginning audio task.\n");
	osSpTaskStart(&audio_task);

	// Wait for task to finish
	// (void)osRecvMesg(&msg_queue_ai, NULL, OS_MESG_BLOCK);
	debug_printf("[Audio] Finished audio task.\n");
}

static void audio_pop()
{
	// On real hardware, it seems that there is some issue with the
	// ordering of the events. So we don't assume that the audio
	// device isn't busy just because this function was called.
	
	// Just try to push the next buffer, and fail otherwise.
	if (osAiSetNextBuffer(audio_buffers[buffer_index], audio_samples_per_frame) != 0)
		return;

	audio_buffers_busy[buffer_index] = FALSE;
	audio_cl_busy[cl_index] = FALSE;

	// samples_remaining = /* IO_READ(AI_LEN_REG) */ osAiGetLength() >> 2;
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
		#ifdef ENABLE_DMA
			osCreateMesgQueue(&dma_msg_queue, dma_msg, DMA_BUFF_COUNT);
		#endif
			osCreateMesgQueue(&msg_queue_ai, &msg_ai, 1);
			osCreateMesgQueue(&msg_queue_frame, msg_frame, AUDIO_FRAME_COUNT);
			osSetEventMesg(OS_EVENT_AI, &msg_queue_ai, (OSMesg)SC_MSG_AUDIO);
			scheduler->audio_notify = &msg_queue_frame;

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
static void audio_threadfunc(void *arg)
{
	int i;
	ALSynConfig audio_cfg;

#ifdef ENABLE_DMA
	// Mark all DMA buffers as "old" so they get used.
	for (i = 0; i < DMA_BUFF_COUNT; i++)
		dma_table[i].age = 1;
#endif

	// Initialize audio heap
	alHeapInit(&heap, (u8 *)AUDIO_HEAP_ADDR, AUDIO_HEAP_SIZE);

	// Initialize buffers dependent on heap
	/*for (i = 0; i < AUDIO_BUFF_COUNT; i++)
		{ audio_buffers[i] = alHeapAlloc(&heap, 1, sizeof(s32) * AUDIO_BUFF_SIZE); }
	for (i = 0; i < AUDIO_CL_COUNT; i++)
		{ audio_cl[i] = (Acmd*)alHeapAlloc(&heap, 1, sizeof(Acmd) * AUDIO_CL_SIZE); }*/

	for (i = 0; i < AUDIO_SEQ_COUNT; i++)
	{
		sequenceData[i] = alHeapAlloc(&heap, 1, AUDIO_SEQ_SIZE); // 40 kB
		sequences[i] = alHeapAlloc(&heap, 1, sizeof(ALSeq));
	}

	// Load all sound banks
	/*for (i = 0; i < 1; i++)
	{
		s32 size = banks[i].ctl_end - banks[i].ctl_start;
		banks[i].sound_file = alHeapAlloc(&heap, 1, size);
		load_from_rom(banks[i].sound_file, banks[i].ctl_start, size);

		for (i = 0; i < 1; i++)
		{
			banks[i].sound_file->sounds[i] = (ALSound*)((u8*)banks[i].sound_file->sounds[i] + (u32)banks[i].sound_file);
			_bnkfPatchSound(banks[i].sound_file->sounds[i], (u32)banks[i].sound_file, (u32)banks[i].tbl_start);
			_bnkfPatchWaveTable(banks[i].sound_file->sounds[i]->wavetable, (u32)banks[i].sound_file, (u32)banks[i].tbl_start);
		}

		debug_printf("[Audio] Loaded sound bank to bank slot %d at address %p\n", i, banks[i].ctl_start);
		debug_printf("[Audio] Total number of sounds in sound bank: %d", banks[i].sound_file->soundCount);
	}*/

	audio_cfg.outputRate = osAiSetFrequency(AUDIO_BITRATE);
	audio_cfg.maxVVoices = MAX_VOICES;
	audio_cfg.maxPVoices = MAX_VOICES;
	audio_cfg.maxUpdates = MAX_UPDATES;
	audio_cfg.heap = &heap;
#ifdef ENABLE_DMA
	audio_cfg.dmaproc = audio_dma_new;
#else
	audio_cfg.dmaproc = NULL;
#endif
	audio_cfg.fxType = AL_FX_SMALLROOM;	// AL_FX_NONE
										// AL_FX_SMALLROOM
										// AL_FX_BIGROOM
										// AL_FX_ECHO
										// AL_FX_CHORUS
										// AL_FX_FLANGE
										// AL_FX_CUSTOM

	// Set audio framerate
	set_frame_size(audio_cfg.outputRate);

	alInit(&globals, &audio_cfg);
	debug_printf("[Audio] Initialized audio driver\n");
	debug_printf("[Audio] Bitrate: %d\n", AUDIO_BITRATE);

	// Set up music and sound players
	music_player = alHeapAlloc(&heap, 1, sizeof(ALSeqPlayer));
	sound_player = alHeapAlloc(&heap, 1, sizeof(ALSndPlayer));
	music_init();
	sound_init();

	while (1)
	{
		audio_handle_buffer();
		audio_do_task();
		audio_pop();

		if (pendingSeq.romStart && alSeqpGetState(music_player) != AL_PLAYING)
		{
			music_play_seq(&pendingSeq);
			pendingSeq.romStart = 0;
		}
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

	alSndpNew(sound_player, &sound_cfg);
	debug_printf("[Audio] Initialized sound player\n");
}
#endif

void sound_set_bank(const char *name)
{
#ifdef ENABLE_AUDIO
	int index = get_bank_index(name);
	if (index == -1)
	{
		if (!banks[index].is_sound)
		{
			if (sound_bank != banks[index].sound_file)
			{
				sound_bank = banks[index].sound_file;
				debug_printf("[Audio] Changed sound bank to %s (%d)\n", banks[index].name, index);
				return;
			}
			else
			{
				debug_printf("[Audio] Sound bank is already set to %s (%d), cancelling...\n", banks[index].name, index);
				return;
			}
		}
	}

	debug_printf("[Audio] Sound bank named %s not found, cancelling...\n", name);
#endif
}

void sound_play(int index)
{
#ifdef ENABLE_AUDIO
	if (sound_bank != NULL)
	{
		ALSound* snd = sound_bank->sounds[index];
		ALSndId sndId = alSndpAllocate(sound_player, snd);

		if (sndId == -1)
			crash_msg("Invalid sound ID");
		alSndpSetSound(sound_player, sndId);
		alSndpSetPitch(sound_player, 1);
		alSndpSetPan(sound_player, 64);
		alSndpSetVol(sound_player, 127);
		alSndpPlay(sound_player);
	}
#endif
}

/* =================================================== *
 *                     MUSIC PLAYER                    *
 * =================================================== */

#ifdef ENABLE_AUDIO
static void music_init()
{
	ALSeqpConfig music_cfg =
	{
		.maxVoices = MAX_VOICES,
		.maxEvents = MAX_EVENTS,
		.maxChannels = MAX_CHANNELS,
		.heap = &heap,
		.initOsc = 0,
		.updateOsc = 0,
		.stopOsc = 0,
		#ifdef DEBUG_MODE
		.debugFlags = NO_VOICE_ERR_MASK | NOTE_OFF_ERR_MASK | NO_SOUND_ERR_MASK,
		#endif
	};

	alSeqpNew(music_player, &music_cfg);
	debug_printf("[Audio] Initialized music player\n");
}
#endif

void music_set_bank(char *ctl_start, char *ctl_end, char *tbl_start)
{
#ifdef ENABLE_AUDIO
	s32 size = ctl_end - ctl_start;
	u8 bank_file[size];
	u8 *bank_file_ptr = bank_file;
	if (size > AUDIO_SEQ_SIZE)
	{
		debug_printf("[Audio] Music bank size exceeds maximum allocated space, cancelling...\n");
		return;
	}

	load_from_rom(bank_file_ptr, ctl_start, size);

	alBnkfNew((ALBankFile *)bank_file_ptr, (u8 *)tbl_start);
	music_bank = ((ALBankFile *)bank_file_ptr)->bankArray[0];
	debug_printf("[Audio] Loaded music bank from address %p\n", ctl_start);

	alSeqpSetBank(music_player, music_bank);
	return;
#endif
}

void music_play_seq(SeqPlayEvent *seq)
{
#ifdef ENABLE_AUDIO
	if (currentSeq == seq->romStart)
	{
		return;
	}
	else if (alSeqpGetState(music_player) == AL_PLAYING)
	{
		alSeqpStop(music_player);
		debug_printf("[Audio] Stopped playback of MIDI at address %p\n", currentSeq);
		pendingSeq = *seq;
	}
	else
	{
		// Read and register the .mid as a sequence
		sequenceLen[targetSeq] = seq->romEnd - seq->romStart;
		load_from_rom((char *) sequenceData[targetSeq], seq->romStart, sequenceLen[targetSeq]);

		alSeqNew(sequences[targetSeq], sequenceData[targetSeq], sequenceLen[targetSeq]);
		alSeqNewMarker(sequences[targetSeq], &sequenceStart[targetSeq], seq->playbackStart);    
		alSeqNewMarker(sequences[targetSeq], &sequenceLoopStart[targetSeq], seq->loopStart);
		alSeqNewMarker(sequences[targetSeq], &sequenceEnd[targetSeq], seq->loopEnd);
		alSeqpLoop(music_player, &sequenceLoopStart[targetSeq], &sequenceEnd[targetSeq], seq->loopCount);
		alSeqpSetVol(music_player, (s16)(0x7fff * 1.0F));

		if (seq->playbackStart)
			alSeqSetLoc(sequences[targetSeq], &sequenceStart[targetSeq]);

		// Play the newly-registered sequence
		alSeqpSetSeq(music_player, sequences[targetSeq]);
		alSeqpPlay(music_player);

		debug_printf("[Audio] Started playback of MIDI at address %p\n", currentSeq);
		currentSeq = seq->romStart;
		targetSeq = (targetSeq + 1) % AUDIO_SEQ_COUNT;
	}
#endif
}