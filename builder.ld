OUTPUT_ARCH (mips)

#define BEGIN_SEG(name, addr) \
    _##name##SegmentStart = ADDR(.name); \
    _##name##SegmentRomStart = __romPos; \
    .name addr : AT(__romPos)

#define END_SEG(name) \
    _##name##SegmentEnd = ADDR(.name) + SIZEOF(.name); \
    _##name##SegmentRomEnd = __romPos + SIZEOF(.name); \
    __romPos += SIZEOF(.name);

#define BEGIN_NOLOAD(name) \
    _##name##SegmentBssStart = ADDR(.name.noload); \
    .name.noload (NOLOAD) :

#define END_NOLOAD(name) \
    _##name##SegmentBssEnd = ADDR(.name.noload) + SIZEOF(.name.noload); \
    _##name##SegmentBssSize = SIZEOF(.name.noload);

SECTIONS
{
	__romPos = 0;

	BEGIN_SEG(boot, 0x04000000)
	{
		build/obj/rom_header.o(.text);
		build/obj/boot.6102.o(.data);
	}
	END_SEG(boot)

	BEGIN_SEG(code, 0x80000400) SUBALIGN(16)
	{
		/* text */
		build/obj/entry.o(.text);
		build/codesegment.o(.text);
		/usr/lib/n64/PR/rspboot.o(.text);
		/usr/lib/n64/PR/gspF3DEX2.fifo.o(.text);
		/usr/lib/n64/PR/aspMain.o(.text);

		/* data */
		build/codesegment.o(.data*);
		/usr/lib/n64/PR/rspboot.o(.data*);
		/usr/lib/n64/PR/gspF3DEX2.fifo.o(.data*);
		/usr/lib/n64/PR/aspMain.o(.data*);

		/* rodata */
		build/codesegment.o(.rodata*);
	}
	END_SEG(code)

	BEGIN_NOLOAD(code)
	{
		build/codesegment.o(COMMON);
		build/codesegment.o(.scommon*);
		build/codesegment.o(.bss*);
		. = ALIGN(0x8);
	}
	END_NOLOAD(code)

	BEGIN_SEG(fxPtrs, __romPos)
    {
        . = ALIGN(16);
		KEEP(BUILD_ROOT/sounds.ptr.o(.*data*));
    }

	BEGIN_SEG(fxBank, __romPos)
    {
        . = ALIGN(16);
		KEEP(BUILD_ROOT/fxbank.bin.o(.*data*));
    }
	END_SEG(wavtable)

	BEGIN_SEG(wavTable, __romPos)
    {
        . = ALIGN(16);
        KEEP(BUILD_ROOT/wavtable.wbk.o(.*data*));
    }
	END_SEG(wavtable)

	/* Discard everything not specifically mentioned above. */
	/DISCARD/ :
	{
		*(*);
	}
}
