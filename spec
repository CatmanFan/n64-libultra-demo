#include "config/global.h"

/* =================================================== *
 *                     CODE SEGMENT                    *
 * =================================================== */

beginseg
	name    "code"
	flags   BOOT OBJECT
	entry   boot
	address 0x80000400
	maxsize 0x00100000
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

// Music bank: "playstation"
// =========================================
beginseg
	name "bgmCtl_playstation"
	flags RAW
	include "src/assets/audio/banks/playstation.ctl"
endseg
beginseg
	name "bgmTbl_playstation"
	flags RAW
	include "src/assets/audio/banks/playstation.tbl"
endseg
// =========================================

beginseg
	name "bgm_sce_logo_0"
	flags RAW
	include "src/assets/audio/music/sce_logo_0.mid"
endseg

// Sound bank: "sample.sounds"
// =========================================
beginseg
	name "sfx_sample"
	flags RAW
	include "src/assets/audio/sounds/sample.sounds"
endseg
beginseg
	name "sfxTbl_sample"
	flags RAW
	include "src/assets/audio/sounds/sample.sounds.tbl"
endseg
// =========================================

/* =================================================== *
 *                         WAVE                        *
 * =================================================== */

beginwave
	name "my_hb_game"
	include "code"

	// Sound banks
	include "sfx_sample"
	include "sfxTbl_sample"

	// Instrument banks
	include "bgmCtl_playstation"
	include "bgmTbl_playstation"

	// MIDIs
	include "bgm_sce_logo_0"
endwave
