PROGRAMS := elf2rom makeromfs mkdmadata mkldscript reloc_prereq vtxdis

WARNFLAGS := -Wall -Wextra -pedantic
OPTFLAGS := -O2 -g
LDFLAGS :=

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
  LDFLAGS += -fuse-ld=lld
endif

.PHONY: all clean distclean

all: $(PROGRAMS) armips
	$(MAKE) -C ZAPD
	$(MAKE) -C fado

clean:
	$(RM) $(PROGRAMS) $(addsuffix .exe,$(PROGRAMS))
	$(MAKE) -C ZAPD clean
	$(MAKE) -C fado clean

distclean: clean
	$(RM) armips

armips: armips.cpp
	$(CXX) -I. $(WARNFLAGS) -std=c++17 $(OPTFLAGS) -s -fno-rtti -pipe -Wno-unused-parameter $^ -pthread -o $@

armips_SOURCES       := armips.cpp
elf2rom_SOURCES      := elf2rom.c elf32.c n64chksum.c util.c
makeromfs_SOURCES    := makeromfs.c n64chksum.c util.c
mkdmadata_SOURCES    := mkdmadata.c spec.c util.c
mkldscript_SOURCES   := mkldscript.c spec.c util.c
reloc_prereq_SOURCES := reloc_prereq.c spec.c util.c
vtxdis_SOURCES       := vtxdis.c

define COMPILE_C
$(1): $($1_SOURCES)
	$(CC) -I. $(WARNFLAGS) -std=c99 $(OPTFLAGS) $$^ $(LDFLAGS) -o $$@
endef

$(foreach p,$(PROGRAMS),$(eval $(call COMPILE_C,$(p))))
