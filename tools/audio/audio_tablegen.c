/**
 * SPDX-FileCopyrightText: Copyright (C) 2024 ZeldaRET
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "samplebank.h"
#include "soundfont.h"
#include "elf32.h"
#include "xml.h"
#include "util.h"

/* Utility */

static bool
is_xml(const char *path)
{
    size_t len = strlen(path);

    if (len < 4)
        return false;
    if (path[len - 4] == '.' && tolower(path[len - 3]) == 'x' && tolower(path[len - 2]) == 'm' &&
        tolower(path[len - 1]) == 'l')
        return true;

    return false;
}

static bool
is_ofile(const char *path)
{
    size_t len = strlen(path);

    if (len < 2)
        return false;
    if (path[len - 2] == '.' && tolower(path[len - 1]) == 'o')
        return true;

    if (len < 4)
        return false;
    if (path[len - 4] == '.' && tolower(path[len - 3]) == 'e' && tolower(path[len - 2]) == 'l' &&
        tolower(path[len - 1]) == 'f')
        return true;

    return false;
}

/* Samplebanks */

int
tablegen_samplebanks(const char *sb_hdr_out, const char **samplebanks_paths, int num_samplebank_files)
{
    samplebank *samplebanks = malloc(num_samplebank_files * sizeof(samplebank));

    for (int i = 0; i < num_samplebank_files; i++) {
        const char *path = samplebanks_paths[i];

        if (!is_xml(path))
            error("Not an xml file? (\"%s\")", path);

        // printf("\"%s\"\n", path);

        // open xml
        xmlDocPtr document = xmlReadFile(path, NULL, XML_PARSE_NONET);
        if (document == NULL)
            error("Could not read xml file \"%s\"\n", path);

        // parse xml
        read_samplebank_xml(&samplebanks[i], document);
    }

    // find largest index

    size_t indices_max = 0;

    for (int i = 0; i < num_samplebank_files; i++) {
        samplebank *sb = &samplebanks[i];
        unsigned index = sb->index;
        if (index > indices_max)
            indices_max = index;

        for (size_t j = 0; j < sb->num_pointers; j++) {
            index = sb->pointer_indices[j];
            if (index > indices_max)
                indices_max = index;
        }
    }

    size_t indices_len = indices_max + 1;

    // classify indices, sort cache policies, check that no two indices are the same

#define INDEX_NONE      0
#define INDEX_NOPOINTER 1
#define INDEX_POINTER   2

    struct sb_index_info {
        char index_type;
        const char *name;
        unsigned ptr_index;
        const char *medium;
        const char *cache_policy;
    };
    struct sb_index_info *index_info = calloc(indices_len, sizeof(struct sb_index_info));

    for (int i = 0; i < num_samplebank_files; i++) {
        samplebank *sb = &samplebanks[i];
        unsigned index = sb->index;

        if (index_info[index].index_type != INDEX_NONE)
            error("Overlapping indices");
        index_info[index].index_type = INDEX_NOPOINTER;

        index_info[index].name = sb->name;
        index_info[index].medium = sb->medium;
        index_info[index].cache_policy = sb->cache_policy;

        unsigned real_index = index;

        // Add pointers
        for (size_t j = 0; j < sb->num_pointers; j++) {
            index = sb->pointer_indices[j];

            if (index_info[index].index_type != INDEX_NONE)
                error("Overlapping indices");
            index_info[index].index_type = INDEX_POINTER;

            index_info[index].ptr_index = real_index;
            index_info[index].medium = sb->medium;
            index_info[index].cache_policy = sb->cache_policy;
        }
    }

    // check that we have no gaps

    for (size_t i = 0; i < indices_len; i++) {
        if (index_info[i].index_type == INDEX_NONE)
            error("Missing index");
    }

    // emit table

    FILE *out = fopen(sb_hdr_out, "w");
    if (out == NULL)
        error("Failed to open samplebank header output");

    fprintf(out,
            // clang-format off
           "/**"                                                    "\n"
           " * DEFINE_SAMPLE_BANK(name, medium, cachePolicy)"       "\n"
           " * DEFINE_SAMPLE_BANK_PTR(name, medium, cachePolicy)"   "\n"
           " */"                                                    "\n"
            // clang-format on
    );

    for (size_t i = 0; i < indices_len; i++) {
        if (index_info[i].index_type == INDEX_NOPOINTER) {
            fprintf(out, "DEFINE_SAMPLE_BANK    (%s, %s, %s)\n", index_info[i].name, index_info[i].medium,
                    index_info[i].cache_policy);
        } else if (index_info[i].index_type == INDEX_POINTER) {
            fprintf(out, "DEFINE_SAMPLE_BANK_PTR(%u, %s, %s)\n", index_info[i].ptr_index, index_info[i].medium,
                    index_info[i].cache_policy);
        } else {
            assert(false);
        }
    }

    fclose(out);

    free(index_info);
    free(samplebanks);

    return EXIT_SUCCESS;
}

