#include "ultra64/asm.h"
#include "ultra64/r4300.h"

# assembler directives
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches
.set gp=64     # allow use of 64-bit general purpose registers

.section .text

.balign 16

LEAF(__osSetCompare)
    mtc0    $a0, C0_COMPARE
    jr      $ra
     nop
END(__osSetCompare)
