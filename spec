#include "config/stack.h"

/* =================================================== *
 *                     CODE SEGMENT                    *
 * =================================================== */

beginseg
	name    "code"
	flags   BOOT OBJECT
	entry   boot
	// address 0x80000400
	// maxsize 0x00100000
	stack   STACK_ADDR_BOOT + STACK_SIZE_BOOT
	include $(CODESEGMENT)
	include "$(ROOT)/usr/lib/PR/rspboot.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.Rej.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DLX2.Rej.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspS2DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/n_aspMain.o"
	include "$(ROOT)/usr/lib/PR/aspMain.o"
endseg

/* =================================================== *
 *                       OBJECTS                       *
 * =================================================== */

/* beginseg
	name "static"
	flags OBJECT
	number 2
	include "build/static.o"
endseg */

/* beginseg
	name "texture"
	flags OBJECT
	number 3
	include "build/texture.o"
endseg */

/* beginseg
	name "font"
	flags RAW
	include "src/assets/textures/font.bin"
endseg */

/* =================================================== *
 *                        AUDIO                        *
 * =================================================== */

// Music bank: "GenMidiBank"
// =========================================
beginseg
	name "GenMidiBank_ctl"
	flags RAW
	include "src/assets/audio/banks/GenMidi44k.ctl"
endseg
beginseg
	name "GenMidiBank_tbl"
	flags RAW
	include "src/assets/audio/banks/GenMidi44k.tbl"
endseg
// =========================================

// Music bank: "playstation"
// =========================================
beginseg
	name "playstation_ctl"
	flags RAW
	include "src/assets/audio/banks/playstation.ctl"
endseg
beginseg
	name "playstation_tbl"
	flags RAW
	include "src/assets/audio/banks/playstation.tbl"
endseg
// =========================================

beginseg
	name "psx_boot_sample_mid"
	flags RAW
	include "src/assets/audio/music/psx_boot_sample.mid"
endseg

beginseg
	name "aicha_mid"
	flags RAW
	include "src/assets/audio/music/aicha.mid"
endseg

beginseg
	name "thomas_mid"
	flags RAW
	include "src/assets/audio/music/thomas.mid"
endseg

beginseg
	name "wrecking_ball_mid"
	flags RAW
	include "src/assets/audio/music/wrecking_ball.mid"
endseg

beginseg
	name "test_mid"
	flags RAW
	include "src/assets/audio/music/test.seq"
endseg

// Sound bank: "sample.sounds"
// =========================================
beginseg
	name "sfx_warioware_diy_sounds"
	flags RAW
	include "src/assets/audio/sounds/sfx_warioware_diy.sounds"
endseg
beginseg
	name "sfx_warioware_diy_sounds_tbl"
	flags RAW
	include "src/assets/audio/sounds/sfx_warioware_diy.sounds.tbl"
endseg
// =========================================

/* =================================================== *
 *                         WAVE                        *
 * =================================================== */

beginwave
	name "my_hb_game"
	include "code"

	// Sound banks
	include "sample_sounds"
	include "sample_sounds_tbl"

	// Instrument banks
	include "GenMidiBank_ctl"
	include "GenMidiBank_tbl"
	include "playstation_ctl"
	include "playstation_tbl"

	// MIDIs
	include "psx_boot_sample_mid"
	include "aicha_mid"
	include "thomas_mid"
	include "wrecking_ball_mid"
	include "test_mid"
endwave
