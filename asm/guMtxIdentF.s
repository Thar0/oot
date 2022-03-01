#include "ultra64/asm.h"

# assembler directives
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches
.set gp=64     # allow use of 64-bit general purpose registers

.section .text

.balign 32

LEAF(guMtxIdentF)
    li      $t0, 0x3F800000 # 1.0f
    sw      $t0, ($a0)
    sw      $zero, 4($a0)
    sw      $zero, 8($a0)
    sw      $zero, 0xC($a0)
    sw      $zero, 0x10($a0)
    sw      $t0, 0x14($a0)
    sw      $zero, 0x18($a0)
    sw      $zero, 0x1C($a0)
    sw      $zero, 0x20($a0)
    sw      $zero, 0x24($a0)
    sw      $t0, 0x28($a0)
    sw      $zero, 0x2C($a0)
    sw      $zero, 0x30($a0)
    sw      $zero, 0x34($a0)
    sw      $zero, 0x38($a0)
    jr      $ra
     sw     $t0, 0x3C($a0)
END(guMtxIdentF)
