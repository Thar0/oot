#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spec.h"
#include "util.h"

struct Segment *g_segments;
int g_segmentsCount;

#define PLF_DIR "build/segments"
#define PLF_EXT ".plf"
#define DEP_EXT ".d"
#define LCF_EXT ".lcf"

static void write_dep(FILE *fout)
{
    int i;

    fprintf(fout,           "zelda_ocarina_mq_dbg.elf: build/ldscript" LCF_EXT " \\\n");

    for (i = 0; i < g_segmentsCount; i++)
    {
        const struct Segment *seg = &g_segments[i];

        fprintf(fout,       "        " PLF_DIR "/%s" PLF_EXT, seg->name);
        if (i != seg->includesCount - 1)
            fputs(                                         " \\", fout);
        fputs(                                                "\n", fout);
    }
}

static void write_seg_dep(FILE *fout, const char *segname)
{
    int i;
    const struct Segment *seg = segment_by_name(segname, g_segments, g_segmentsCount);

    if (seg == NULL)
        util_fatal_error("No segment named %s found", segname);

    fprintf(fout,           PLF_DIR "/%s" PLF_EXT ": " PLF_DIR "/%s" LCF_EXT " \\\n", seg->name, seg->name);

    for (i = 0; i < seg->includesCount; i++)
    {
        fprintf(fout,       "        %s", seg->includes[i].fpath);
        if (i != seg->includesCount - 1)
            fputs(                    " \\", fout);
        fputs(                           "\n", fout);
    }
}

static void usage(const char *execname)
{
    fprintf(stderr, "Nintendo 64 spec dependency tool v0.01\n"
                    "usage: %s SPEC_FILE LD_SCRIPT [SEGMENT]\n"
                    "SPEC_FILE  file describing the organization of object files into segments\n"
                    "DEP_FILE   filename of output dependency file\n"
                    "SEGMENT    optional: name of segment to create dependency file for\n",
                    execname);
}

int main(int argc, char **argv)
{
    void *spec;
    FILE *dep_out;
    const char *segname;
    size_t size;

    if (argc != 3 && argc != 4)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    spec = util_read_whole_file(argv[1], &size);
    parse_rom_spec(spec, &g_segments, &g_segmentsCount);

    dep_out = fopen(argv[2], "w");
    if (dep_out == NULL)
        util_fatal_error("failed to open file '%s' for writing", argv[2]);

    segname = (argc == 4) ? argv[3] : NULL;

    if (segname != NULL)
        write_seg_dep(dep_out, segname);
    else
        write_dep(dep_out);

    fclose(dep_out);

    free_rom_spec(g_segments, g_segmentsCount);
    free(spec);

    return EXIT_SUCCESS;
}
