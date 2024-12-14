/* RSP code for cic6105.c, used only in N64 versions. */
.rsp
#include "rcp.h"
#include "sptask.h"
#include "rspboot.h"

.create CODE_FILE, RSPBOOT_ENTRYPOINT_VIRT

entry:
    // Store the contents of v12, set by RSP code ran during IPL3 6105, to DMEM+0x10
    sqv     $v12[0], 0x10($zero)
    // Set task done and halt the RSP
    li      $1, SP_SET_TASKDONE
    mtc0    $1, SP_STATUS
    break
    nop
    // This loop should be unreachable
forever:
    j       forever
     nop

.if . > IMEM_END_VIRT
    .error "Not enough room in IMEM"
.endif

.close
