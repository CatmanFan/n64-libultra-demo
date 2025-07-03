#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <PR/libmus.h>

/* ============= FUNCTIONS ============== */

extern void init_audio();

extern void load_inst(char* pbank_start, char* pbank_end, char* wbank_start);
extern void play_bgm(char* title_start, char* title_end);

extern void load_sounds(char* pbank_start, char* pbank_end, char* wbank_start, char* title_start, char* title_end);
extern void play_sound(int index);

#endif