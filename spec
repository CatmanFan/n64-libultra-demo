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
	number SEG_STATIC
	include "build/static.o"
endseg */

beginwave
	name "sample"
	include "code"
endwave
