PROGRAMS := afile_sizes atblgen elfpatch sbc sfc

ifeq ($(shell which xml2-config),)
  $(error Install libxml2-dev)
endif

CLANG_FORMAT := clang-format-14
FORMAT_ARGS := -i -style=file

CC := gcc
CFLAGS := -Wall -Wextra -pedantic
OPTFLAGS := -Og -g3

XML_CFLAGS := $(shell xml2-config --cflags)
XML_LDFLAGS := $(shell xml2-config --libs)

.PHONY: all clean distclean format

all: $(PROGRAMS)
	$(MAKE) -C sampleconv

clean:
	$(RM) $(PROGRAMS)
	$(MAKE) -C sampleconv clean

distclean: clean
	$(MAKE) -C sampleconv distclean

format:
	$(CLANG_FORMAT) $(FORMAT_ARGS) $(shell find . -maxdepth 1 -type f -name "*.[ch]")
	$(MAKE) -C sampleconv format

afile_sizes: afile_sizes.c util.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@

atblgen: audio_tablegen.c samplebank.c soundfont.c xml.c util.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(XML_CFLAGS) $^ $(XML_LDFLAGS) -o $@

elfpatch: elfpatch.c util.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $^ -o $@

sbc: samplebank_compiler.c samplebank.c aifc.c xml.c util.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(XML_CFLAGS) $^ $(XML_LDFLAGS) -o $@

sfc: soundfont_compiler.c samplebank.c soundfont.c aifc.c xml.c util.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(XML_CFLAGS) $^ $(XML_LDFLAGS) -o $@
