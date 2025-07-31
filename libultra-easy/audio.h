#ifndef __AUDIO_H__
#define __AUDIO_H__

	#include "libultra-easy/types.h"
	#include "libultra-easy/scheduler.h"

	typedef struct 
	{
		u32 soundCount;
		ALSound* sounds[100];
	} SoundArray;

	typedef struct
	{
		char* name;
		void* ctl_start;
		void* ctl_end;
		void* tbl_start;
		bool is_sound;
		ALBankFile* bank_file;
		SoundArray* sound_file;
	} AudioBank;

	typedef struct
	{
		char* romStart;
		char* romEnd;
		int playbackStart;
		int loopStart;
		int loopEnd;
		int loopCount;
	} SeqPlayEvent;

	/* ============= FUNCTIONS ============== */

	/**
	 * @brief Initializes the audio subsystem.
	 */
	void init_audio(Scheduler *sc);
	void audio_close();

	void sound_set_bank(const char *name);
	void music_play_seq(SeqPlayEvent *seq);
	void sound_play(int index);

	#define music_load_bank(N) \
		{ \
			extern void music_set_bank(char *ctl_start, char *ctl_end, char *tbl_start); \
			extern char _bgmCtl_ ## N ## SegmentRomStart[]; \
			extern char _bgmCtl_ ## N ## SegmentRomEnd[]; \
			extern char _bgmTbl_ ## N ## SegmentRomStart[]; \
			music_set_bank(_bgmCtl_ ## N ## SegmentRomStart, _bgmCtl_ ## N ## SegmentRomEnd, _bgmTbl_ ## N ## SegmentRomStart); \
		}

	#define music_play(N, playback_S, loop_S, loop_E, loop_C) \
		{ \
			extern char _ ## N ## SegmentRomStart[]; \
			extern char _ ## N ## SegmentRomEnd[]; \
			SeqPlayEvent seq = \
			{ \
				_ ## N ## SegmentRomStart, \
				_ ## N ## SegmentRomEnd, \
				playback_S, \
				loop_S, \
				loop_E, \
				loop_C, \
			}; \
			music_play_seq(&seq); \
		}

#endif