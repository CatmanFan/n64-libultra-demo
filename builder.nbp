[N64Project]
TargetName=homebrew.n64
BuildFolder=build
ROMHeader_Name=TEST
ROMHeader_Manufacturer=N
ROMHeader_ID=HB
ROMHeader_Country=E
Flags_GCC=-Wall -I. -IC:/ultra/usr/include/PR -IC:/ultra/usr/include -G 0 -DF3DEX_GBI_2 -DNOT_SPEC -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32 
Flags_LD=-L. -LC:/ultra/usr/lib -LC:/ultra/usr/lib/PR -LC:/ultra/gcc/mipse/lib -lkmc
Flags_MILD=
