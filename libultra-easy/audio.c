#include <ultra64.h>
#include "libultra-easy.h"

#ifdef ENABLE_AUDIO
/* ============= PROTOTYPES ============= */

#define AUDIO_BUF_SIZE 2048
#define CL_SIZE 4096

static OSThread audio_thread;
static ALGlobals audio_globals;
static ALHeap audio_heap;

static ALSndPlayer snd_player;
static ALSeqPlayer seq_player;

static Acmd audio_cl[2][CL_SIZE];
static u16 audio_buf[3][2 * AUDIO_BUF_SIZE] __attribute__((aligned(16)));
static int audio_current_buf, audio_current_task;

extern Scheduler scheduler;

static void audio_loop(void *arg);

/* ================ DMA ================= */

#define AUDIO_DMA_COUNT 8
#define AUDIO_DMA_BUFSZ (2 * 1024)

struct audio_dmainfo {
	u32 age;
	u32 offset;
};

static struct audio_dmainfo audio_dma[AUDIO_DMA_COUNT];

static u8 audio_dma_buf[AUDIO_DMA_COUNT][AUDIO_DMA_BUFSZ] __attribute__((aligned(16)));

static OSIoMesg audio_dma_msg_io[AUDIO_DMA_COUNT];
static OSMesg audio_dma_msg[AUDIO_DMA_COUNT];
static OSMesgQueue audio_dma_msg_queue;

static OSMesg audio_wait_msg;
static OSMesgQueue audio_wait_msg_queue;

static unsigned audio_dmanext, audio_dmanactive;

extern OSPiHandle* rom_handle;

static s32 audio_dma_callback(s32 addr, s32 len, void *state)
{
	struct audio_dmainfo *dma = audio_dma;
	u32 astart = addr, aend = astart + len;

	// If these samples are already buffered, return the buffer.
	int oldest = 0;
	u32 oldest_age = 0;
	int i;

	(void)state;
	for (i = 0; i < AUDIO_DMA_COUNT; i++)
	{
		u32 dstart = dma[i].offset, dend = dstart + AUDIO_DMA_BUFSZ;
		if (dma[i].age > oldest_age)
		{
			oldest = i;
			oldest_age = dma[i].age;
		}
		if (dstart <= astart && aend <= dend)
		{
			u32 offset = astart - dstart;
			dma[i].age = 0;
			return K0_TO_PHYS(audio_dma_buf[i] + offset);
		}
	}

	// Otherwise, use the oldest buffer to start a new DMA.
	if (oldest_age == 0 || audio_dmanactive >= AUDIO_DMA_COUNT) {
		// If the buffer is in use, don't bother.
		crash_msg("DMA buffer in use"); // FIXME: not in release builds
		return K0_TO_PHYS(audio_dma_buf[oldest]);
	}

	{
		u32 dma_addr = astart & ~1u;
		OSIoMesg *mesg = &audio_dma_msg_io[audio_dmanext];
		audio_dmanext = (audio_dmanext + 1) % AUDIO_DMA_COUNT;
		audio_dmanactive++;
		*mesg = (OSIoMesg){
			.hdr = {.pri = OS_MESG_PRI_NORMAL, .retQueue = &audio_dma_msg_queue},
			.dramAddr = audio_dma_buf[oldest],
			.devAddr = dma_addr,
			.size = AUDIO_DMA_BUFSZ,
		};
		osEPiStartDma(rom_handle, mesg, OS_READ);
		dma[oldest] = (struct audio_dmainfo){
			.age = 0,
			.offset = dma_addr,
		};
		return K0_TO_PHYS(audio_dma_buf[oldest] + (astart & 1u));
	}
}

static ALDMAproc audio_dma_new(void *arg)
{
	(void)arg;
	return audio_dma_callback;
}
#endif

/* ============= FUNCTIONS ============== */

