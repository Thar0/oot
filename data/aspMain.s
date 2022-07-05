.include "macro.inc"

.section .text

.balign 16

glabel aspMainTextStart
    .incbin "baseroms/gc-eu-mq-dbg/baserom-decompressed.z64", 0xB89260, 0xFB0
glabel aspMainTextEnd

.section .rodata

.balign 16

glabel aspMainDataStart
    .incbin "baseroms/gc-eu-mq-dbg/baserom-decompressed.z64", 0xBCCE10, 0x2E0
glabel aspMainDataEnd
