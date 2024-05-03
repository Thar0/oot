MAKEFLAGS += --no-builtin-rules

# Ensure the build fails if a piped command fails
SHELL = /bin/bash
.SHELLFLAGS = -o pipefail -c

# Build options can either be changed by modifying the makefile, or by building with 'make SETTING=value'

# If COMPARE is 1, check the output md5sum after building
COMPARE := 1
# If NON_MATCHING is 1, define the NON_MATCHING C flag when building
NON_MATCHING := 0
# If ORIG_COMPILER is 1, compile with QEMU_IRIX and the original compiler
ORIG_COMPILER := 0
# If COMPILER is "gcc", compile with GCC instead of IDO.
COMPILER := ido
# Target game version. Currently the following versions are supported:
#   gc-eu          GameCube Europe/PAL
#   gc-eu-mq       GameCube Europe/PAL Master Quest
#   gc-eu-mq-dbg   GameCube Europe/PAL Master Quest Debug (default)
# The following versions are work-in-progress and not yet matching:
#   gc-us          GameCube US
VERSION := gc-eu-mq-dbg
# Number of threads to extract and compress with
N_THREADS := $(shell nproc)
# Music Macro Language Version
MML_VERSION := MML_VERSION_OOT
# Check code syntax with host compiler
RUN_CC_CHECK := 1

CFLAGS ?=
CPPFLAGS ?=
CPP_DEFINES ?=

# ORIG_COMPILER cannot be combined with a non-IDO compiler. Check for this case and error out if found.
ifneq ($(COMPILER),ido)
  ifeq ($(ORIG_COMPILER),1)
    $(error ORIG_COMPILER can only be used with the IDO compiler. Please check your Makefile variables and try again)
  endif
endif

ifeq ($(COMPILER),gcc)
  CPP_DEFINES += -DCOMPILER_GCC
  NON_MATCHING := 1
endif

# Set prefix to mips binutils binaries (mips-linux-gnu-ld => 'mips-linux-gnu-') - Change at your own risk!
# In nearly all cases, not having 'mips-linux-gnu-*' binaries on the PATH is indicative of missing dependencies
MIPS_BINUTILS_PREFIX := mips-linux-gnu-

ifeq ($(NON_MATCHING),1)
  CPP_DEFINES += -DNON_MATCHING -DAVOID_UB
  COMPARE := 0
endif

# Version-specific settings
ifeq ($(VERSION),gc-us)
  DEBUG := 0
  COMPARE := 0
  CPP_DEFINES += -DOOT_NTSC=1 -DOOT_PAL=0 -DOOT_MQ=0
else ifeq ($(VERSION),gc-eu)
  DEBUG := 0
  CPP_DEFINES += -DOOT_NTSC=0 -DOOT_PAL=1 -DOOT_MQ=0
else ifeq ($(VERSION),gc-eu-mq)
  DEBUG := 0
  CPP_DEFINES += -DOOT_NTSC=0 -DOOT_PAL=1 -DOOT_MQ=1
else ifeq ($(VERSION),gc-eu-mq-dbg)
  DEBUG := 1
  CPP_DEFINES += -DOOT_NTSC=0 -DOOT_PAL=1 -DOOT_MQ=1
else
$(error Unsupported version $(VERSION))
endif

PROJECT_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR := build/$(VERSION)
EXPECTED_DIR := expected/$(BUILD_DIR)
BASEROM_DIR := baseroms/$(VERSION)
EXTRACTED_DIR := extracted/$(VERSION)
VENV := .venv

MAKE = make
CPPFLAGS += -P -xc -fno-dollars-in-identifiers

ifeq ($(DEBUG),1)
  CPP_DEFINES += -DOOT_DEBUG=1
  OPTFLAGS := -O2
else
  CPP_DEFINES += -DNDEBUG -DOOT_DEBUG=0
  OPTFLAGS := -O2 -g3
endif

ifeq ($(OS),Windows_NT)
    DETECTED_OS=windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        DETECTED_OS=linux
    endif
    ifeq ($(UNAME_S),Darwin)
        DETECTED_OS=macos
        MAKE=gmake
    endif
endif

#### Tools ####
ifneq ($(shell type $(MIPS_BINUTILS_PREFIX)ld >/dev/null 2>/dev/null; echo $$?), 0)
  $(error Unable to find $(MIPS_BINUTILS_PREFIX)ld. Please install or build MIPS binutils, commonly mips-linux-gnu. (or set MIPS_BINUTILS_PREFIX if your MIPS binutils install uses another prefix))
endif

# Detect compiler and set variables appropriately.
ifeq ($(COMPILER),gcc)
  CC       := $(MIPS_BINUTILS_PREFIX)gcc
else ifeq ($(COMPILER),ido)
  CC       := tools/ido_recomp/$(DETECTED_OS)/7.1/cc
  CC_OLD   := tools/ido_recomp/$(DETECTED_OS)/5.3/cc
else
$(error Unsupported compiler. Please use either ido or gcc as the COMPILER variable.)
endif

# if ORIG_COMPILER is 1, check that either QEMU_IRIX is set or qemu-irix package installed
ifeq ($(ORIG_COMPILER),1)
  ifndef QEMU_IRIX
    QEMU_IRIX := $(shell which qemu-irix)
    ifeq (, $(QEMU_IRIX))
      $(error Please install qemu-irix package or set QEMU_IRIX env var to the full qemu-irix binary path)
    endif
  endif
  CC        = $(QEMU_IRIX) -L tools/ido7.1_compiler tools/ido7.1_compiler/usr/bin/cc
  CC_OLD    = $(QEMU_IRIX) -L tools/ido5.3_compiler tools/ido5.3_compiler/usr/bin/cc
endif

AS      := $(MIPS_BINUTILS_PREFIX)as
LD      := $(MIPS_BINUTILS_PREFIX)ld
OBJCOPY := $(MIPS_BINUTILS_PREFIX)objcopy
OBJDUMP := $(MIPS_BINUTILS_PREFIX)objdump
NM      := $(MIPS_BINUTILS_PREFIX)nm

