#include "config.h"

beginseg
	name    "code"
	flags   BOOT OBJECT
	entry   boot
	stack   boot_stack + STACK_SIZE_BOOT
	include $(CODESEGMENT)
	include "$(ROOT)/usr/lib/PR/rspboot.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.xbus.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
endseg

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
	include "assets/textures/font.bin"
endseg */

beginwave
	name "sample"
	include "code"
endwave