/* Soundfonts */

static int
validate_samplebank_index(samplebank *sb, int ptr_idx)
{
    if (ptr_idx != -1) {
        // Validate pointer index
        bool found = false;

        for (size_t i = 0; i < sb->num_pointers; i++) {
            if (ptr_idx == sb->pointer_indices[i]) {
                found = true;
                break;
            }
        }
        if (!found)
            warning("Invalid pointer indirect: %d for samplebank %s\n", ptr_idx, sb->name);

        return ptr_idx;
    } else {
        return sb->index;
    }
}

int
tablegen_soundfonts(const char *sf_hdr_out, char **soundfonts_paths, int num_soundfont_files)
{
    soundfont *soundfonts = malloc(num_soundfont_files * sizeof(soundfont));
    int max_index = 0;

    for (int i = 0; i < num_soundfont_files; i++) {
        char *path = soundfonts_paths[i];

        if (!is_xml(path))
            error("Not an xml file? (\"%s\")", path);

        xmlDocPtr document = xmlReadFile(path, NULL, XML_PARSE_NONET);
        if (document == NULL)
            error("Could not read xml file \"%s\"\n", path);

        xmlNodePtr root = xmlDocGetRootElement(document);
        if (!strequ(XMLSTR_TO_STR(root->name), "Soundfont"))
            error("Root node must be <Soundfont>");

        soundfont *sf = &soundfonts[i];

        // Transform the xml path into a header include path
        // Assumption: replacing .xml -> .h forms a valid header include path
        size_t pathlen = strlen(path);
        path[pathlen - 3] = 'h';
        path[pathlen - 2] = '\0';

        read_soundfont_info(sf, root);

        if (max_index < sf->info.index)
            max_index = sf->info.index;
    }

    struct soundfont_file_info {
        soundfont *soundfont;
        int normal_bank_index;
        int dd_bank_index;
        char *name;
    };
    struct soundfont_file_info *finfo = calloc(max_index + 1, sizeof(struct soundfont_file_info));

    for (int i = 0; i < num_soundfont_files; i++) {
        soundfont *sf = &soundfonts[i];

        // Resolve samplebank indices

        int normal_idx = validate_samplebank_index(&sf->sb, sf->info.pointer_index);

        int dd_idx = 255;
        if (sf->info.bank_path_dd != NULL)
            dd_idx = validate_samplebank_index(&sf->sbdd, sf->info.pointer_index_dd);

        // Add info

        if (finfo[sf->info.index].soundfont != NULL)
            error("Duplicate soundfont indices");

        finfo[sf->info.index].soundfont = &soundfonts[i];
        finfo[sf->info.index].normal_bank_index = normal_idx;
        finfo[sf->info.index].dd_bank_index = dd_idx;
        finfo[sf->info.index].name = soundfonts_paths[i];
    }

    // Make sure there are no gaps
    for (int i = 0; i < max_index + 1; i++) {
        if (finfo[i].soundfont == NULL)
            error("No soundfont for index %d", i);
    }

    FILE *out = fopen(sf_hdr_out, "w");

    fprintf(out,
            // clang-format off
           "/**"                                                                    "\n"
           " * DEFINE_SOUNDFONT(name, medium, cachePolicy, sampleBankNormal, "
                               "sampleBankDD, nInstruments, nDrums, nSfx)"          "\n"
           " */"                                                                    "\n"
            // clang-format on
    );

    for (int i = 0; i < max_index + 1; i++) {
        soundfont *sf = finfo[i].soundfont;

        fprintf(out,
                // clang-format off
               "#include \"%s\""                                                                            "\n"
               "DEFINE_SOUNDFONT(%s, %s, %s, %d, %d, SF%d_NUM_INSTRUMENTS, SF%d_NUM_DRUMS, SF%d_NUM_SFX)"   "\n",
                // clang-format on
                finfo[i].name, sf->info.name, sf->info.medium, sf->info.cache_policy, finfo[i].normal_bank_index,
                finfo[i].dd_bank_index, sf->info.index, sf->info.index, sf->info.index);
    }

    fclose(out);

    free(soundfonts);
    free(finfo);

    return EXIT_SUCCESS;
}