N64_EMULATOR ?=

INC := -Iinclude -Iinclude/libc -Isrc -I$(BUILD_DIR) -I. -I$(EXTRACTED_DIR)

# Check code syntax with host compiler
CHECK_WARNINGS := -Wall -Wextra -Wno-format-security -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-variable -Wno-missing-braces
CHECK_WARNINGS += -Werror=implicit-int -Werror=implicit-function-declaration -Werror=int-conversion -Werror=incompatible-pointer-types

# The `cpp` command behaves differently on macOS (it behaves as if
# `-traditional-cpp` was passed) so we use `gcc -E` instead.
CPP        := gcc -E
MKLDSCRIPT := tools/mkldscript
MKDMADATA  := tools/mkdmadata
ELF2ROM    := tools/elf2rom
ZAPD       := tools/ZAPD/ZAPD.out
FADO       := tools/fado/fado.elf
PYTHON     ?= $(VENV)/bin/python3

# Command to replace $(BUILD_DIR) in some files with the build path.
# We can't use the C preprocessor for this because it won't substitute inside string literals.
BUILD_DIR_REPLACE := sed -e 's|$$(BUILD_DIR)|$(BUILD_DIR)|g'

# Audio tools
AUDIO_EXTRACT := $(PYTHON) tools/audio/extraction/audio_extract.py
SAMPLECONV    := tools/audio/sampleconv/sampleconv
SBC           := tools/audio/sbc --matching
SFC           := tools/audio/sfc --matching
ELFPATCH      := tools/audio/elfpatch
ATBLGEN       := tools/audio/atblgen
AFILE_SIZES   := tools/audio/afile_sizes

# We want linemarkers in sequence assembly files for better assembler error messages
SEQ_CPP := cpp -fno-dollars-in-identifiers
SEQ_CPPFLAGS := -D_LANGUAGE_ASEQ -DMML_VERSION=$(MML_VERSION) -I include/audio -I include/tables/sfx -I $(BUILD_DIR)/assets/audio/soundfonts

CFLAGS += $(CPP_DEFINES)
CPPFLAGS += $(CPP_DEFINES)

ifeq ($(COMPILER),gcc)
  OPTFLAGS := -Os -ffast-math -fno-unsafe-math-optimizations
endif

# TODO PL and DOWHILE should be disabled for non-gamecube
GBI_DEFINES := -DF3DEX_GBI_2 -DF3DEX_GBI_PL -DGBI_DOWHILE
ifeq ($(DEBUG),1)
  GBI_DEFINES += -DGBI_DEBUG
endif

CFLAGS += $(GBI_DEFINES)

ASFLAGS := -march=vr4300 -32 -no-pad-sections -Iinclude

ifeq ($(COMPILER),gcc)
  CFLAGS += -G 0 -nostdinc $(INC) -march=vr4300 -mfix4300 -mabi=32 -mno-abicalls -mdivide-breaks -fno-PIC -fno-common -ffreestanding -fbuiltin -fno-builtin-sinf -fno-builtin-cosf $(CHECK_WARNINGS) -funsigned-char
  MIPS_VERSION := -mips3
else
  # Suppress warnings for wrong number of macro arguments (to fake variadic
  # macros) and Microsoft extensions such as anonymous structs (which the
  # compiler does support but warns for their usage).
  CFLAGS += -G 0 -non_shared -fullwarn -verbose -Xcpluscomm $(INC) -Wab,-r4300_mul -woff 516,609,649,838,712,807
  MIPS_VERSION := -mips2
endif

ifeq ($(COMPILER),ido)
  # Have CC_CHECK pretend to be a MIPS compiler
  MIPS_BUILTIN_DEFS := -D_MIPS_ISA_MIPS2=2 -D_MIPS_ISA=_MIPS_ISA_MIPS2 -D_ABIO32=1 -D_MIPS_SIM=_ABIO32 -D_MIPS_SZINT=32 -D_MIPS_SZLONG=32 -D_MIPS_SZPTR=32
  CC_CHECK  = gcc -fno-builtin -fsyntax-only -funsigned-char -std=gnu90 -D_LANGUAGE_C $(CPP_DEFINES) $(MIPS_BUILTIN_DEFS) $(GBI_DEFINES) $(INC) $(CHECK_WARNINGS)
  ifeq ($(shell getconf LONG_BIT), 32)
    # Work around memory allocation bug in QEMU
    export QEMU_GUEST_BASE := 1
  else
    # Ensure that gcc (warning check) treats the code as 32-bit
    CC_CHECK += -m32
  endif
else
  RUN_CC_CHECK := 0
endif

OBJDUMP_FLAGS := -d -r -z -Mreg-names=32

#### Files ####

# ROM image
ROM      := $(BUILD_DIR)/oot-$(VERSION).z64
ROMC     := $(ROM:.z64=-compressed.z64)
ELF      := $(ROM:.z64=.elf)
MAP      := $(ROM:.z64=.map)
LDSCRIPT := $(ROM:.z64=.ld)
# description of ROM segments
SPEC := spec

ifeq ($(COMPILER),ido)
SRC_DIRS := $(shell find src -type d -not -path src/gcc_fix)
else
SRC_DIRS := $(shell find src -type d)
endif

ifneq ($(wildcard assets/audio/samples),)
  AIFF_DIRS := $(shell find assets/audio/samples -type d)
  SAMPLEBANK_DIRS := $(shell find assets/audio/samplebanks -type d)
  SOUNDFONT_DIRS := $(shell find assets/audio/soundfonts -type d)
  SEQUENCE_DIRS := $(shell find assets/audio/sequences -type d)
else
  AIFF_DIRS :=
  SAMPLEBANK_DIRS :=
  SOUNDFONT_DIRS :=
  SEQUENCE_DIRS :=
endif

