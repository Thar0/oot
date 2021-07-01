#include "iodev.h"

IODev* gIODevice = NULL;

s32 IODev_Init(void) {
    if (gDevED64_v3.probe()) {
        gIODevice = &gDevED64_v3;
        return true;
    } else if (gDevED64_x.probe()) {
        gIODevice = &gDevED64_x;
        return true;
    } else if (gDevEmu.probe()) {
        gIODevice = &gDevEmu;
        return true;
    }
    return false;
}
