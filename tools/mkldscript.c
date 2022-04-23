#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spec.h"
#include "util.h"

// Note: *SECTION ALIGNMENT* Object files built with a compiler such as GCC can, by default, use narrower 
// alignment for sections size, compared to IDO padding sections to a 0x10-aligned size.
// To properly generate relocations relative to section starts, sections currently need to be aligned 
// explicitly (to 0x10 currently, a narrower alignment might work), otherwise the linker does implicit alignment 
// and inserts padding between the address indicated by section start symbols (such as *SegmentRoDataStart) and 
// the actual aligned start of the section.
// With IDO, the padding of sections to an aligned size makes the section start at aligned addresses out of the box, 
// so the explicit alignment has no further effect.

struct Segment *g_segments;
int g_segmentsCount;

static void write_epilogue(FILE *fout)
{
    fputs(
        // Debugging sections

		   "    .reginfo          : { *(.reginfo) }"                                    "\n"
		   "    .pdr              : { *(.pdr) }"                                        "\n"
		   "\n"
		   "    .comment              0 : { *(.comment) }"                              "\n"
		   "    .gnu.build.attributes 0 : { *(.gnu.build.attributes) }"                 "\n"
		   "\n"
        // DWARF debug sections
        // Symbols in the DWARF debugging sections are relative to the beginning of the section so we begin them at 0.
        // DWARF 1
		   "    .debug          0 : { *(.debug) }"                                      "\n"
		   "    .line           0 : { *(.line) }"                                       "\n"
		   "\n"
        // GNU DWARF 1 extensions
		   "    .debug_srcinfo  0 : { *(.debug_srcinfo) }"                              "\n"
		   "    .debug_sfnames  0 : { *(.debug_sfnames) }"                              "\n"
		   "\n"
        // DWARF 1.1 and DWARF 2
		   "    .debug_aranges  0 : { *(.debug_aranges) }"                              "\n"
		   "    .debug_pubnames 0 : { *(.debug_pubnames) }"                             "\n"
		   "\n"
        // DWARF 2
		   "    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }"              "\n"
		   "    .debug_abbrev   0 : { *(.debug_abbrev) }"                               "\n"
		   "    .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }"  "\n"
		   "    .debug_frame    0 : { *(.debug_frame) }"                                "\n"
		   "    .debug_str      0 : { *(.debug_str) }"                                  "\n"
		   "    .debug_loc      0 : { *(.debug_loc) }"                                  "\n"
		   "    .debug_macinfo  0 : { *(.debug_macinfo) }"                              "\n"
		   "\n"
        // SGI/MIPS DWARF 2 extensions
		   "    .debug_weaknames 0 : { *(.debug_weaknames) }"                           "\n"
		   "    .debug_funcnames 0 : { *(.debug_funcnames) }"                           "\n"
		   "    .debug_typenames 0 : { *(.debug_typenames) }"                           "\n"
		   "    .debug_varnames  0 : { *(.debug_varnames) }"                            "\n"
		   "\n"
        // DWARF 3
		   "    .debug_pubtypes 0 : { *(.debug_pubtypes) }"                             "\n"
		   "    .debug_ranges   0 : { *(.debug_ranges) }"                               "\n"
		   "\n"
        // DWARF Extensions
		   "    .debug_addr     0 : { *(.debug_addr) }"                                 "\n"
		   "    .debug_line_str 0 : { *(.debug_line_str) }"                             "\n"
		   "    .debug_loclists 0 : { *(.debug_loclists) }"                             "\n"
		   "    .debug_macro    0 : { *(.debug_macro) }"                                "\n"
		   "    .debug_names    0 : { *(.debug_names) }"                                "\n"
		   "    .debug_rnglists 0 : { *(.debug_rnglists) }"                             "\n"
		   "    .debug_str_offsets 0 : { *(.debug_str_offsets) }"                       "\n"
		   "    .debug_sup      0 : { *(.debug_sup) }"                                  "\n"
		   "\n"
        // gnu attributes
		   "    .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }"                      "\n"
           "\n"
        // mdebug debug sections
		   "    .mdebug         0 : { KEEP (*(.mdebug)) }"                              "\n"
		   "    .mdebug.abi32   0 : { KEEP (*(.mdebug.abi32)) }"                        "\n"
           "\n"

        // Discard all unmentioned sections

           "    /DISCARD/ :"                                                            "\n"
           "    {"                                                                      "\n"
           "       *(*);"                                                               "\n"
           "    }"                                                                      "\n", fout);
}

#define PLF_DIR "build/segments"
#define PLF_EXT ".plf"

#define DEP_EXT ".d"
#define LCF_EXT ".lcf"