AIFF_FILES := $(foreach dir,$(AIFF_DIRS),$(wildcard $(dir)/*.wav))
AIFC_FILES := $(foreach f,$(AIFF_FILES),$(BUILD_DIR)/$(f:.wav=.aifc))

SAMPLEBANK_XMLS := $(foreach dir,$(SAMPLEBANK_DIRS),$(wildcard $(dir)/*.xml))
SAMPLEBANK_BUILD_XMLS := $(foreach f,$(SAMPLEBANK_XMLS),$(BUILD_DIR)/$f)
SAMPLEBANK_O_FILES := $(foreach f,$(SAMPLEBANK_XMLS),$(BUILD_DIR)/$(f:.xml=.o))
SAMPLEBANK_DEP_FILES := $(foreach f,$(SAMPLEBANK_O_FILES),$(f:.o=.d))

SOUNDFONT_XMLS := $(foreach dir,$(SOUNDFONT_DIRS),$(wildcard $(dir)/*.xml))
SOUNDFONT_BUILD_XMLS := $(foreach f,$(SOUNDFONT_XMLS),$(BUILD_DIR)/$f)
SOUNDFONT_O_FILES := $(foreach f,$(SOUNDFONT_XMLS),$(BUILD_DIR)/$(f:.xml=.o))
SOUNDFONT_HEADERS := $(foreach f,$(SOUNDFONT_XMLS),$(BUILD_DIR)/$(f:.xml=.h))
SOUNDFONT_DEP_FILES := $(foreach f,$(SOUNDFONT_O_FILES),$(f:.o=.d))

SEQUENCE_FILES := $(foreach dir,$(SEQUENCE_DIRS),$(wildcard $(dir)/*.seq))
SEQUENCE_O_FILES := $(foreach f,$(SEQUENCE_FILES:.seq=.o),$(BUILD_DIR)/$f)
SEQUENCE_DEP_FILES := $(foreach f,$(SEQUENCE_O_FILES),$(f:.o=.d))

SEQUENCE_TABLE := include/tables/sequence_table.h

# create extracted directories
$(shell mkdir -p $(EXTRACTED_DIR) $(EXTRACTED_DIR)/assets $(EXTRACTED_DIR)/text)

ASSET_BIN_DIRS_EXTRACTED := $(shell find $(EXTRACTED_DIR)/assets -type d)
ASSET_BIN_DIRS_COMMITTED := $(shell find assets -type d -not -path "assets/xml*" -not -path assets/text)
ASSET_BIN_DIRS := $(ASSET_BIN_DIRS_EXTRACTED) $(ASSET_BIN_DIRS_COMMITTED)

ASSET_FILES_BIN_EXTRACTED := $(foreach dir,$(ASSET_BIN_DIRS_EXTRACTED),$(wildcard $(dir)/*.bin))
ASSET_FILES_BIN_COMMITTED := $(foreach dir,$(ASSET_BIN_DIRS_COMMITTED),$(wildcard $(dir)/*.bin))
ASSET_FILES_OUT := $(foreach f,$(ASSET_FILES_BIN_EXTRACTED:.bin=.bin.inc.c),$(f:$(EXTRACTED_DIR)/%=$(BUILD_DIR)/%)) \
                   $(foreach f,$(ASSET_FILES_BIN_COMMITTED:.bin=.bin.inc.c),$(BUILD_DIR)/$f) \
                   $(foreach f,$(wildcard assets/text/*.c),$(BUILD_DIR)/$(f:.c=.o))

UNDECOMPILED_DATA_DIRS := $(shell find data -type d)

BASEROM_BIN_FILES := $(wildcard $(EXTRACTED_DIR)/baserom/*)

# source files
ASSET_C_FILES_EXTRACTED := $(filter-out %.inc.c,$(foreach dir,$(ASSET_BIN_DIRS_EXTRACTED),$(wildcard $(dir)/*.c)))
ASSET_C_FILES_COMMITTED := $(filter-out %.inc.c,$(foreach dir,$(ASSET_BIN_DIRS_COMMITTED),$(wildcard $(dir)/*.c)))
SRC_C_FILES   := $(filter-out %.inc.c,$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)))
S_FILES       := $(foreach dir,$(SRC_DIRS) $(UNDECOMPILED_DATA_DIRS),$(wildcard $(dir)/*.s))
O_FILES       := $(foreach f,$(S_FILES:.s=.o),$(BUILD_DIR)/$f) \
                 $(foreach f,$(SRC_C_FILES:.c=.o),$(BUILD_DIR)/$f) \
                 $(foreach f,$(ASSET_C_FILES_EXTRACTED:.c=.o),$(f:$(EXTRACTED_DIR)/%=$(BUILD_DIR)/%)) \
                 $(foreach f,$(ASSET_C_FILES_COMMITTED:.c=.o),$(BUILD_DIR)/$f) \
                 $(foreach f,$(BASEROM_BIN_FILES),$(BUILD_DIR)/baserom/$(notdir $f).o)

OVL_RELOC_FILES := $(shell $(CPP) $(CPPFLAGS) $(SPEC) | $(BUILD_DIR_REPLACE) | grep -o '[^"]*_reloc.o' )

# Automatic dependency files
# (Only asm_processor dependencies and reloc dependencies are handled for now)
DEP_FILES := $(O_FILES:.o=.asmproc.d) $(OVL_RELOC_FILES:.o=.d)


TEXTURE_FILES_PNG_EXTRACTED := $(foreach dir,$(ASSET_BIN_DIRS_EXTRACTED),$(wildcard $(dir)/*.png))
TEXTURE_FILES_PNG_COMMITTED := $(foreach dir,$(ASSET_BIN_DIRS_COMMITTED),$(wildcard $(dir)/*.png))
TEXTURE_FILES_JPG_EXTRACTED := $(foreach dir,$(ASSET_BIN_DIRS_EXTRACTED),$(wildcard $(dir)/*.jpg))
TEXTURE_FILES_JPG_COMMITTED := $(foreach dir,$(ASSET_BIN_DIRS_COMMITTED),$(wildcard $(dir)/*.jpg))
TEXTURE_FILES_OUT := $(foreach f,$(TEXTURE_FILES_PNG_EXTRACTED:.png=.inc.c),$(f:$(EXTRACTED_DIR)/%=$(BUILD_DIR)/%)) \
                     $(foreach f,$(TEXTURE_FILES_PNG_COMMITTED:.png=.inc.c),$(BUILD_DIR)/$f) \
                     $(foreach f,$(TEXTURE_FILES_JPG_EXTRACTED:.jpg=.jpg.inc.c),$(f:$(EXTRACTED_DIR)/%=$(BUILD_DIR)/%)) \
                     $(foreach f,$(TEXTURE_FILES_JPG_COMMITTED:.jpg=.jpg.inc.c),$(BUILD_DIR)/$f)

# create build directories
$(shell mkdir -p $(BUILD_DIR)/baserom $(BUILD_DIR)/include/tables $(BUILD_DIR)/assets/text $(foreach dir,$(SRC_DIRS) $(UNDECOMPILED_DATA_DIRS) $(AIFF_DIRS) $(SAMPLEBANK_DIRS) $(SOUNDFONT_DIRS) $(SEQUENCE_DIRS),$(BUILD_DIR)/$(dir)) $(foreach dir,$(ASSET_BIN_DIRS),$(dir:$(EXTRACTED_DIR)/%=$(BUILD_DIR)/%)))

ifeq ($(COMPILER),ido)
$(BUILD_DIR)/src/boot/stackcheck.o: OPTFLAGS := -O2

$(BUILD_DIR)/src/code/__osMalloc.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/code_800FC620.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/fp_math.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/rand.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/gfxprint.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/jpegutils.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/jpegdecoder.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/load.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/loadfragment2.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/logutils.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/mtxuty-cvt.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/padsetup.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/padutils.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/printutils.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/relocation.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/sleep.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/system_malloc.o: OPTFLAGS := -O2

$(BUILD_DIR)/src/code/fault.o: CFLAGS += -trapuv
$(BUILD_DIR)/src/code/fault.o: OPTFLAGS := -O2 -g3
$(BUILD_DIR)/src/code/fault_drawer.o: CFLAGS += -trapuv
$(BUILD_DIR)/src/code/fault_drawer.o: OPTFLAGS := -O2 -g3
$(BUILD_DIR)/src/code/ucode_disas.o: OPTFLAGS := -O2 -g3

ifeq ($(DEBUG),1)
$(BUILD_DIR)/src/code/fmodf.o: OPTFLAGS := -g
$(BUILD_DIR)/src/code/__osMemset.o: OPTFLAGS := -g
$(BUILD_DIR)/src/code/__osMemmove.o: OPTFLAGS := -g
else
$(BUILD_DIR)/src/code/fmodf.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/__osMemset.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/code/__osMemmove.o: OPTFLAGS := -O2
endif

$(BUILD_DIR)/src/audio/%.o: OPTFLAGS := -O2

# Use signed chars instead of unsigned for this audio file (needed to match AudioDebug_ScrPrt)
$(BUILD_DIR)/src/audio/general.o: CFLAGS += -signed

# Put string literals in .data for some audio files (needed to match these files with literals)
$(BUILD_DIR)/src/audio/sfx.o: CFLAGS += -use_readwrite_const
$(BUILD_DIR)/src/audio/sequence.o: CFLAGS += -use_readwrite_const

ifeq ($(DEBUG),1)
$(BUILD_DIR)/src/libultra/libc/absf.o: OPTFLAGS := -O2 -g3
$(BUILD_DIR)/src/libultra/libc/sqrt.o: OPTFLAGS := -O2 -g3
else
$(BUILD_DIR)/src/libultra/libc/absf.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/libultra/libc/sqrt.o: OPTFLAGS := -O2
endif

$(BUILD_DIR)/src/libultra/libc/ll.o: OPTFLAGS := -O1
$(BUILD_DIR)/src/libultra/libc/ll.o: MIPS_VERSION := -mips3 -32
$(BUILD_DIR)/src/libultra/libc/llcvt.o: OPTFLAGS := -O1
$(BUILD_DIR)/src/libultra/libc/llcvt.o: MIPS_VERSION := -mips3 -32

$(BUILD_DIR)/src/libultra/os/%.o: OPTFLAGS := -O1
$(BUILD_DIR)/src/libultra/io/%.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/libultra/libc/%.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/libultra/rmon/%.o: OPTFLAGS := -O2
$(BUILD_DIR)/src/libultra/gu/%.o: OPTFLAGS := -O2

$(BUILD_DIR)/assets/misc/z_select_static/%.o: GBI_DEFINES := -DF3DEX_GBI

$(BUILD_DIR)/src/libultra/gu/%.o: CC := $(CC_OLD)
$(BUILD_DIR)/src/libultra/io/%.o: CC := $(CC_OLD)
$(BUILD_DIR)/src/libultra/libc/%.o: CC := $(CC_OLD)
$(BUILD_DIR)/src/libultra/os/%.o: CC := $(CC_OLD)
$(BUILD_DIR)/src/libultra/rmon/%.o: CC := $(CC_OLD)

$(BUILD_DIR)/src/code/jpegutils.o: CC := $(CC_OLD)
$(BUILD_DIR)/src/code/jpegdecoder.o: CC := $(CC_OLD)

# For using asm_processor on some files:
#$(BUILD_DIR)/.../%.o: CC := $(PYTHON) tools/asm_processor/build.py $(CC) -- $(AS) $(ASFLAGS) --

ifeq ($(PERMUTER),)  # permuter + preprocess.py misbehaves, permuter doesn't care about rodata diffs or bss ordering so just don't use it in that case
# Handle encoding (UTF-8 -> EUC-JP) and custom pragmas
$(BUILD_DIR)/src/%.o: CC := $(PYTHON) tools/preprocess.py $(CC)
endif

else
# Note that if adding additional assets directories for modding reasons these flags must also be used there
$(BUILD_DIR)/assets/%.o: CFLAGS += -fno-zero-initialized-in-bss -fno-toplevel-reorder
$(BUILD_DIR)/src/%.o: CFLAGS += -fexec-charset=euc-jp
$(BUILD_DIR)/src/libultra/libc/ll.o: OPTFLAGS := -Ofast
$(BUILD_DIR)/src/overlays/%.o: CFLAGS += -fno-merge-constants -mno-explicit-relocs -mno-split-addresses
endif

#### Main Targets ###

all: rom compress

rom: $(ROM)
ifneq ($(COMPARE),0)
	@md5sum $(ROM)
	@md5sum -c $(BASEROM_DIR)/checksum.md5
endif

compress: $(ROMC)
ifneq ($(COMPARE),0)
	@md5sum $(ROMC)
	@md5sum -c $(BASEROM_DIR)/checksum-compressed.md5
endif

clean:
	$(RM) -r $(BUILD_DIR)

assetclean:
	$(RM) -r $(EXTRACTED_DIR)
	$(RM) -r assets/audio/samplebanks
	$(RM) -r assets/audio/samples
	$(RM) -r assets/audio/soundfonts
	find assets/audio/sequences -type f -name "*.seq" -not -name "*.prg.seq" -delete

distclean:
	$(RM) -r extracted/
	$(RM) -r build/
	$(MAKE) -C tools distclean

venv:
# Create the virtual environment if it doesn't exist.
# Delete the virtual environment directory if creation fails.
	test -d $(VENV) || python3 -m venv $(VENV) || { rm -rf $(VENV); false; }
	$(PYTHON) -m pip install -U pip
	$(PYTHON) -m pip install -U -r requirements.txt

setup-audio:
	$(AUDIO_EXTRACT) -r $(BASEROM_DIR)/baserom-decompressed.z64 -v oot_mqdbg --read-xml

setup: venv
	$(MAKE) -C tools
	$(PYTHON) tools/decompress_baserom.py $(VERSION)
	$(PYTHON) tools/extract_baserom.py $(BASEROM_DIR)/baserom-decompressed.z64 --oot-version $(VERSION) -o $(EXTRACTED_DIR)/baserom
	$(PYTHON) tools/msgdis.py $(VERSION)
	$(PYTHON) extract_assets.py -v $(VERSION) -j$(N_THREADS)
	$(MAKE) setup-audio

disasm:
	$(RM) -r $(EXPECTED_DIR)
	VERSION=$(VERSION) DISASM_BASEROM=$(BASEROM_DIR)/baserom-decompressed.z64 DISASM_DIR=$(EXPECTED_DIR) PYTHON=$(PYTHON) AS_CMD='$(AS) $(ASFLAGS)' LD=$(LD) ./tools/disasm/do_disasm.sh

run: $(ROM)
ifeq ($(N64_EMULATOR),)
	$(error Emulator path not set. Set N64_EMULATOR in the Makefile or define it as an environment variable)
endif
	$(N64_EMULATOR) $<


.PHONY: all rom compress clean assetclean distclean venv setup disasm run
.DEFAULT_GOAL := rom

#### Various Recipes ####

$(ROM): $(ELF)
	$(ELF2ROM) -cic 6105 $< $@

$(ROMC): $(ROM) $(ELF) $(BUILD_DIR)/compress_ranges.txt
	$(PYTHON) tools/compress.py --in $(ROM) --out $@ --dmadata-start `./tools/dmadata_start.sh $(NM) $(ELF)` --compress `cat $(BUILD_DIR)/compress_ranges.txt` --threads $(N_THREADS)
	$(PYTHON) -m ipl3checksum sum --cic 6105 --update $@

$(ELF): $(TEXTURE_FILES_OUT) $(ASSET_FILES_OUT) $(O_FILES) $(OVL_RELOC_FILES) $(LDSCRIPT) $(BUILD_DIR)/undefined_syms.txt \
        $(AIFC_FILES) \
        $(SAMPLEBANK_O_FILES) \
        $(SOUNDFONT_O_FILES) \
        $(SEQUENCE_O_FILES) \
        $(BUILD_DIR)/src/audio/tables/samplebank_table.o \
        $(BUILD_DIR)/src/audio/tables/soundfont_table.o \
        $(BUILD_DIR)/src/audio/tables/sequence_table.o \
        $(BUILD_DIR)/assets/audio/sequence_font_table.o \
        $(BUILD_DIR)/assets/audio/sequence_sizes.h \
        $(BUILD_DIR)/assets/audio/soundfont_sizes.h \
        $(BUILD_DIR)/assets/audio/audiobank_padding.o
	$(LD) -T $(LDSCRIPT) -T $(BUILD_DIR)/undefined_syms.txt --no-check-sections --accept-unknown-input-arch --emit-relocs -Map $(MAP) -o $@

## Order-only prerequisites
# These ensure e.g. the O_FILES are built before the OVL_RELOC_FILES.
# The intermediate phony targets avoid quadratically-many dependencies between the targets and prerequisites.

o_files: $(O_FILES)
$(OVL_RELOC_FILES): | o_files

asset_files: $(TEXTURE_FILES_OUT) $(ASSET_FILES_OUT)
$(O_FILES): | asset_files

.PHONY: o_files asset_files

$(BUILD_DIR)/$(SPEC): $(SPEC)
	$(CPP) $(CPPFLAGS) $< | $(BUILD_DIR_REPLACE) > $@

$(LDSCRIPT): $(BUILD_DIR)/$(SPEC)
	$(MKLDSCRIPT) $< $@

$(BUILD_DIR)/undefined_syms.txt: undefined_syms.txt
	$(CPP) $(CPPFLAGS) $< > $@

$(BUILD_DIR)/baserom/%.o: $(EXTRACTED_DIR)/baserom/%
	$(OBJCOPY) -I binary -O elf32-big $< $@

$(BUILD_DIR)/data/%.o: data/%.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/assets/text/%.enc.jpn.h: assets/text/%.h $(EXTRACTED_DIR)/text/%.h assets/text/charmap.txt
	$(CPP) $(CPPFLAGS) -I$(EXTRACTED_DIR) $< | $(PYTHON) tools/msgenc.py --encoding jpn --charmap assets/text/charmap.txt - $@

$(BUILD_DIR)/assets/text/%.enc.nes.h: assets/text/%.h $(EXTRACTED_DIR)/text/%.h assets/text/charmap.txt
	$(CPP) $(CPPFLAGS) -I$(EXTRACTED_DIR) $< | $(PYTHON) tools/msgenc.py --encoding nes --charmap assets/text/charmap.txt - $@

# Dependencies for files including message data headers
# TODO remove when full header dependencies are used.
$(BUILD_DIR)/assets/text/jpn_message_data_static.o: $(BUILD_DIR)/assets/text/message_data.enc.jpn.h
$(BUILD_DIR)/assets/text/nes_message_data_static.o: $(BUILD_DIR)/assets/text/message_data.enc.nes.h
$(BUILD_DIR)/assets/text/ger_message_data_static.o: $(BUILD_DIR)/assets/text/message_data.enc.nes.h
$(BUILD_DIR)/assets/text/fra_message_data_static.o: $(BUILD_DIR)/assets/text/message_data.enc.nes.h
$(BUILD_DIR)/assets/text/staff_message_data_static.o: $(BUILD_DIR)/assets/text/message_data_staff.enc.nes.h
$(BUILD_DIR)/src/code/z_message.o: assets/text/message_data.h assets/text/message_data_staff.h

$(BUILD_DIR)/assets/text/%.o: assets/text/%.c
ifneq ($(COMPILER),gcc)
# Preprocess text with modern cpp for varargs macros
	$(CPP) -undef -D_LANGUAGE_C -D__sgi $(CPPFLAGS) $(INC) $< -o $(@:.o=.c)
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $@ $(@:.o=.c)
else
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $@ $<
endif
	$(OBJCOPY) -O binary -j.rodata $@ $@.bin

$(BUILD_DIR)/assets/%.o: assets/%.c
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $@ $<
	$(OBJCOPY) -O binary $@ $@.bin

$(BUILD_DIR)/assets/%.o: $(EXTRACTED_DIR)/assets/%.c
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $@ $<
	$(OBJCOPY) -O binary $@ $@.bin

$(BUILD_DIR)/src/%.o: src/%.s
	$(CPP) $(CPPFLAGS) -Iinclude $< | $(AS) $(ASFLAGS) -o $@

$(BUILD_DIR)/dmadata_table_spec.h $(BUILD_DIR)/compress_ranges.txt: $(BUILD_DIR)/$(SPEC)
	$(MKDMADATA) $< $(BUILD_DIR)/dmadata_table_spec.h $(BUILD_DIR)/compress_ranges.txt

# Dependencies for files that may include the dmadata header automatically generated from the spec file
$(BUILD_DIR)/src/boot/z_std_dma.o: $(BUILD_DIR)/dmadata_table_spec.h
$(BUILD_DIR)/src/dmadata/dmadata.o: $(BUILD_DIR)/dmadata_table_spec.h

# Dependencies for files including from include/tables/
# TODO remove when full header dependencies are used.
$(BUILD_DIR)/src/code/graph.o: include/tables/gamestate_table.h
$(BUILD_DIR)/src/code/object_table.o: include/tables/object_table.h
$(BUILD_DIR)/src/code/z_actor.o: include/tables/actor_table.h # so uses of ACTOR_ID_MAX update when the table length changes
$(BUILD_DIR)/src/code/z_actor_dlftbls.o: include/tables/actor_table.h
$(BUILD_DIR)/src/code/z_effect_soft_sprite_dlftbls.o: include/tables/effect_ss_table.h
$(BUILD_DIR)/src/code/z_game_dlftbls.o: include/tables/gamestate_table.h
$(BUILD_DIR)/src/code/z_scene_table.o: include/tables/scene_table.h include/tables/entrance_table.h
$(BUILD_DIR)/src/audio/general.o: $(SEQUENCE_TABLE)
$(BUILD_DIR)/src/audio/sfx_params.o: include/tables/sfx/*.h

$(BUILD_DIR)/src/%.o: src/%.c
ifneq ($(RUN_CC_CHECK),0)
	$(CC_CHECK) $<
endif
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $@ $<
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $(@:.o=.s)

$(BUILD_DIR)/src/libultra/libc/ll.o: src/libultra/libc/ll.c
ifneq ($(RUN_CC_CHECK),0)
	$(CC_CHECK) $<
endif
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $@ $<
	$(PYTHON) tools/set_o32abi_bit.py $@
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $(@:.o=.s)

$(BUILD_DIR)/src/libultra/libc/llcvt.o: src/libultra/libc/llcvt.c
ifneq ($(RUN_CC_CHECK),0)
	$(CC_CHECK) $<
endif
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $@ $<
	$(PYTHON) tools/set_o32abi_bit.py $@
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $(@:.o=.s)

$(BUILD_DIR)/src/overlays/%_reloc.o: $(BUILD_DIR)/$(SPEC)
	$(FADO) $$(tools/reloc_prereq $< $(notdir $*)) -n $(notdir $*) -o $(@:.o=.s) -M $(@:.o=.d)
	$(AS) $(ASFLAGS) $(@:.o=.s) -o $@

$(BUILD_DIR)/assets/%.inc.c: assets/%.png
	$(ZAPD) btex -eh -tt $(subst .,,$(suffix $*)) -i $< -o $@

$(BUILD_DIR)/assets/%.inc.c: $(EXTRACTED_DIR)/assets/%.png
	$(ZAPD) btex -eh -tt $(subst .,,$(suffix $*)) -i $< -o $@

$(BUILD_DIR)/assets/%.bin.inc.c: assets/%.bin
	$(ZAPD) bblb -eh -i $< -o $@

$(BUILD_DIR)/assets/%.bin.inc.c: $(EXTRACTED_DIR)/assets/%.bin
	$(ZAPD) bblb -eh -i $< -o $@

$(BUILD_DIR)/assets/%.jpg.inc.c: assets/%.jpg
	$(ZAPD) bren -eh -i $< -o $@

$(BUILD_DIR)/assets/%.jpg.inc.c: $(EXTRACTED_DIR)/assets/%.jpg
	$(ZAPD) bren -eh -i $< -o $@

# Audio

# first build samples...

$(BUILD_DIR)/assets/audio/samples/%.half.aifc: assets/audio/samples/%.half.wav
	$(SAMPLECONV) vadpcm-half $< $@
# TESTING:
#	@(cmp $(<D)/aifc/$(<F:.half.wav=.half.aifc) $@ && echo "$(<F) OK") || (mkdir -p NONMATCHINGS/$(<D) && cp $(<D)/aifc/$(<F:.half.wav=.half.aifc) NONMATCHINGS/$(<D)/$(<F:.half.wav=.half.aifc))

$(BUILD_DIR)/assets/audio/samples/%.aifc: assets/audio/samples/%.wav
	$(SAMPLECONV) vadpcm $< $@
# TESTING:
#	@(cmp $(<D)/aifc/$(<F:.wav=.aifc) $@ && echo "$(<F) OK") || (mkdir -p NONMATCHINGS/$(<D) && cp $(<D)/aifc/$(<F:.wav=.aifc) NONMATCHINGS/$(<D)/$(<F:.wav=.aifc))

# then assemble the samplebanks...

$(BUILD_DIR)/assets/audio/samplebanks/%.xml: assets/audio/samplebanks/%.xml
	cat $< | $(BUILD_DIR_REPLACE) > $@

.PRECIOUS: $(BUILD_DIR)/assets/audio/samplebanks/%.s
$(BUILD_DIR)/assets/audio/samplebanks/%.s: $(BUILD_DIR)/assets/audio/samplebanks/%.xml | $(AIFC_FILES)
	$(SBC) --makedepend $(@:.s=.d) $< $@

-include $(SAMPLEBANK_DEP_FILES)

$(BUILD_DIR)/assets/audio/samplebanks/%.o: $(BUILD_DIR)/assets/audio/samplebanks/%.s
	$(AS) $(ASFLAGS) $< -o $@
# TESTING:
#	$(OBJCOPY) -O binary -j.rodata $@ $(@:.o=.bin)
#	@cmp $(@:.o=.bin) $(patsubst $(BUILD_DIR)/assets/audio/samplebanks/%,$(EXTRACTED_DIR)/baserom/audiotable_files/%,$(@:.o=.bin)) && echo "$(<F) OK"

# also assemble the soundfonts and generate the associated headers...

$(BUILD_DIR)/assets/audio/soundfonts/%.xml: assets/audio/soundfonts/%.xml
	cat $< | $(BUILD_DIR_REPLACE) > $@

.PRECIOUS: $(BUILD_DIR)/assets/audio/soundfonts/%.c $(BUILD_DIR)/assets/audio/soundfonts/%.h $(BUILD_DIR)/assets/audio/soundfonts/%.name
$(BUILD_DIR)/assets/audio/soundfonts/%.c $(BUILD_DIR)/assets/audio/soundfonts/%.h $(BUILD_DIR)/assets/audio/soundfonts/%.name: $(BUILD_DIR)/assets/audio/soundfonts/%.xml | $(SAMPLEBANK_BUILD_XMLS) $(AIFC_FILES)
# This rule can be triggered for either the .c or .h file, so $@ may refer to either the .c or .h file. A simple
# substitution $(@:.c=.h) will fail ~50% of the time with -j. Instead, don't assume anything about the suffix of $@.
	$(SFC) --makedepend $(basename $@).d $< $(basename $@).c $(basename $@).h $(basename $@).name

-include $(SOUNDFONT_DEP_FILES)

$(BUILD_DIR)/assets/audio/soundfonts/%.o: $(BUILD_DIR)/assets/audio/soundfonts/%.c $(BUILD_DIR)/assets/audio/soundfonts/%.name #$(SAMPLEBANK_O_FILES) (for debugging only)
# compile c to unlinked object
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -I include/audio -o $(@:.o=.tmp) $<
# partial link
	@$(LD) -r -T include/audio/sf.ld $(@:.o=.tmp) -o $(@:.o=.tmp2)
# patch defined symbols to be ABS symbols so that they remain file-relative offsets forever
	@$(ELFPATCH) $(@:.o=.tmp2) $(@:.o=.tmp2)
# write start and size symbols afterwards, filename != symbolic name so source symbolic name from the .name file written by sfc
	@$(OBJCOPY) --add-symbol $$(cat $(<:.c=.name))_Start=.rodata:0,global --redefine-sym __LEN__=$$(cat $(<:.c=.name))_Size $(@:.o=.tmp2) $@
# cleanup temp files
	@$(RM) $(@:.o=.tmp) $(@:.o=.tmp2)
# TESTING: link with samplebanks and dump binary
#	$(LD) $(foreach f,$(SAMPLEBANK_O_FILES),-R $f) -T include/audio/sf.ld $@ -o $(@:.o=.elf)
#	$(OBJCOPY) -O binary -j.rodata $(@:.o=.elf) $(@:.o=.bin)
#	@(cmp $(@:.o=.bin) $(patsubst $(BUILD_DIR)/assets/audio/soundfonts/%,$(EXTRACTED_DIR)/baserom/audiobank_files/%,$(@:.o=.bin)) && echo "$(<F) OK" || (mkdir -p NONMATCHINGS/soundfonts && cp $(@:.o=.bin) NONMATCHINGS/soundfonts/$(@F:.o=.bin)))

# then assemble the sequences...

$(BUILD_DIR)/assets/audio/sequences/%.o: assets/audio/sequences/%.seq include/audio/aseq.h | include/audio/sequence_ids.h $(SEQUENCE_TABLE) $(SOUNDFONT_HEADERS)
	$(SEQ_CPP) $(SEQ_CPPFLAGS) $< -o $(@:.o=.S) -MMD -MT $@
	$(AS) $(ASFLAGS) -I $(BUILD_DIR)/assets/audio/soundfonts -I include/audio $(@:.o=.S) -o $@
# TESTING:
#	$(OBJCOPY) -O binary -j.data $@ $(@:.o=.aseq)
#	@(cmp $(@:.o=.aseq) $(patsubst $(BUILD_DIR)/assets/audio/sequences/%,$(EXTRACTED_DIR)/baserom/audioseq_files/%,$(@:.o=.aseq)) && echo "$(<F) OK" || (mkdir -p NONMATCHINGS/sequences && cp $(@:.o=.aseq) NONMATCHINGS/sequences/$(@F:.o=.aseq)))

-include $(SEQUENCE_DEP_FILES)

# put together the tables

$(BUILD_DIR)/assets/audio/samplebank_table.h: $(SAMPLEBANK_BUILD_XMLS)
	$(ATBLGEN) -banks $@ $^

$(BUILD_DIR)/assets/audio/soundfont_table.h: $(SOUNDFONT_BUILD_XMLS)
	$(ATBLGEN) -fonts $@ $^

SEQ_ORDER_DEFS := -DDEFINE_SEQUENCE_PTR\(name,seqId,_2,_3,_4,_5\)=*\(name,seqId\) \
                  -DDEFINE_SEQUENCE\(name,seqId,_2,_3,_4,_5\)=\(name,seqId\)
$(BUILD_DIR)/include/tables/sequence_order.in: $(SEQUENCE_TABLE)
	$(CPP) $(CPPFLAGS) $< $(SEQ_ORDER_DEFS) -o $@

$(BUILD_DIR)/assets/audio/sequence_font_table.s: $(BUILD_DIR)/include/tables/sequence_order.in $(SEQUENCE_O_FILES)
	$(ATBLGEN) -sequences $@ $^

# build the tables into objects, move data -> rodata

$(BUILD_DIR)/src/audio/tables/samplebank_table.o: src/audio/tables/samplebank_table.c $(BUILD_DIR)/assets/audio/samplebank_table.h
ifneq ($(RUN_CC_CHECK),0)
	$(CC_CHECK) $<
endif
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $(@:.o=.tmp) $<
	@$(LD) -r -T include/audio/atbl_rdata.ld $(@:.o=.tmp) -o $@
	@$(RM) $(@:.o=.tmp)
# TESTING:
#	$(OBJCOPY) -O binary -j.rodata $@ $(@:.o=.bin)
# Can't compare this due to pointers

$(BUILD_DIR)/src/audio/tables/soundfont_table.o: src/audio/tables/soundfont_table.c $(BUILD_DIR)/assets/audio/soundfont_table.h $(SOUNDFONT_HEADERS)
ifneq ($(RUN_CC_CHECK),0)
	$(CC_CHECK) $<
endif
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -o $(@:.o=.tmp) $<
	@$(LD) -r -T include/audio/atbl_rdata.ld $(@:.o=.tmp) -o $@
	@$(RM) $(@:.o=.tmp)
# TESTING:
#	$(OBJCOPY) -O binary -j.rodata $@ $(@:.o=.bin)
# Can't compare this due to pointers

$(BUILD_DIR)/src/audio/tables/sequence_table.o: src/audio/tables/sequence_table.c $(SEQUENCE_TABLE)
ifneq ($(RUN_CC_CHECK),0)
	$(CC_CHECK) $<
endif
	$(CC) -c $(CFLAGS) $(MIPS_VERSION) $(OPTFLAGS) -I include/tables -o $(@:.o=.tmp) $<
	@$(LD) -r -T include/audio/atbl_rdata.ld $(@:.o=.tmp) -o $@
	@$(RM) $(@:.o=.tmp)
# TESTING:
#	$(OBJCOPY) -O binary -j.rodata $@ $(@:.o=.bin)
# Can't compare this due to pointers

$(BUILD_DIR)/assets/audio/sequence_font_table.o: $(BUILD_DIR)/assets/audio/sequence_font_table.s
	$(AS) $(ASFLAGS) $< -o $@
# TESTING:
#	$(OBJCOPY) -O binary -j.rodata $@ $(@:.o=.bin)
#	@cmp $(@:.o=.bin) $(EXTRACTED_DIR)/baserom/audio_code_tables/sequence_font_table.bin && echo "$(@F:.o=) OK"

# make headers with file sizes and amounts

$(BUILD_DIR)/src/audio/session_config.o: $(BUILD_DIR)/assets/audio/soundfont_sizes.h $(BUILD_DIR)/assets/audio/sequence_sizes.h

$(BUILD_DIR)/assets/audio/soundfont_sizes.h: $(SOUNDFONT_O_FILES)
	$(AFILE_SIZES) $@ NUM_SOUNDFONTS SOUNDFONT_SIZES $^

$(BUILD_DIR)/assets/audio/sequence_sizes.h: $(SEQUENCE_O_FILES)
	$(AFILE_SIZES) $@ NUM_SEQUENCES SEQUENCE_SIZES $^

$(BUILD_DIR)/assets/audio/audiobank_padding.o:
	dd if=/dev/zero of=$(@:.o=.bin) bs=32 count=1 status=none
	$(OBJCOPY) -I binary -O elf32-big --rename-section .data=.rodata,alloc,contents,load,readonly $(@:.o=.bin) $@

-include $(DEP_FILES)

# Print target for debugging
print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
