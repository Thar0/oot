#include "ultra64/asm.h"

.rdata

.align 2
DATA(__libm_qnan_f)
    .word 0x7F810000
ENDDATA(__libm_qnan_f)
