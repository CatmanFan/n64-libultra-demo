[N64Project]
TargetName=homebrew.n64
BuildFolder=build
ROMHeader_Name=TEST
ROMHeader_Manufacturer=N
ROMHeader_ID=HB
ROMHeader_Country=E
Flags_GCC=-Wall -I. -IC:/n64proj/empty/src -IC:/n64proj/empty/usb -IC:/ultra/usr/include/PR -IC:/ultra/usr/include -G -O0 -DF3DEX_GBI_2 -DS2DEX_GBI_2 -DNOT_SPEC -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32 -DNAUDIO
Flags_LD=-L. -LC:/ultra/usr/lib -LC:/ultra/usr/lib/PR -LC:/ultra/gcc/mipse/lib -lkmc -ln_gmus -ln_gaudio_sc -lgultra
Flags_MILD=
