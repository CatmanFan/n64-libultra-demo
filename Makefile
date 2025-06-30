################################################################
#                     /!\ Do not touch! /!\                    #
################################################################

# SDK directories
ROOT = C:/ultra

# Include SDK PR defs
include $(ROOT)/usr/include/make/PRdefs

OBJECTS = $(CODESEGMENT)
# Specifies all relocationable object file names
# Specifies to the ROM image creation tool

# Specifies the ROM image file name
APP = $(OUT).out
# Specifies the symbol file name for debugger
ROM = $(OUT).n64

################################################################
#                       Compiler Settings                      #
################################################################

CC = gcc
LD = ld
MAKEROM = mild

# Default name of the output ROM/object
OUT = homebrew

################################################################
#                          Code Files                          #
################################################################

# Specify the creation of the object file by using the compiler.
# Output the object file having the debugger source code information
# by combining "-g" and "-c".
# -G 0 -I$(INC)/PR -I$(INC) -D_LANGUAGE_C -D_MIPS_SZLONG=32 -
# D_MIPS_SZINT=32 is the required option when you use the N64 OS.
# -I.-D_DEBUG is the option for simple.

# Files to be used by the image
HFILES = 	$(wildcard *.h) \
			$(wildcard ./helpers/*.h) \
			$(wildcard ./assets/fonts/*.h) \
			$(wildcard ./assets/models/*.h) \
CODEFILES = $(wildcard *.c) \
			$(wildcard ./helpers/*.c) \
			$(wildcard ./stages/*.c) \
			$(wildcard ./assets/fonts/*.c)
CODEOBJECTS = $(CODEFILES:.c=.o)
CODESEGMENT = build/codesegment.o

# Specifies the relocationable object file name created
# as a result of linking the program code
# DATAFILES = 
# DATAOBJECTS = $(DATAFILES:.c=.o)

################################################################
#                     ROM debug mode check                     #
################################################################

N64LIB = -lgultra_d
# Set to -lgultra_rom if final

OPTIMIZER = -O2
# Set to -G if final

LCDEFS = -DDEBUG \
# Set to -D_FINALROM -DNDEBUG if final
         -DF3DEX_GBI_2 -DNOT_SPEC -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32

################################################################
#                        Linker Settings                       #
################################################################

LCINCS  = -Wall -I. -I$(ROOT)/usr/include/PR -I$(ROOT)/usr/include
LCOPTS  = -G 0
LASINCS = $(LCINCS)
LASOPTS = -non_shared -G 0
LDFLAGS = $(MKDEPOPT) -L$(ROOT)/usr/lib -L$(ROOT)/usr/lib/PR $(N64LIB) \
          -lgmus -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc
LDIRT   = $(APP)

################################################################
#                          Compilation                         #
################################################################

default: $(ROM)
 
$(CODESEGMENT): $(CODEOBJECTS)
    	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)
 
$(ROM) :  $(OBJECTS)
    	$(MAKEROM) spec -r $(ROM) -e $(APP) 

include $(ROOT)/usr/include/make/commonrules
# This space is needed or makefile errors
