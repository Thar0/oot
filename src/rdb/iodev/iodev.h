#ifndef _IODEV_H_
#define _IODEV_H_

#include "global.h"

typedef s32 (*PollFunc)(void);
typedef s32 (*IOFunc)(void* dat, size_t nBlocks);
typedef void (*ParamSetFunc)(s32);

typedef struct {
    /* 0x00 */ PollFunc probe;          // check for existence of this device
    /* 0x04 */ PollFunc fifoPoll;       // check for data arrival in the fifo
    /* 0x08 */ PollFunc unsafeFifoPoll; // not for general use, if this offset changes modify exceptasm.s
    /* 0x0C */ IOFunc   fifoRead;       // read from fifo
    /* 0x10 */ IOFunc   fifoWrite;      // write to fifo
} IODev; // size = 0x18

extern IODev gDevED64_v3; // Everdrive 64 v3
extern IODev gDevED64_x;  // Everdrive 64 x series
extern IODev gDevEmu;     // Project64 script "device"

extern IODev* gIODevice; // Active IO device interface

s32 IODev_Init(void);

s32 PiMaybeGetAccess(void);

#endif
