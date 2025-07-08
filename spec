#include "config/global.h"

// ------------------------------

/* ======= CODESEGMENT ======= */

beginseg
	name    "code"
	flags   BOOT OBJECT
	entry   boot
	stack   boot_stack + STACK_SIZE_BOOT
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

// ------------------------------

/* ========= OBJECTS ========= */

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

// ------------------------------

/* ========== AUDIO ========== */

// Pointer to bank sounds
beginseg
	name "pbank_inst1"
	flags RAW
	include "src/assets/audio/banks/xp_remix.ptr"
endseg

beginseg
	name "pbank_sfx1"
	flags RAW
	include "src/assets/audio/banks/sfx1.ptr"
endseg

// Banks containing sound data
beginseg
	name "wbank_inst1"
	flags RAW
	include "src/assets/audio/banks/xp_remix.wbk"
endseg

beginseg
	name "wbank_sfx1"
	flags RAW
	include "src/assets/audio/banks/sfx1.wbk"
endseg

// Music score
beginseg
	name "bgm1"
	flags RAW
	include "src/assets/audio/music/xp_remix.bin"
endseg

// Sound effects list
beginseg
	name "sfx1"
	flags RAW
	include "src/assets/audio/sfx/sfx1.bfx"
endseg

// ------------------------------

beginwave
	name "sample"
	include "code"

	include "pbank_inst1"
	include "pbank_sfx1"

	include "wbank_inst1"
	include "wbank_sfx1"

	include "bgm1"
	include "sfx1"
endwave