/* Sequences */

struct seqdata {
    char *elf_path;
    char *name;
    uint32_t font_section_offset;
    size_t font_section_size;
};

static void
gen_sequence_font_table(FILE *out, struct seqdata **sequences, size_t num_sequences)
{
    fprintf(out,
            // clang-format off
                                            "\n"
           ".section .rodata"               "\n"
                                            "\n"
           ".global gSequenceFontTable"     "\n"
           "gSequenceFontTable:"            "\n"
            // clang-format on
    );

    for (size_t i = 0; i < num_sequences; i++) {
        fprintf(out, "    .half Fonts_%lu - gSequenceFontTable\n", i);
    }
    fprintf(out, "\n");

    for (size_t i = 0; i < num_sequences; i++) {
        fprintf(out,
                // clang-format off
               "Fonts_%lu:"                         "\n"
               "    .byte %ld"                      "\n"
               "    .incbin \"%s\", 0x%X, %lu"      "\n"
                                                    "\n",
                // clang-format on
                i, sequences[i]->font_section_size, sequences[i]->elf_path, sequences[i]->font_section_offset,
                sequences[i]->font_section_size);
    }
    fprintf(out, ".balign 16\n");
}

struct seq_order {
    size_t num_sequences;
    char **names;
    char **enum_names;
    bool *ptr_flags;
    void *buf_ptr;
};

static void
read_seq_order(struct seq_order *order, const char *path)
{
    // Read from file, we assume the file has been preprocessed with cpp so that whitespace is collapsed and each line
    // has the form:
    // (name,enum_name) or PTR(name,enum_name)
    UNUSED size_t data_size;
    char *filedata = util_read_whole_file(path, &data_size);

    // We expect one per line

    size_t total_size = 0;

    for (char *p = filedata; *p != '\0'; p++) {
        if (*p == '\n') {
            total_size++;
        } else if (isspace(*p)) {
            goto malformed;
        }
    }

    char **name_list = malloc(total_size * sizeof(char *));
    char **enum_name_list = malloc(total_size * sizeof(char *));
    bool *ptr_flags = malloc(total_size * sizeof(bool));

    char *startp = filedata;

    size_t i = 0;
    bool newline = true;
    bool eol = false;
    bool gotname = false;
    for (char *p = filedata; *p != '\0'; p++) {
        // printf("%c (%d, %d, %d)\n", *p, newline, eol, gotname);

        if (newline) {
            // Started a new line, look for either an opening '(' or '*(', anything else is invalid.

            bool isptr = false;
            if (*p == '*') {
                p++;
                isptr = true;
            }
            // Record whether it was a pointer or not
            ptr_flags[i] = isptr;

            // Should always have '(' here
            if (*p != '(')
                goto malformed;

            // End of newline, start name
            newline = false;
            gotname = false;
            startp = p + 1;
            continue;
        }

        if (eol) {
            // At end of line, only whitespace is permitted
            if (!isspace(*p))
                goto malformed;

            // When we find the new line, return to newline state
            if (*p == '\n') {
                newline = true;
                eol = false;
            }
            continue;
        }

        // Deal with middle of the line, of the form: "<name>,<enum_name>)"

        // Whitespace is prohibited
        if (isspace(*p))
            goto malformed;

        if (!gotname) {
            // Haven't found name yet, wait for comma
            if (*p == ',') {
                // Record start pos
                name_list[i] = startp;
                // Null-terminate (replaces ',')
                *p = '\0';
                // Set new start pos for enum_name
                startp = p + 1;
                gotname = true;
            }
        } else {
            // Have found name, now we want enum_name
            if (*p == ')') {
                // Record start pos
                enum_name_list[i] = startp;
                // Null-terminate (replaces ')')
                *p = '\0';
                // Enum name is the last thing we care about on this line, advance index
                i++;
                // Continue to eol handling
                eol = true;
            }
        }
    }
    // We should have found one entry for each line
    assert(i == total_size);

    // Write results
    order->num_sequences = total_size;
    order->names = name_list;
    order->enum_names = enum_name_list;
    order->ptr_flags = ptr_flags;
    order->buf_ptr = filedata; // so we can free it
    return;
malformed:
    error("Malformed sequence_order.in?");
}

