#ifndef __AUDIO_H__
#define __AUDIO_H__

	#include "libultra-easy/types.h"
	#include "libultra-easy/scheduler.h"

	#define BANK_DECLARE(filename)	extern char _##filename##_ctlSegmentRomStart[]; \
									extern char _##filename##_ctlSegmentRomEnd[]; \
									extern char _##filename##_tblSegmentRomStart[]; \
									InstBank \
									filename## = \
									{ \
										.ctl_start = _##filename##_ctlSegmentRomStart, \
										.ctl_end = _##filename##_ctlSegmentRomEnd, \
										.tbl_start = _##filename##_tblSegmentRomStart, \
										.initialized = FALSE, \
										.name = #filename \
									};

	#define SFX_DECLARE(filename)	extern char _##filename##_soundsSegmentRomStart[]; \
									extern char _##filename##_soundsSegmentRomEnd[]; \
									extern char _##filename##_sounds_tblSegmentRomStart[]; \
									SoundBank \
									filename## = \
									{ \
										.ctl_start = _##filename##_soundsSegmentRomStart, \
										.ctl_end = _##filename##_soundsSegmentRomEnd, \
										.tbl_start = _##filename##_sounds_tblSegmentRomStart, \
										.initialized = FALSE, \
										.name = #filename \
									};

	#define MIDI_DECLARE(filename)	extern char _##filename##_midSegmentRomStart[]; \
									extern char _##filename##_midSegmentRomEnd[];\
									MIDI \
									filename## = \
									{ \
										.mid_start = _##filename##_midSegmentRomStart, \
										.mid_end = _##filename##_midSegmentRomEnd, \
										.playback_start = 0, \
										.loop_start = 0, \
										.loop_end = -1, \
										.loop_count = -1, \
										.initialized = FALSE, \
										.name = #filename \
									};

	typedef struct
	{
		char *name;
		char *ctl_start;
		char *ctl_end;
		char *tbl_start;
		bool initialized;

		ALBankFile *ctl_file;
		ALBank *bank;
	} InstBank;

	typedef struct
	{
		char *name;
		u8 *data;
		char *mid_start;
		char *mid_end;
		bool initialized;

		int tempo;
		u32 playback_start;
		u32 loop_start;
		u32 loop_end;
		int loop_count;

		ALSeq *sequence;
		ALSeqMarker m_playback_start;
		ALSeqMarker m_loop_start;
		ALSeqMarker m_loop_end;
	} MIDI;

	typedef struct
	{
		ALSndId id;
		ALSound *snd;
		int snd_index;
		bool playing;
	} Sound;

	typedef struct
	{
		u32 soundCount;
		ALSound *sounds[100];
	} SoundFile;

	typedef struct 
	{
		char *name;
		char *ctl_start;
		char *ctl_end;
		char *tbl_start;
		bool initialized;

		int count;
		SoundFile *file;
		Sound *sounds;
	} SoundBank;

	/* ============= FUNCTIONS ============== */

	/**
	 * @brief Initializes the audio subsystem.
	 */
	void init_audio(Scheduler *sc);

	/**
	 * @brief Closes the audio subsystem.
	 */
	void audio_close();

	void music_init_bank(InstBank *bank);
	void music_set_bank(InstBank *bank);
	void midi_set_tempo(MIDI *mid, int tempo);
	void midi_set_markers(MIDI *mid, int playback_start, int loop_start, int loop_end, int loop_count);
	void midi_send_event(long ticks, u8 status, u8 byte1, u8 byte2);
	void midi_init(MIDI *mid);
	void music_set_midi(MIDI *mid);
	void music_player_start();
	void music_player_stop();

	void sound_init_bank(SoundBank *sfx);
	void sound_set_bank(SoundBank *sfx);
	void sound_play(SoundBank *sfx, int snd_index, int snd_slot); // sound_create ?
	void sound_stop(int snd_slot);
	// void sound_destroy(int index);
	// void sound_stop_all();

#endif