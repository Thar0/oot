.include "macro.inc"

.section .text

.balign 16

glabel gspS2DEX2d_fifoTextStart
    .incbin "baseroms/gc-eu-mq-dbg/baserom-decompressed.z64", 0xB8A210, 0x18C0
glabel gspS2DEX2d_fifoTextEnd

.section .rodata

.balign 16

glabel gspS2DEX2d_fifoDataStart
    .incbin "baseroms/gc-eu-mq-dbg/baserom-decompressed.z64", 0xBCEB40, 0x390
glabel gspS2DEX2d_fifoDataEnd