int
tablegen_sequences(const char *seq_font_tbl_out, const char *seq_order_path, const char **sequences_paths,
                   int num_sequence_files)
{
    struct seq_order order;
    read_seq_order(&order, seq_order_path);

    // for (size_t i = 0; i < order.num_sequences; i++) {
    //     printf("\"%s\" \"%s\" %d\n", order.names[i], order.enum_names[i], order.ptr_flags[i]);
    // }

    struct seqdata *file_data = malloc(num_sequence_files * sizeof(struct seqdata));

    for (int i = 0; i < num_sequence_files; i++) {
        const char *path = sequences_paths[i];

        if (!is_ofile(path))
            error("Not a .o file? (\"%s\")", path);

        // printf("\"%s\"\n", path);

        // Open ELF file

        size_t data_size;
        void *data = elf32_read(path, &data_size);

        Elf32_Shdr *symtab = elf32_get_symtab(data, data_size);
        if (symtab == NULL)
            error("No symtab?");
        Elf32_Shdr *shstrtab = elf32_get_shstrtab(data, data_size);
        if (shstrtab == NULL)
            error("No shstrtab?");
        Elf32_Shdr *strtab = elf32_get_strtab(data, data_size);
        if (symtab == NULL)
            error("No strtab?");

        Elf32_Shdr *font_section = elf32_section_forname(".fonts", shstrtab, data, data_size);
        if (font_section == NULL)
            error("No fonts defined?");

        // Find the <name>_Start symbol

        uint32_t sh_offset = elf32_read32(symtab->sh_offset);
        uint32_t sh_size = elf32_read32(symtab->sh_size);

        Elf32_Sym *sym = GET_PTR(data, sh_offset);
        Elf32_Sym *sym_end = GET_PTR(data, sh_offset + sh_size);

        char *seq_name = NULL;

        for (size_t i = 0; sym < sym_end; sym++, i++) {
            validate_read(sh_offset + i * sizeof(Elf32_Sym), sizeof(Elf32_Sym), data_size);

            // The start symbol should be defined and global
            int bind = sym->st_info >> 4;
            if (bind != SB_GLOBAL || elf32_read16(sym->st_shndx) == SHN_UND)
                continue;

            const char *sym_name = elf32_get_string(elf32_read32(sym->st_name), strtab, data, data_size);
            size_t name_len = strlen(sym_name);

            if (!str_endswith(sym_name, name_len, "_Start"))
                continue;

            // Got start symbol, extract name
            seq_name = malloc(name_len - 6 + 1);
            strncpy(seq_name, sym_name, name_len - 6);
            seq_name[name_len - 6] = '\0';
            break;
        }

        if (seq_name == NULL)
            error("No start symbol?");

        // Populate new data
        struct seqdata *seqdata = &file_data[i];
        seqdata->elf_path = strdup(path);
        seqdata->name = seq_name;
        seqdata->font_section_offset = elf32_read32(font_section->sh_offset);
        seqdata->font_section_size = elf32_read32(font_section->sh_size);

        free(data);
    }

#if 0
    printf(                 "\n");
    printf("Result:"        "\n");
    printf("Num files: %lu" "\n", num_sequence_files);
    printf(                 "\n");

    for (int i = 0; i < num_sequence_files; i++) {
        struct seqdata *seqdata = &file_data[i];

        printf("    elf path    : \"%s\""   "\n", seqdata->elf_path);
        printf("    name        : \"%s\""   "\n", seqdata->name);
        printf("    font offset : 0x%X"     "\n", seqdata->font_section_offset);
        printf("    num fonts   : %lu"      "\n", seqdata->font_section_size);
        printf(                             "\n");
    }
#endif

    // link against seq order

    struct seqdata **final_seqdata = calloc(order.num_sequences, sizeof(struct seqdata *));

    for (size_t i = 0; i < order.num_sequences; i++) {
        // Skip pointers for now
        if (order.ptr_flags[i])
            continue;

        // If it's not a pointer, "name" is the name as it appears in a sequence file, find it in the list of sequences
        char *name = order.names[i];

        for (int j = 0; j < num_sequence_files; j++) {
            struct seqdata *seqdata = &file_data[j];

            if (strequ(name, seqdata->name)) {
                // Found name, done
                final_seqdata[i] = seqdata;
                break;
            }
        }
    }

    for (size_t i = 0; i < order.num_sequences; i++) {
        // Now we only care about pointers
        if (!order.ptr_flags[i])
            continue;

        // If it's a pointer, "name" is the ENUM name of the REAL sequence and "enum_name" is irrel
        char *name = order.names[i];

        for (size_t j = 0; j < order.num_sequences; j++) {
            // Skip pointers
            if (order.ptr_flags[j])
                continue;

            if (strequ(name, order.enum_names[j])) {
                // For pointers, we just duplicate the fonts for the original into the pointer entry.
                // TODO ideally we would allow fonts to be different when a sequence is accessed by pointer, but how
                // to supply this info?
                final_seqdata[i] = final_seqdata[j];
                break;
            }
        }
    }

    for (size_t i = 0; i < order.num_sequences; i++) {
        if (final_seqdata[i] == NULL)
            error("Could not find object file for sequence %lu : %s", i, order.names[i]);
    }

    FILE *out = fopen(seq_font_tbl_out, "w");
    if (out == NULL)
        error("Failed to open output file");

    gen_sequence_font_table(out, final_seqdata, order.num_sequences);

    fclose(out);

    // Free all

    for (int i = 0; i < num_sequence_files; i++) {
        struct seqdata *seqdata = &file_data[i];

        free(seqdata->elf_path);
        free(seqdata->name);
    }
    free(file_data);
    free(final_seqdata);

    free(order.buf_ptr);
    free(order.names);
    free(order.enum_names);
    free(order.ptr_flags);

    return EXIT_SUCCESS;
}

