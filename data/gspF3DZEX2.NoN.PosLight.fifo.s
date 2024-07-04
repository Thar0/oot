.include "macro.inc"

.section .rodata

.balign 16

glabel gspF3DZEX2_NoN_PosLight_fifoTextStart
    .incbin "baseroms/gc-eu-mq-dbg/baserom-decompressed.z64", 0xBCD0F0, 0x1630
glabel gspF3DZEX2_NoN_PosLight_fifoTextEnd

.section .rodata

.balign 16

glabel gspF3DZEX2_NoN_PosLight_fifoDataStart
    .incbin "baseroms/gc-eu-mq-dbg/baserom-decompressed.z64", 0xBCE720, 0x420
glabel gspF3DZEX2_NoN_PosLight_fifoDataEnd
