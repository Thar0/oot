/**
 * SPDX-FileCopyrightText: Copyright (C) 2024 ZeldaRET
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "xml.h"
#include "aifc.h"
#include "samplebank.h"
#include "util.h"

NORETURN static void
usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [--matching] <filename.xml> <out.s>\n", progname);
    exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
    static uint8_t match_buf[BUG_BUF_SIZE];
    const char *filename = NULL;
    xmlDocPtr document;
    const char *outfilename = NULL;
    FILE *outf;
    samplebank sb;
    uint8_t *match_buf_ptr;
    size_t match_buf_pos;
    bool got_infile = false;
    bool matching = false;

    // parse args
    if (argc != 3 && argc != 4)
        usage(argv[0]);

    for (int i = 1; i < argc; i++) {
        if (strequ(argv[i], "--matching")) {
            if (argc != 4 || matching)
                usage(argv[0]);

            matching = true;
        } else if (!got_infile) {
            filename = argv[i];
            got_infile = true;
        } else {
            outfilename = argv[i];
        }
    }

    assert(filename != NULL);
    assert(outfilename != NULL);

    // open xml
    document = xmlReadFile(filename, NULL, XML_PARSE_NONET);
    if (document == NULL)
        return EXIT_FAILURE;

    // xml_print_tree(document);

    // parse xml
    read_samplebank_xml(&sb, document);

    // printf("Sample Bank %d [%s]\n", sb.index, sb.name);
    // printf("Paths:\n");
    // for (size_t i = 0; i < sb.num_samples; i++)
    //     printf("    [%s] = \"%s\"\n", sb.sample_names[i], sb.sample_paths[i]);

    // open output asm file
    outf = fopen(outfilename, "w");
    if (outf == NULL)
        error("Unable to open output file [%s] for writing", outfilename);

    // write output

    fprintf(outf,
            // clang-format off
           ".rdata"                 "\n"
           ".balign 16"             "\n"
                                    "\n"
           ".global %s_Start"       "\n"
           "%s_Start:"              "\n"
           "$start:"                "\n",
            // clang-format on
            sb.name, sb.name);

    // original tool appears to have a buffer clearing bug involving a buffer sized BUG_BUF_SIZE
    match_buf_ptr = (matching) ? match_buf : NULL;
    match_buf_pos = 0;

    for (size_t i = 0; i < sb.num_samples; i++) {
        const char *name = sb.sample_names[i];
        const char *path = sb.sample_paths[i];
        bool is_sample = sb.is_sample[i];

        if (!is_sample) {
            // blob
            fprintf(outf,
                    // clang-format off
                                        "\n"
                   "# BLOB %s"          "\n"
                                        "\n"
                   ".incbin \"%s\""     "\n"
                                        "\n"
                   ".balign 16"         "\n"
                                        "\n",
                    // clang-format on
                    name, path);
            continue;
        }

        // aifc sample
        fprintf(outf,
                // clang-format off
                                                "\n"
               "# SAMPLE %lu"                   "\n"
                                                "\n"
               ".global %s_%s_Abs"              "\n"
               "%s_%s_Abs:"                     "\n"
               ".global %s_%s_Off"              "\n"
               ".set %s_%s_Off, . - $start"     "\n"
                                                "\n",
                // clang-format on
                i, sb.name, name, sb.name, name, sb.name, name, sb.name, name);

        aifc_data aifc;
        aifc_read(&aifc, path, match_buf_ptr, &match_buf_pos);

        fprintf(outf, ".incbin \"%s\", 0x%lX, 0x%lX\n", path, aifc.ssnd_offset, aifc.ssnd_size);

        if (matching && sb.buffer_bug && i == sb.num_samples - 1) {
            // emplace garbage
            size_t end = ALIGN16(match_buf_pos);

            for (; match_buf_pos < end; match_buf_pos++)
                fprintf(outf, ".byte 0x%02X\n", match_buf[match_buf_pos]);
        } else {
            fputs("\n.balign 16\n", outf);
        }

        aifc_dispose(&aifc);
    }

    fprintf(outf,
            // clang-format off
           ".global %s_Size"            "\n"
           ".set %s_Size, . - $start"   "\n",
            // clang-format on
            sb.name, sb.name);

    fclose(outf);
    xmlFreeDoc(document);
    return EXIT_SUCCESS;
}