/* Common */

static int
usage(const char *progname)
{
    fprintf(stderr,
            // clang-format off
           "Usage:"                                                                                 "\n"
           "    %s -banks     <samplebank_table.h> <samplebank xml files...>"                       "\n"
           "    %s -fonts     <soundfont_table.h> <soundfont xml files...>"                         "\n"
           "    %s -sequences <seq_font_table.s> <sequence_order.in> <sequence object files...>"    "\n",
            // clang-format on
            progname, progname, progname);
    return EXIT_FAILURE;
}

int
main(int argc, char **argv)
{
    // -banks [header_out] [files...]
    // -fonts [header_out] [files...]
    // -sequences [seq_font_tbl_out] [order] [files...]
    int ret = EXIT_SUCCESS;

    const char *progname = argv[0];

    if (argc < 2)
        return usage(progname);

    const char *mode = argv[1];

    if (strequ(mode, "-banks")) {
        if (argc < 4)
            return usage(progname);

        const char *sb_hdr_out = argv[2];
        const char **samplebanks_paths = (const char **)&argv[3];
        int num_samplebank_files = argc - 3;

        ret = tablegen_samplebanks(sb_hdr_out, samplebanks_paths, num_samplebank_files);
    } else if (strequ(mode, "-fonts")) {
        if (argc < 4)
            return usage(progname);

        const char *sf_hdr_out = argv[2];
        char **soundfonts_paths = &argv[3];
        int num_soundfont_files = argc - 3;

        ret = tablegen_soundfonts(sf_hdr_out, soundfonts_paths, num_soundfont_files);
    } else if (strequ(mode, "-sequences")) {
        if (argc < 5)
            return usage(progname);

        const char *seq_font_tbl_out = argv[2];
        const char *seq_order_path = argv[3];
        const char **sequences_paths = (const char **)&argv[4];
        int num_sequence_files = argc - 4;

        ret = tablegen_sequences(seq_font_tbl_out, seq_order_path, sequences_paths, num_sequence_files);
    } else {
        return usage(progname);
    }

    return ret;
}
