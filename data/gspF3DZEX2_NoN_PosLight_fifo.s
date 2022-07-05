.include "macro.inc"

.section .text

.balign 16

#ifndef F3DEX_GBI_PL
glabel gspF3DZEX2_NoN_fifoTextStart
    .incbin "incbin/gspF3DZEX2_NoN_fifoText"
glabel gspF3DZEX2_NoN_fifoTextEnd
#endif

.section .rodata

.balign 16

#ifdef F3DEX_GBI_PL
glabel gspF3DZEX2_NoN_PosLight_fifoTextStart
    .incbin "incbin/gspF3DZEX2_NoN_PosLight_fifoText"
glabel gspF3DZEX2_NoN_PosLight_fifoTextEnd
#endif

.section .rodata

.balign 16

#ifdef F3DEX_GBI_PL
glabel gspF3DZEX2_NoN_PosLight_fifoDataStart
    .incbin "incbin/gspF3DZEX2_NoN_PosLight_fifoData"
glabel gspF3DZEX2_NoN_PosLight_fifoDataEnd
#else
glabel gspF3DZEX2_NoN_fifoDataStart
    .incbin "incbin/gspF3DZEX2_NoN_fifoData"
glabel gspF3DZEX2_NoN_fifoDataEnd
#endif