static void write_ld_script(FILE *fout)
{
    int i;

    fputs(                  "OUTPUT_ARCH (mips)\n\n"
                            "SECTIONS {\n"
                            "    _RomSize = 0;\n"
                            "    _RomStart = _RomSize;\n\n",
          fout);

    for (i = 0; i < g_segmentsCount; i++)
    {
        const struct Segment *seg = &g_segments[i];

        // align start of ROM segment

        if (seg->fields & (1 << STMT_romalign))
            fprintf(fout,   "    _RomSize = (_RomSize + %i) & ~ %i;\n", seg->romalign - 1, seg->romalign - 1);

        // initialized data (.text, .data, .rodata, .sdata)

        fprintf(fout,       "    _%sSegmentRomStartTemp = _RomSize;\n"
                            "    _%sSegmentRomStart = _%sSegmentRomStartTemp;\n"
                            "    ..%s ",
                seg->name, seg->name, seg->name, seg->name);

        if (seg->fields & (1 << STMT_after))
            fprintf(fout, "_%sSegmentEnd ", seg->after);
        else if (seg->fields & (1 << STMT_number))
            fprintf(fout, "0x%02X000000 ", seg->number);
        else if (seg->fields & (1 << STMT_address))
            fprintf(fout, "0x%08X ", seg->address);

        // (AT(_RomSize) isn't necessary, but adds useful "load address" lines to the map file)
        fprintf(fout,       ": AT(_RomSize)\n"
                            "    {\n"
                            "        _%sSegmentStart = .;\n",
                seg->name);

        if (seg->fields & (1 << STMT_align))
            fprintf(fout,   "        . = ALIGN(0x%X);\n", seg->align);

        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.text)\n",    seg->name);
        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.data)\n",    seg->name);
        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.rodata)\n",  seg->name);
        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.sdata)\n",   seg->name);
        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.ovl)\n",     seg->name);

        if (seg->fields & (1 << STMT_increment))
            fprintf(fout,   "    . += 0x%08X;\n", seg->increment);

        fputs(              "    }\n", fout);

        fprintf(fout,       "    _RomSize += ( _%sSegmentOvlEnd - _%sSegmentTextStart );\n", seg->name, seg->name);

        fprintf(fout,       "    _%sSegmentRomEndTemp = _RomSize;\n"
                            "    _%sSegmentRomEnd = _%sSegmentRomEndTemp;\n\n",
                  seg->name, seg->name, seg->name);

        // align end of ROM segment
        if (seg->fields & (1 << STMT_romalign))
            fprintf(fout,   "    _RomSize = (_RomSize + %i) & ~ %i;\n", seg->romalign - 1, seg->romalign - 1);

        // uninitialized data (.sbss, .scommon, .bss, COMMON)

        fprintf(fout,       "    ..%s.bss ADDR(..%s) + SIZEOF(..%s) (NOLOAD) :\n"
                            "    {\n",
                seg->name, seg->name, seg->name);

        if (seg->fields & (1 << STMT_align))
            fprintf(fout,   "        . = ALIGN(0x%X);\n", seg->align);

        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.sbss)\n",    seg->name);
        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.scommon)\n", seg->name);
        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (.bss)\n",     seg->name);
        fprintf(fout,       "            " PLF_DIR "/%s" PLF_EXT " (COMMON)\n",   seg->name);

        fprintf(fout,       "        _%sSegmentEnd = .;\n"
                            "    }\n"
                            "\n",
                seg->name);
    }

    fputs(                  "    _RomEnd = _RomSize;\n\n", fout);

    write_epilogue(fout);

    fputs(                  "}\n", fout);
}

