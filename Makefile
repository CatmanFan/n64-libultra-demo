include util.mk

# Output file name
TARGET_STRING := homebrew

# Location of the SDK
LIBULTRA := C:/n64dev/libultra/ultra

# Preprocessor definitions
DEFINES := F3DEX_GBI_2=1 S2DEX_GBI_2=1 NAUDIO=1

# LD flags
LINK_FLAGS := -L. -L$(LIBULTRA)/usr/lib -L$(LIBULTRA)/usr/lib/PR -L$(N64_LIBGCCDIR) -L$(LIBULTRA)/usr/lib/n64 -lgcc -lmus -lultra_rom
# For NuSys, L/usr/lib/n64/nusys + lnusys + lnualstl

SRC_DIRS :=
USE_DEBUG := 0

# TOOLS_DIR := tools

# Whether to hide commands or not
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
  V := @
endif

# Whether to colorize build messages
COLOR ?= 1

# ifeq ($(filter clean distclean print-%,$(MAKECMDGOALS)),)
  # Make tools if out of date
  # $(info Building tools...)
  # DUMMY != $(MAKE) -s -C $(TOOLS_DIR) >&2 || echo FAIL
    # ifeq ($(DUMMY),FAIL)
      # $(error Failed to build tools)
    # endif
  $(info Building ROM...)
# endif

#==============================================================================#
# Target Executable and Sources                                                #
#==============================================================================#
# BUILD_DIR is the location where all build artifacts are placed
BUILD_DIR := build
ROM            := $(TARGET_STRING).z64
ELF            := $(BUILD_DIR)/$(TARGET_STRING).elf
LD_SCRIPT      := builder.ld
CODESEGMENT	   := $(BUILD_DIR)/codesegment.o
BOOT		:= $(LIBULTRA)/usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	:= $(BUILD_DIR)/boot.6102.o

# Directories containing source files
SRC_DIRS     += lib src src/assets

C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

SRC_OBJECTS := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
			$(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o))
			
# Object files
O_FILES := $(SRC_OBJECTS) \
		   $(BOOT_OBJ) 

# Automatic dependency files
DEP_FILES := $(SRC_OBJECTS:.o=.d) $(BUILD_DIR)/$(LD_SCRIPT).d

#==============================================================================#
# Compiler Options                                                             #
#==============================================================================#

# detect prefix for MIPS toolchain
ifneq ($(call find-command,mips64-elf-ld),)
  CROSS := mips64-elf-
else ifneq ($(call find-command,mips-n64-ld),)
  CROSS := mips-n64-
else ifneq ($(call find-command,mips64-ld),)
  CROSS := mips64-
else ifneq ($(call find-command,mips-linux-gnu-ld),)
  CROSS := mips-linux-gnu-
else ifneq ($(call find-command,mips64-linux-gnu-ld),)
  CROSS := mips64-linux-gnu-
else ifneq ($(call find-command,mips64-none-elf-ld),)
  CROSS := mips64-none-elf-
else ifneq ($(call find-command,mips-ld),)
  CROSS := mips-
else
  $(error Unable to detect a suitable MIPS toolchain installed)
endif

# Main compilation tool paths.
AS        := $(CROSS)as
CC        := $(CROSS)gcc
CPP       := cpp
LD        := $(CROSS)ld
AR        := $(CROSS)ar
OBJDUMP   := $(CROSS)objdump
OBJCOPY   := $(CROSS)objcopy
MAKEMASK  := makemask

INCLUDE_DIRS += $(LIBULTRA)/usr/include $(LIBULTRA)/usr/include/PR include $(BUILD_DIR) src asm .

C_DEFINES := $(foreach d,$(DEFINES),-D$(d))
DEF_INC_CFLAGS := $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(C_DEFINES)

CFLAGS      := -Werror=implicit-function-declaration -ffunction-sections -fdata-sections -mdivide-breaks -G 0 -Os -mabi=32 -ffreestanding -mfix4300 $(DEF_INC_CFLAGS)
CFLAGS +=  -nostdinc -DTARGET_N64 -D_LANGUAGE_C
ASFLAGS     := -march=vr4300 -mabi=32 $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(foreach d,$(DEFINES),--defsym $(d))

# C preprocessor flags
CPPFLAGS := -P -Wno-trigraphs $(DEF_INC_CFLAGS)

# tools
PRINT = printf

ifeq ($(COLOR),1)
NO_COL  := \033[0m
RED     := \033[0;31m
GREEN   := \033[0;32m
BLUE    := \033[0;34m
YELLOW  := \033[0;33m
BLINK   := \033[33;5m
endif

# Common build print status function
define print
  @$(PRINT) "$(GREEN)$(1) $(YELLOW)$(2)$(GREEN) -> $(BLUE)$(3)$(NO_COL)\n"
endef

#==============================================================================#
# Main Targets                                                                 #
#==============================================================================#

# Default target
default: $(ROM)

clean:
	$(RM) -r $(BUILD_DIR)

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))
          # $(addprefix $(FILESYSTEM_ROOT)/,$(GFX_DIRS))

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

#==============================================================================#
# Compilation Recipes                                                          #
#==============================================================================#

# Compile C code
$(BUILD_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -MMD -o $@ $<

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) -c $(CFLAGS) -std=c++17 -Wno-register -MMD -o $@ $<

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	$(CC) -c $(CFLAGS) -MMD -o $@ $<

# Assemble assembly code
$(BUILD_DIR)/%.o: %.s
	$(call print,Assembling:,$<,$@)
	$(V)$(AS) $(ASFLAGS) -MD $(BUILD_DIR)/$*.d  -o $@ $<

# Run linker script through the C preprocessor
$(BUILD_DIR)/$(LD_SCRIPT): $(LD_SCRIPT)
	$(call print,Preprocessing linker script:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ -MF $@.d -o $@ $<

$(BOOT_OBJ): $(BOOT)
	$(V)$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

# Link final ELF file
$(ELF): $(O_FILES) $(BUILD_DIR)/$(LD_SCRIPT)
	@$(PRINT) "$(GREEN)Linking ELF file:  $(BLUE)$@ $(NO_COL)\n"
	$(V)$(LD) -L $(BUILD_DIR) -T $(BUILD_DIR)/$(LD_SCRIPT) -Map $(BUILD_DIR)/$(TARGET_STRING).map --gc-sections --no-check-sections -o $@ $(O_FILES) $(LINK_FLAGS)

# Build ROM
$(ROM): $(ELF)
	$(call print,Building ROM:,$<,$@)
	$(V)$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $< $@ -O binary
	$(V)$(MAKEMASK) $@

.PHONY: clean default
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
