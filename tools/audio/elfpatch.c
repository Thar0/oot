/* SPDX-FileCopyrightText: Copyright (C) 2024 ZeldaRET */
/* SPDX-License-Identifier: CC0-1.0 */
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf32.h"
#include "util.h"

int
main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s in.elf out.elf\n", argv[0]);
        return EXIT_FAILURE;
    }

    // read input elf file

    size_t data_size;
    void *data = elf32_read(argv[1], &data_size);

    // locate symtab

    Elf32_Shdr *symtab = elf32_get_symtab(data, data_size);
    if (symtab == NULL)
        error("Symtab not found");

    // patch defined symbols to be ABS

    Elf32_Sym *sym = GET_PTR(data, symtab->sh_offset);
    Elf32_Sym *sym_end = GET_PTR(data, symtab->sh_offset + symtab->sh_size);

    for (size_t i = 0; sym < sym_end; sym++, i++) {
        validate_read(symtab->sh_offset + i * sizeof(Elf32_Sym), sizeof(Elf32_Sym), data_size);

        if (sym->st_shndx != SHN_UND)
            sym->st_shndx = SHN_ABS;
    }

    // write output elf file

    util_write_whole_file(argv[2], data, data_size);
    return EXIT_SUCCESS;
}