static void write_seg_ld_script(FILE *fout, const char *segname)
{
    int i;
    const struct Segment *seg = segment_by_name(segname, g_segments, g_segmentsCount);

    if (seg == NULL)
        util_fatal_error("No segment named %s found", segname);

    fputs(                  "OUTPUT_ARCH (mips)\n"
                            "\n"
                            "SECTIONS {\n", fout);

    // TEXT

    fprintf(fout,           "    .text ALIGN(16) : {\n"
                            "        _%sSegmentTextStart = .;\n", seg->name);

    for (i = 0; i < seg->includesCount; i++)
    {
        fprintf(fout,       "            %s (.text)\n", seg->includes[i].fpath);
        if (seg->includes[i].linkerPadding != 0)
            fprintf(fout,   "            . += 0x%X;\n", seg->includes[i].linkerPadding);
    }

    fprintf(fout,           "        _%sSegmentTextEnd = .;\n"
                            "    }\n"
                            "    _%sSegmentTextSize = ABSOLUTE( _%sSegmentTextEnd - _%sSegmentTextStart );\n"
                            "\n", seg->name, seg->name, seg->name, seg->name);

    // DATA

    fprintf(fout,           "    .data ALIGN(16) : {\n"
                            "        _%sSegmentDataStart = .;\n", seg->name);

    for (i = 0; i < seg->includesCount; i++)
    {
        if (!seg->includes[i].dataWithRodata)
            fprintf(fout,   "            %s (.data)\n", seg->includes[i].fpath);
    }

    fprintf(fout,           "        _%sSegmentDataEnd = .;\n"
                            "    }\n"
                            "    _%sSegmentDataSize = ABSOLUTE( _%sSegmentDataEnd - _%sSegmentDataStart );\n"
                            "\n", seg->name, seg->name, seg->name, seg->name);

    // RODATA

    fprintf(fout,           "    .rodata ALIGN(16) : {\n"
                            "        _%sSegmentRoDataStart = .;\n", seg->name);

    for (i = 0; i < seg->includesCount; i++)
    {
        if (seg->includes[i].dataWithRodata)
            fprintf(fout,   "            %s (.data)\n", seg->includes[i].fpath);

        fprintf(fout,       "            %s (.rodata)\n", seg->includes[i].fpath);
        // Compilers other than IDO, such as GCC, produce different sections such as
        // the ones named directly below. These sections do not contain values that
        // need relocating, but we need to ensure that the base .rodata section
        // always comes first. The reason this is important is due to relocs assuming
        // the base of .rodata being the offset for the relocs and thus needs to remain
        // the beginning of the entire rodata area in order to remain consistent.
        // Inconsistencies will lead to various .rodata reloc crashes as a result of
        // either missing relocs or wrong relocs.
        fprintf(fout,       "            %s (.rodata.str1.4)\n", seg->includes[i].fpath);
        fprintf(fout,       "            %s (.rodata.cst4)\n", seg->includes[i].fpath);
        fprintf(fout,       "            %s (.rodata.cst8)\n", seg->includes[i].fpath);
    }

    fprintf(fout,           "        _%sSegmentRoDataEnd = .;\n"
                            "    }\n"
                            "    _%sSegmentRoDataSize = ABSOLUTE( _%sSegmentRoDataEnd - _%sSegmentRoDataStart );\n"
                            "\n", seg->name, seg->name, seg->name, seg->name);

    // SDATA

    fprintf(fout,           "    .sdata ALIGN(16) : {\n"
                            "        _%sSegmentSDataStart = .;\n", seg->name);

    for (i = 0; i < seg->includesCount; i++)
        fprintf(fout, "            %s (.sdata)\n", seg->includes[i].fpath);

    fprintf(fout,           "        _%sSegmentSDataEnd = .;\n"
                            "    }\n"
                            "    _%sSegmentSDataSize = ABSOLUTE( _%sSegmentSDataEnd - _%sSegmentSDataStart );\n"
                            "\n", seg->name, seg->name, seg->name, seg->name);

    // OVL

    fprintf(fout,           "    .ovl ALIGN(16) : {\n"
                            "        _%sSegmentOvlStart = .;\n", seg->name);

    for (i = 0; i < seg->includesCount; i++)
        fprintf(fout, "            %s (.ovl)\n", seg->includes[i].fpath);

    fprintf(fout,           "        _%sSegmentOvlEnd = .;\n"
                            "    }\n"
                            "    _%sSegmentOvlSize = ABSOLUTE( _%sSegmentOvlEnd - _%sSegmentOvlStart );\n"
                            "\n", seg->name, seg->name, seg->name, seg->name);

    // BSS

    fprintf(fout,           "    .bss ALIGN(16) : {\n"
                            "        _%sSegmentBssStart = .;\n", seg->name);

    for (i = 0; i < seg->includesCount; i++)
        fprintf(fout,       "            %s (.sbss)\n", seg->includes[i].fpath);

    for (i = 0; i < seg->includesCount; i++)
        fprintf(fout,       "            %s (.scommon)\n", seg->includes[i].fpath);

    for (i = 0; i < seg->includesCount; i++)
        fprintf(fout,       "            %s (.bss)\n", seg->includes[i].fpath);

    for (i = 0; i < seg->includesCount; i++)
        fprintf(fout,       "            %s (COMMON)\n", seg->includes[i].fpath);

    fprintf(fout,           "        _%sSegmentBssEnd = .;\n"
                            "    }\n"
                            "    _%sSegmentBssSize = ABSOLUTE( _%sSegmentBssEnd - _%sSegmentBssStart );\n",
            seg->name, seg->name, seg->name, seg->name);

    write_epilogue(fout);

    fputs(                  "}\n", fout);
}

static void usage(const char *execname)
{
    fprintf(stderr, "Nintendo 64 linker script generation tool v0.04\n"
                    "usage: %s SPEC_FILE LD_SCRIPT [SEGMENT] [-d]\n"
                    "SPEC_FILE  file describing the organization of object files into segments\n"
                    "LD_SCRIPT  filename of output linker script\n"
                    "SEGMENT    optional: name of segment to create linker script for\n",
                    execname);
}

int main(int argc, char **argv)
{
    void *spec;
    FILE *ldout;
    const char *segname;
    size_t size;

    if (argc != 3 && argc != 4)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    spec = util_read_whole_file(argv[1], &size);
    parse_rom_spec(spec, &g_segments, &g_segmentsCount);

    ldout = fopen(argv[2], "w");
    if (ldout == NULL)
        util_fatal_error("failed to open file '%s' for writing", argv[2]);

    segname = (argc == 4) ? argv[3] : NULL;

    if (segname != NULL)
        write_seg_ld_script(ldout, segname);
    else
        write_ld_script(ldout);

    fclose(ldout);

    free_rom_spec(g_segments, g_segmentsCount);
    free(spec);

    return EXIT_SUCCESS;
}
