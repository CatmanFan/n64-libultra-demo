include $(ROOT)/usr/include/make/PRdefs

LIB = $(ROOT)/usr/lib
LPR = $(LIB)/PR
INC = $(ROOT)/usr/include
CC = gcc
LD = ld
MAKEROM = mild

.c.o:
$(CC) -Wall -g -G 0 -c -I. -I$(INC)/PR -I$(INC) -D_LANGUAGE_C
-DF3DEX_GBI_2 -DNOT_SPEC -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32 -D_DEBUG $<

# Specify the creation of the object file by using the compiler.
# Output the object file having the debugger source code information
# by combining "-g" and "-c".
# -G 0 -I$(INC)/PR -I$(INC) -D_LANGUAGE_C -D_MIPS_SZLONG=32 -
# D_MIPS_SZINT=32 is the required option when you use the N64 OS.
# -I.-D_DEBUG is the option for simple.

OUT = homebrew
# Specifies the ROM image file name
APP = $(OUT).out
# Specifies the symbol file name for debugger
TARGETS = $(OUT).n64

# Files to be used by the image
HFILES = 
CODEFILES = 
CODEOBJECTS = $(CODEFILES:.c=.o)
CODESEGMENT = codesegment.o

# Specifies the relocationable object file name created
# as a result of linking the program code
DATAFILES = \ 
:
DATAOBJECTS = $(DATAFILES:.c=.o)

OBJECTS =$(CODESEGMENT) $(DATAOBJECTS)
# Specifies all relocationable object file names
# Specifies to the ROM image creation tool

LDFLAGS = $(MKDEPOPT) -L$(LIB) -L$(LPR) -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc
# Specifies the option to specify to the linker
# "MKDEPORT" is a reserved name and required option when you create
# the relocationable object file by using the linker.
# "-L$(LIB)-lgultra_d" is the option to link the N64 OS library.
# "-L$(GCCDIR)/mipse/lib-lkmc" is the required option when you link
# the object file created by the compiler

# Default command redirects to $(TARGETS), which in turn creates ROM image file
default: $(TARGETS)
$(CODESEGMENT): $(CODEOBJECTS)
# Specify the dependent relation among "codesegment.o", the .o file and "Makefile"
# Provide the following process when ".o" and "Makefile' are updated:
$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)
# Specify the creation of the relocationable object file by using the linker.
# "-o $(CODESEGMENT)" is the option to specify the output
# file name.
# "-r" is the option to create the relocationable object file.
# $(CODEOBJECTS) Specifies the linking object file name
# $(LDFLAGS) is the specification of passing other options to the linker

$(TARGETS): $(OBJECTS)
# Specify the dependent relation between the ROM image file and all ".o" files.
# Provide the following process if ".o" is updated:
$(MAKEROM) spec -r $(TARGETS) -e $(APP)
# Specify the creation of the ROM image file by using
# the ROM image creation tool.
# "spec" is the text file to specify the ROM image to the ROM
# image creation tool. This will be mentioned later.
# "-r $(TARGETS)" is the option to specify the ROM image
# file name.
# "-e $(APP)" is the option to specify the symbol file name.
makemask $(TARGETS)