static void play_beep_sound()
{
	#ifdef ENABLE_AUDIO
    /*static ALEnvelope sndenv = {
        .attackTime = 0,
        .decayTime = 1414784,
        .releaseTime = 0,
        .attackVolume = 127,
        .decayVolume = 127,
    };
    int frames = 2;
    int samples = frames * 16;
    int microsec = (u64)samples * 1000000 / 32000;
    sndenv.decayTime = microsec;

	{
		size_t book_size = pak_objects[obj].size;
		ALADPCMBook *book = mem_alloc(book_size);
		pak_load_asset_sync(book, book_size, obj);

		{
			static ALWaveTable wtable = {
				.type = AL_ADPCM_WAVE,
				.flags = 1,
			};
			wtable.base = (u8 *)pak_objects[obj + 1].offset;
			wtable.len = pak_objects[obj + 1].size;
			wtable.waveInfo.adpcmWave.book = book;
			{
				static ALSound snd = {
					.envelope = &sndenv,
					.wavetable = &wtable,
					.samplePan = AL_PAN_CENTER,
					.sampleVolume = AL_VOL_FULL,
					.flags = 1,
				};
				ALSndId sndid = alSndpAllocate(&snd_player, &snd);
				alSndpSetSound(&snd_player, sndid);
				alSndpSetPitch(&snd_player, 1.0f);
				alSndpSetPan(&snd_player, 64);
				alSndpSetVol(&snd_player, 30000);
				alSndpPlay(&snd_player);
			}
		}
	}*/
	#endif
}

void init_audio()
{
	#ifdef ENABLE_AUDIO
	ALSynConfig audio_cfg;
	ALSndpConfig snd_player_cfg;
	ALSeqpConfig seq_player_cfg;
	int i;

    // Mark all DMA buffers as "old" so they get used.
    for (i = 0; i < AUDIO_DMA_COUNT; i++)
        audio_dma[i].age = 1;

	// Reset current buffer index
	audio_current_buf = 0;

	osCreateMesgQueue(&audio_dma_msg_queue, audio_dma_msg, AUDIO_DMA_COUNT);
	osCreateMesgQueue(&audio_wait_msg_queue, audio_wait_msg, 1);

	alHeapInit(&audio_heap, (u8 *)RAMBANK_7, HEAP_SIZE);
	audio_cfg.outputRate = osAiSetFrequency(AUDIO_BITRATE);
	debug_printf("[Audio] Bitrate: %d\n", AUDIO_BITRATE);

	// Set up ALSynConfig
	audio_cfg.maxVVoices = 24;
	audio_cfg.maxPVoices = 24;
	audio_cfg.maxUpdates = 64;
	audio_cfg.dmaproc = audio_dma_new;
	audio_cfg.heap = &audio_heap;
	audio_cfg.fxType = AL_FX_SMALLROOM;

	alInit(&audio_globals, &audio_cfg);
	debug_printf("[Audio] Initialized global config\n");

	// Set up sound player
	snd_player_cfg.maxSounds = audio_cfg.maxVVoices;
	snd_player_cfg.maxEvents = 32;
	snd_player_cfg.heap = &audio_heap;
	alSndpNew(&snd_player, &snd_player_cfg);
	debug_printf("[Audio] Initialized sound player\n");

	// Set up music player
	seq_player_cfg.maxVoices = audio_cfg.maxVVoices;
	seq_player_cfg.maxEvents = 32;
	seq_player_cfg.maxChannels = 16;
	seq_player_cfg.heap = &audio_heap;
	alSeqpNew(&seq_player, &seq_player_cfg);
	debug_printf("[Audio] Initialized music player\n");

	// Start the audio thread
	osCreateThread(&audio_thread, ID_AUDIO, audio_loop, NULL, STACK_ADDR(audio_stack, STACK_SIZE_AUDIO), PR_AUDIO);
	osStartThread(&audio_thread);
	
	play_beep_sound();
	#endif
}

