#include <ultra64.h>

/* === Configuration === */
#include "config/global.h"
#include "config/stack.h"
#include "config/audio.h"
#include "config/video.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
#include "libultra-easy/audio.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/rcp.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "stages.h"
#include "strings.h"

/* [AUDIO FILES]
========================================= */
// Music banks
BANK_DECLARE(GenMidiBank)
BANK_DECLARE(playstation)

// Sound banks
SFX_DECLARE(sfx_warioware_diy)

// MIDIs
MIDI_DECLARE(psx_boot_sample)
MIDI_DECLARE(aicha)
MIDI_DECLARE(thomas)
MIDI_DECLARE(test)

/* [VARIABLES]
========================================= */
// Message timer
static f64 msg_time_marker;
static f64 msg_timer;
static int msg_screen;

static InstBank *my_bank;
static MIDI *my_midi;

/* [MAIN FUNCTIONS]
========================================= */

/* ==============================
 * Initializes stage.
 * ============================== */
void test_03_init()
{
	my_bank = &playstation;
	music_init_bank(my_bank);
	music_set_bank(my_bank);

	midi_set_tempo(&test, 240);
	// midi_set_markers(&psx_boot_sample, 0, 20, 30, -1);

	my_midi = &psx_boot_sample;
	midi_init(my_midi);
	music_set_midi(my_midi);

	sound_init_bank(&sfx_warioware_diy);
	// midi_send_event(0, 0xB0, 0x0A, 0);
	// midi_send_event(0, 0xB1, 0x0A, 127);
	// midi_send_event(0, 0xB2, 0x0A, 0);
	// midi_send_event(0, 0xB3, 0x0A, 127);
	// midi_send_event(0, 0xC0, 48, 0);
	// midi_send_event(0, 0xC1, 48, 0);
	// midi_send_event(0, 0xC2, 48, 0);
	// midi_send_event(0, 0xC3, 48, 0);
	// midi_send_event(0, 0x90, 55, round(0.52 * 127));
	// midi_send_event(0, 0x91, 55, round(0.99 * 127));
	// midi_send_event(0, 0x92, 55, round(0.99 * 127));
	// midi_send_event(0, 0x93, 55, round(0.99 * 127));

	time_reset();
	msg_timer = 5.0;
	msg_time_marker = 5.0;
	msg_screen = 0;
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void test_03_update()
{
	if (joypad_button(A, 0))
		sound_play(&sfx_warioware_diy, 0, 0);
	if (joypad_button(B, 0))
		sound_play(&sfx_warioware_diy, 1, 1);
	if (joypad_button(Z, 0))
		sound_play(&sfx_warioware_diy, 2, 2);
	if (joypad_button(L, 0))
		sound_play(&sfx_warioware_diy, 3, 3);
	if (joypad_button(R, 0))
		sound_play(&sfx_warioware_diy, 4, 4);

	music_player_start();

	msg_timer = msg_time_marker - time_current();
	if (msg_timer < 0.0)
	{
		msg_screen = (msg_screen + 1) % 5;
		while (msg_timer < 0.0)
		{
			msg_time_marker += 5.0;
			msg_timer += 5.0;
		}
	}
}

/* ==============================
 * Renders frame.
 * ============================== */
void test_03_render()
{
	extern int audio_sample_xxx;

	clear_zfb();
	clear_cfb(64, 64, 64);

	console_clear();
	switch (msg_screen)
	{
		#ifdef ENABLE_AUDIO
		case 0:
			console_puts("Audio test (1/4)\n");
			console_puts("Heap address: %x", AUDIO_HEAP_ADDR);
			console_puts("Heap size:    %x\n", AUDIO_HEAP_SIZE);
			console_puts("%d\n", audio_sample_xxx);

			// if (my_bank != NULL && my_bank->initialized)
			// {
				// console_puts("Bank:                     %s", my_bank->name);
				// console_puts("Bank RAM address:         %x", my_bank->ctl_file);
				// console_puts("");
				// console_puts("Bank sample rate:         %d", my_bank->bank->sampleRate);

				// if (my_bank->bank->flags) console_puts("Bank instrument type:     Pointer");
				// else console_puts("Bank instrument type:     Offset");
			// }
			// else
			// {
				// console_puts("Bank data unavailable");
			// }
			break;

		case 1:
			console_puts("Audio test (2/4)\n");
			console_puts("%d\n", audio_sample_xxx);

			// if (my_bank != NULL && my_bank->initialized)
			// {
				// console_puts("Acoustic Grand Piano sounds: %d", my_bank->bank->instArray[0]->soundCount);
				// console_puts("Acoustic Grand Piano volume: %d", my_bank->bank->instArray[0]->volume);
				// console_puts("Acoustic Grand Piano keymap: %d", my_bank->bank->instArray[0]->soundArray[0]->keyMap->keyBase);
				// console_puts("\nString Ensemble 1 sounds: %d", my_bank->bank->instArray[48]->soundCount);
				// console_puts("String Ensemble 1 volume: %d", my_bank->bank->instArray[48]->volume);
				// console_puts("String Ensemble 1 keymap: %d", my_bank->bank->instArray[48]->soundArray[0]->keyMap->keyBase);
			// }
			// else
			// {
				// console_puts("Bank data unavailable");
			// }
			break;

		case 2:
			console_puts("Audio test (3/4)\n");

			if (my_midi != NULL && my_midi->initialized)
			{
				console_puts("MIDI name:                %s", my_midi->name);
				console_puts("MIDI RAM address:         %x", my_midi->sequence);
				console_puts("");
				console_puts("MIDI ROM address:         %p", my_midi->sequence->base);
				console_puts("MIDI length:              %d", my_midi->sequence->len);
				console_puts("MIDI track start:         %p", my_midi->sequence->trackStart);
				console_puts("MIDI next event:          %p", my_midi->sequence->curPtr);
				console_puts("MIDI last event in ticks: %d", my_midi->sequence->lastTicks);
				console_puts("MIDI active status:       %d", my_midi->sequence->lastStatus);
			}
			else
			{
				console_puts("MIDI data unavailable");
			}
			break;

		case 3:
			console_puts("Audio test (4/4)\n");

			if (sfx_warioware_diy.initialized)
			{
				console_puts("Sound bank name: %s", sfx_warioware_diy.name);
				console_puts("Sound bank count: %d", sfx_warioware_diy.count);
				console_puts("Sound bank RAM address: %x", sfx_warioware_diy.file);
			}
			else
			{
				console_puts("Sound data unavailable");
			}
			break;

		#else
		case 0:
			console_puts("Audio test\n");
			console_puts("Audio not available");
			break;
		#endif

		default:
			console_puts("Controller test\n");
			console_puts("A        %d", joypad_button(A, 0));
			console_puts("B        %d", joypad_button(B, 0));
			console_puts("L        %d", joypad_button(L, 0));
			console_puts("R        %d", joypad_button(R, 0));
			console_puts("Z        %d", joypad_button(Z, 0));
			console_puts("START    %d", joypad_button(START, 0));
			console_puts("D_UP     %d", joypad_button(DPAD_UP, 0));
			console_puts("D_DOWN   %d", joypad_button(DPAD_DOWN, 0));
			console_puts("D_LEFT   %d", joypad_button(DPAD_LEFT, 0));
			console_puts("D_RIGHT  %d", joypad_button(DPAD_RIGHT, 0));
			console_puts("C_UP     %d", joypad_button(C_UP, 0));
			console_puts("C_DOWN   %d", joypad_button(C_DOWN, 0));
			console_puts("C_LEFT   %d", joypad_button(C_LEFT, 0));
			console_puts("C_RIGHT  %d", joypad_button(C_RIGHT, 0));
			break;
	}
	console_draw_dl();
}