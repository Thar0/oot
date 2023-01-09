#include "ultra64/asm.h"
#include "ultra64/regdef.h"
#include "ultra64/R4300.h"

.text

LEAF(__osGetCause)
    MFC0(   v0, C0_CAUSE)
    nop
    jr      ra
END(__osGetCause)
