PROGRAMS := armips elf2rom makeromfs mkdmadata mkldscript reloc_prereq vtxdis

WARNFLAGS := -Wall -Wextra -pedantic
STDFLAGS := -std=c99
OPTFLAGS := -O2 -g
CFLAGS := -I. $(WARNFLAGS) $(STDFLAGS) $(OPTFLAGS)

ifeq ($(shell command -v clang >/dev/null 2>&1; echo $$?),0)
  CC := clang
  CXX := clang++
else
  CC := gcc
  CXX := g++
endif

ifeq ($(shell command -v ld.lld >/dev/null 2>&1; echo $$?),0)
  LLD := 1
else
  LLD := 0
endif
ifneq ($(LLD),0)
  CFLAGS += -fuse-ld=lld
endif

.PHONY: all clean distclean

all: $(PROGRAMS)
	$(MAKE) -C ZAPD
	$(MAKE) -C fado

clean:
	$(RM) $(PROGRAMS) $(addsuffix .exe,$(PROGRAMS))
	$(MAKE) -C ZAPD clean
	$(MAKE) -C fado clean

distclean: clean

armips_SOURCES       := armips.cpp
elf2rom_SOURCES      := elf2rom.c elf32.c n64chksum.c util.c
makeromfs_SOURCES    := makeromfs.c n64chksum.c util.c
mkdmadata_SOURCES    := mkdmadata.c spec.c util.c
mkldscript_SOURCES   := mkldscript.c spec.c util.c
reloc_prereq_SOURCES := reloc_prereq.c spec.c util.c
vtxdis_SOURCES       := vtxdis.c

armips: CC := $(CXX)
armips: STDFLAGS := -std=c++17
armips_CFLAGS := -s -fno-rtti -pipe -Wno-unused-parameter
armips_LDFLAGS := -pthread

define COMPILE
$(1): $($1_SOURCES)
	$$(CC) $(CFLAGS) $($1_CFLAGS) $$^ -o $$@ $($1_LDFLAGS)
endef

$(foreach p,$(PROGRAMS),$(eval $(call COMPILE,$(p))))
