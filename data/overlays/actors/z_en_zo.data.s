.include "macro.inc"

 # assembler directives
 .set noat      # allow manual use of $at
 .set noreorder # don't insert nops after branches
 .set gp=64     # allow use of 64-bit general purpose registers

.section .data

.balign 16

glabel D_80B62450
 .word 0x00000000, 0x00000000, 0x00000000
glabel D_80B6245C
 .word 0x00000000, 0x00000000, 0x00000000
glabel D_80B62468
 .word 0x00000000, 0x3F800000, 0x00000000
glabel D_80B62474
 .word 0x00000000, 0xBF800000, 0x00000000
glabel D_80B62480
 .word 0x00000000, 0x00000000, 0x00000000
glabel D_80B6248C
 .word 0x0A000039, 0x20010000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100, 0x001A0040, 0x00000000, 0x00000000
glabel D_80B624B8
 .word 0x00000000, 0x00000000, 0xFF000000
glabel En_Zo_InitVars
 .word 0x01CE0400, 0x00000009, 0x00FE0000, 0x000006A8
.word EnZo_Init
.word EnZo_Destroy
.word EnZo_Update
.word EnZo_Draw
glabel D_80B624E4
 .word 0x06002FE8, 0x3F800000, 0x00000000, 0xBF800000, 0x00000000, 0xC1000000, 0x06002FE8, 0x3F800000, 0x00000000, 0xBF800000, 0x00000000, 0x00000000, 0x06002F10, 0x00000000, 0x3F800000, 0x3F800000, 0x02000000, 0x00000000, 0x06002F10, 0x3F800000, 0x3F800000, 0xBF800000, 0x00000000, 0xC1000000, 0x06002F10, 0x3F800000, 0x41000000, 0xBF800000, 0x00000000, 0xC1000000, 0x0600219C, 0x3F800000, 0x00000000, 0xBF800000, 0x00000000, 0xC1000000, 0x06000598, 0x3F800000, 0x00000000, 0xBF800000, 0x00000000, 0xC1000000, 0x06000D48, 0x3F800000, 0x00000000, 0xBF800000, 0x00000000, 0xC1000000
glabel D_80B625A4
 .word 0x00000000, 0x44160000, 0x00000000
glabel D_80B625B0
 .word 0x06003E40, 0x06004640, 0x06004E40, 0x00000000