static void audio_frame()
{
	#ifdef ENABLE_AUDIO
	int i;
    // Return finished DMA messages.
    {
        int nactive = audio_dmanactive;
        for (;;) {
            OSMesg mesg;
            int r = osRecvMesg(&audio_dma_msg_queue, &mesg, OS_MESG_NOBLOCK);
            if (r == -1) {
                break;
            }
            nactive--;
        }
        audio_dmanactive = nactive;
    }

    // Increase the age of all sample buffers.
    for (i = 0; i < AUDIO_DMA_COUNT; i++)
        audio_dma[i].age++;

    // Create the command list.
	{
		u16 *buffer = audio_buf[audio_current_buf];
		s32 cmdlen = 0;
		Acmd *al_start = audio_cl[audio_current_task];
		Acmd *al_end = alAudioFrame(al_start, &cmdlen, (s16 *)K0_TO_PHYS(buffer), AUDIO_BUF_SIZE);
		my_assert(al_end - al_start > CL_SIZE, "Audio assertion failed");

		audio_current_task++;
		if (audio_current_task >= 2) audio_current_task = 0;

		// Create and submit the task.
		if (cmdlen == 0)
		{
			bzero(buffer, 4 * AUDIO_BUF_SIZE);
			osWritebackDCache(buffer, 4 * AUDIO_BUF_SIZE);
		}
		else
		{
			OSTask l_task =
			{{
				.type = M_AUDTASK,
				.flags = OS_TASK_DP_WAIT,
				.ucode_boot = (u64*)rspbootTextStart,
				.ucode_boot_size = ((u32)rspbootTextEnd-(u32)rspbootTextStart),
				.ucode_data = (u64 *)aspMainDataStart,
				.ucode_data_size = SP_UCODE_DATA_SIZE,
				.ucode = (u64 *)aspMainTextStart,
				.ucode_size = SP_UCODE_SIZE,
				.dram_stack = NULL,
				.dram_stack_size = 0,
				.data_ptr = (u64 *)al_start,
				.data_size = sizeof(Acmd) * (al_end - al_start),
			}};
			osWritebackDCache(al_start, sizeof(Acmd) * (al_end - al_start));

			// Do the thing
			scheduler.task_audio = &l_task;
			debug_printf("[Audio] Beginning audio task at buffer %d (%p)\n", audio_current_buf, &audio_buf[audio_current_buf]);
			osSpTaskStart(&l_task);
		}
	}

	scheduler.audio_notify = &audio_wait_msg_queue;
	osRecvMesg(&audio_wait_msg_queue, NULL, OS_MESG_BLOCK);

	audio_current_buf++;
	if (audio_current_buf >= 3) { audio_current_buf = 0; }
	#endif
}

void load_inst(char *start, char *end, char *wbank)
{
	#ifdef ENABLE_AUDIO
	// Read and register the instrument bank and its pointers. (instruments ptr file)
	//load_binary((void*)start, (void*)ptr_inst_buf, end-start);
	#endif
}

void play_bgm(char *start, char *end)
{
	#ifdef ENABLE_AUDIO
	// Read and register the background music (song bin file)
	//load_binary((void*)start,(void*)bgm_buf, end-start);

	// Set override for the instrument bank pointers

	// Play the song utilizing the instruments bank
	#endif
}

void load_sounds(char *ptr_start, char *ptr_end, char *wbank, char *start, char *end)
{
	#ifdef ENABLE_AUDIO
	// Read and register the instrument bank and its pointers. (instruments ptr file)
	//load_binary((void*)ptr_start, (void*)ptr_sfx_buf, ptr_end-ptr_start);

	// Read and register the sound effects. (sound effects bfx file)
	//load_binary((void*)start, (void*)sfx_buf, end-start);
	#endif
}

void play_sound(int index)
{
	#ifdef ENABLE_AUDIO
	#endif
}

static void audio_loop(void *arg)
{
	while (1)
	{
		int scheduler_msg = scheduler_get_status();
		switch (scheduler_msg)
		{
			case SC_MSG_VSYNC:
				audio_frame();
				break;
		}
	}
}