#include "rdb/rdb.h"
#include "rdb/iodev/iodev.h"

char sRdbPollStack[0x150]; // modify exceptasm.s if this size changes
char sRdbThreadStack[0x1000];
OSThread gRdbThread;
OSMesgQueue sRdbEvtQ;
OSMesg sRdbMesgBuf[8];

s32 gRdbWorking = false;
s32 gRdbActive = false;

#define PKT_SIZE 512

#define PKT_TYPE_LOG  0x01
#define PKT_TYPE_PING 0x02

typedef struct {
    u8 type;
    u32 crc; // not yet implemented
} PacketHeader;

typedef struct {
    PacketHeader header;
    union {
        char raw[PKT_SIZE - sizeof(PacketHeader)];
        long long int force_structure_alignment;
    } payload;
} RdbPacket;

RdbPacket ipkt;
RdbPacket opkt;

void* Rdb_ProutSyncPrintf(void* arg, const char* str, u32 count) {
    static const u32 payload_size = sizeof(RdbPacket) - sizeof(PacketHeader);
    static RdbPacket blk;
    static u32 blk_p;

    s32 endsWithNewline;

    if (!gRdbActive)
        return (void*)1;

    // silly hack to make output cleaner, it slows down the overall print rate but
    // prevents messages from getting stalled when the buffer isn't seeing much
    // activity
    endsWithNewline = str[count - 1] == '\n';

    blk.header.type = PKT_TYPE_LOG;

    while (count != 0) {
        u32 rem = payload_size - blk_p;
        u32 chunk = MIN(count, rem);

        memcpy(&blk.payload.raw[blk_p], str, chunk);
        str += chunk;
        count -= chunk;
        blk_p += chunk;

        if (blk_p == payload_size || endsWithNewline) {
            gIODevice->fifoWrite(&blk, 1);
            // reset
            bzero(&blk.payload, payload_size);
            blk_p = 0;
        }
    }

    return (void*)1;
}

void Rdb_Printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    _Printf(Rdb_ProutSyncPrintf, NULL, fmt, args);
}

void Rdb_PingHost(void) {
    bzero(&opkt, sizeof(RdbPacket));

    opkt.header.type = PKT_TYPE_PING;
    opkt.payload.raw[0] = 'P';
    opkt.payload.raw[1] = 'I';
    opkt.payload.raw[2] = 'N';
    opkt.payload.raw[3] = 'G';

    gIODevice->fifoWrite(&opkt, 1);
}

void Rdb_ThreadEntry(void* arg) {
    vu8 exit = false;

    osCreateMesgQueue(&sRdbEvtQ, sRdbMesgBuf, ARRAY_COUNT(sRdbMesgBuf));

    osSetEventMesg(OS_EVENT_CPU_BREAK, &sRdbEvtQ, RDB_CPU_BREAK_MSG);
    osSetEventMesg(OS_EVENT_FAULT, &sRdbEvtQ, RDB_FAULT_MSG);
    osSetEventMesg(OS_EVENT_SP_BREAK, &sRdbEvtQ, RDB_SP_BREAK_MSG);
    osSetEventMesg(OS_EVENT_RDB_DATA_ARRIVAL, &sRdbEvtQ, RDB_RD_MSG);

    gRdbActive = true;

    Rdb_Printf("Hello from RDB\n");
    Rdb_PingHost();

    while (!exit) {
        OSEvent evt = 0xFFFFFFFF;

        osRecvMesg(&sRdbEvtQ, (OSMesg*)&evt, OS_MESG_BLOCK);
        if (evt == 0xFFFFFFFF)
            continue;

        gRdbWorking = true;

        switch (evt) {
            case RDB_CPU_BREAK_MSG:
                {
                    
                }
                break;
            case RDB_FAULT_MSG:
                {
                    Rdb_Printf("RDB RECEIVED FAULT MSG\n");
                }
                break;
            case RDB_SP_BREAK_MSG:
                {
                    
                }
                break;
            case RDB_RD_MSG:
                {
                    /*
                     *  read packet from fifo, else the exception handler will notice 
                     *  there is still data and continue to ask us to deal with it
                     */
                    gIODevice->fifoRead(&ipkt, 1);
                    Rdb_Printf("Pong\n");
                }
                break;
        }

        gRdbWorking = false;
    }
}

s32 Rdb_Start(void) {
    if (!IODev_Init()) {
        return 0; // No compatible io device found, cannot start RDB
    }

    osCreateThread(&gRdbThread, 0, Rdb_ThreadEntry, NULL, sRdbThreadStack + sizeof(sRdbThreadStack), OS_PRIORITY_RMON);
    osStartThread(&gRdbThread);

    return 1;
}
