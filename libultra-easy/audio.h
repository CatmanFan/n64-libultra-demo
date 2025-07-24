#ifndef __AUDIO_H__
#define __AUDIO_H__

	#include "libultra-easy/types.h"

	/* ============= FUNCTIONS ============== */

	/**
	 * @brief Initializes the audio subsystem.
	 */
	void init_audio();

	void load_inst(char* pbank_start, char* pbank_end, char* wbank_start);
	void play_bgm(char* title_start, char* title_end);

	void load_sounds(char* pbank_start, char* pbank_end, char* wbank_start, char* title_start, char* title_end);
	void play_sound(int index);

#endif