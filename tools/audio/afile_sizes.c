/* SPDX-FileCopyrightText: Copyright (C) 2024 ZeldaRET */
/* SPDX-License-Identifier: CC0-1.0 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "elf32.h"
#include "util.h"

static int
usage(const char *progname)
{
    fprintf(stderr, "Usage: %s <header_out.h> <num define> <header guard> <object files...>\n", progname);
    return EXIT_FAILURE;
}

static void
file_handler(const char *path, FILE *out, size_t *total_num)
{
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

    char *object_name = NULL;
    size_t object_size = 0;

    Elf32_Sym *sym = GET_PTR(data, symtab->sh_offset);
    Elf32_Sym *sym_end = GET_PTR(data, symtab->sh_offset + symtab->sh_size);

    for (size_t i = 0; sym < sym_end; sym++, i++) {
        validate_read(symtab->sh_offset + i * sizeof(Elf32_Sym), sizeof(Elf32_Sym), data_size);

        // The start and size symbol should be defined and global
        if (sym->st_shndx == SHN_UND || (sym->st_info >> 4) != SB_GLOBAL) {
            continue;
        }

        const char *sym_name = elf32_get_string(sym->st_name, strtab, data, data_size);
        size_t name_len = strlen(sym_name);

        if (name_len > 5 && strcmp(&sym_name[name_len - 5], "_Size") == 0) {
            if (sym->st_shndx != SHN_ABS)
                continue;

            object_size = sym->st_value;

            if (object_name != NULL)
                break;

            continue;
        }

        if (name_len > 6 && strcmp(&sym_name[name_len - 6], "_Start") == 0) {
            object_name = (char *)sym_name;
            object_name[name_len - 6] = '\0';

            if (object_size != 0)
                break;

            continue;
        }
    }

    if (object_name == NULL)
        error("Object \"%s\" has no start symbol?", path);
    if (object_size == 0)
        error("Object \"%s\" has no size symbol?", path);

    fprintf(out, "#define %s_SIZE 0x%lX\n", object_name, object_size);
    (*total_num)++;

    free(data);
}

int
main(int argc, char **argv)
{
    const char *progname = argv[0];

    if (argc < 5) // progname, 3 required args, at least 1 input file
        return usage(progname);

    const char *header_out = argv[1];
    const char *num_def = argv[2];
    const char *header_guard = argv[3];

    FILE *out = fopen(header_out, "w");
    if (out == NULL)
        error("Could not open output file \"%s\" for writing", header_out);

    fprintf(out,
            // clang-format off
           "#ifndef %s_H_"      "\n"
           "#define %s_H_"      "\n"
                                "\n",
            // clang-format on
            header_guard, header_guard);

    size_t total_num = 0;

    for (int i = 4; i < argc; i++) {
        file_handler(argv[i], out, &total_num);
    }

    fprintf(out,
            // clang-format off
                                "\n"
           "#define %s %lu"     "\n"
                                "\n"
           "#endif"             "\n",
            // clang-format on
            num_def, total_num);
    fclose(out);

    return EXIT_SUCCESS;
}
