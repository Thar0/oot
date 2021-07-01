#include "iodev.h"

/*
 *  Maybe get access to the PI,
 *  returns true if access was obtained or false otherwise
 */
s32 PiMaybeGetAccess(void) {
    if (!__osPiAccessQueueEnabled) {
        __osPiCreateAccessQueue();
    }

    return osRecvMesg(&__osPiAccessQueue, NULL, OS_MESG_NOBLOCK) == 0;
}

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
