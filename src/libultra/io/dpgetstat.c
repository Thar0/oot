#include "rcp.h"
#include "rdp.h"

u32 osDpGetStatus(void) {
    return IO_READ(DPC_STATUS_REG);
}
