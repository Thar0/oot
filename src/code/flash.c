/**
 * Flash driver implementation. Header for external interface is flash.h
 */
#include "global.h"

#define STATIC static

#define THREAD_ID_FLASH 6
#define THREAD_PRI_FLASH 12

#define FLASH_START_ADDR 0x08000000

// DMA parameters
#define FLASH_LAT 0x5 // Flash PI latency
#define FLASH_PWD 0xC // Flash PI pulse width
#define FLASH_PGS 0xF // Flash PI page size (NOT the same as FLASH_PAGE_SIZE)
#define FLASH_REL 0x2 // Flash PI release time

#define DEVICE_TYPE_FLASH 8

// flash type
#define FLASH_TYPE 0x11118001

// flash vendors
#define FLASH_VERSION_MX_PROTO_A 0x00C20000
#define FLASH_VERSION_MX_A 0x00C20001
#define FLASH_VERSION_MX_C 0x00C2001E
#define FLASH_VERSION_MX_B_AND_D 0x00C2001D
#define FLASH_VERSION_MEI 0x003200F1

// OLD_FLASH is MX_PROTO_A, MX_A and MX_C
#define OLD_FLASH 0
// NEW_FLASH is MX_B_AND_D and MATSUSHITA flash
#define NEW_FLASH 1

#define FLASH_STATUS_ERASE_BUSY 2
#define FLASH_STATUS_ERASE_OK 0
#define FLASH_STATUS_ERASE_ERROR -1

#define FLASH_STATUS_WRITE_BUSY 1
#define FLASH_STATUS_WRITE_OK 0
#define FLASH_STATUS_WRITE_ERROR -1

/**
 * Flash commands
 */
#define FLASH_CMD_REG 0x10000

/* set whole chip erase mode */
#define FLASH_CMD_CHIP_ERASE 0x3C000000
/* set sector erase mode */
#define FLASH_CMD_SECTOR_ERASE 0x4B000000
/* do erasure */
#define FLASH_CMD_EXECUTE_ERASE 0x78000000
/* program selected page */
#define FLASH_CMD_PROGRAM_PAGE 0xA5000000
/* set page program mode */
#define FLASH_CMD_PAGE_PROGRAM 0xB4000000
/* set status mode */
#define FLASH_CMD_STATUS 0xD2000000
/* set silicon id mode */
#define FLASH_CMD_ID 0xE1000000
/* set read mode */
#define FLASH_CMD_READ_ARRAY 0xF0000000

typedef enum { FLASH_REQ_WRITE, FLASH_REQ_ERASE, FLASH_REQ_ERASE_ALL } FlashRequestType;

typedef struct {
    s32 isInit;
    OSPiHandle piHandle;
    u32 flashVersion;
    const char* flashName;
    OSThread thread;
    STACK(stack, 0xA00);
    StackEntry stackInfo;
    OSMesgQueue requestQueue;
    OSMesg requestBuf[4];
} FlashMgr;

FlashMgr sFlashMgr;

STATIC u32 Flash_GetAddr(u32 pageNum) {
    return (sFlashMgr.flashVersion == OLD_FLASH) ? pageNum << 6 : pageNum << 7;
}

STATIC void Flash_ClearStatus(void) {
    // select status mode
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_STATUS);
    // clear status
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, 0);
}

STATIC void Flash_ReadStatus(u8* status) {
    u32 statusTemp;

    // read status
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_STATUS);
    osEPiReadIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, &statusTemp);
    // again
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_STATUS);
    osEPiReadIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, &statusTemp);

    *status = statusTemp;
}

STATIC void Flash_ReadId(u32* type, u32* vendor) {
    union {
        struct {
            u32 type;
            u32 vendor;
            u32 words[2];
        };
        u64 force_structure_alignment[2];
    } dmaBuf;
    OSMesgQueue retQueue;
    OSMesg msg;
    OSIoMesg mb;
    u8 status;

    osCreateMesgQueue(&retQueue, &msg, 1);

    // read status
    Flash_ReadStatus(&status);

    // select silicon id read mode
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_ID);

    // read silicon id using DMA
    mb.hdr.pri = OS_MESG_PRI_NORMAL;
    mb.hdr.retQueue = &retQueue;
    mb.dramAddr = &dmaBuf;
    mb.devAddr = 0;
    mb.size = sizeof(dmaBuf.words);

    osInvalDCache(&dmaBuf, sizeof(dmaBuf));
    osEPiStartDma(&sFlashMgr.piHandle, &mb, OS_READ);
    osRecvMesg(&retQueue, NULL, OS_MESG_BLOCK);

    *type = dmaBuf.type;
    *vendor = dmaBuf.vendor;
}

STATIC s32 Flash_EraseWait(void) {
    u32 status;
    OSTimer timer;
    OSMesgQueue mq;
    OSMesg msg;

    // wait for completion by polling erase-busy flag
    osCreateMesgQueue(&mq, &msg, 1);
    do {
        osSetTimer(&timer, OS_USEC_TO_CYCLES(15000), 0, &mq, &msg);
        osRecvMesg(&mq, &msg, OS_MESG_BLOCK);

        osEPiReadIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, &status);
    } while ((status & FLASH_STATUS_ERASE_BUSY) == FLASH_STATUS_ERASE_BUSY);

    // check erase operation status, clear status
    osEPiReadIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, &status);
    Flash_ClearStatus();

    // check for success
    status &= 0xFF;
    if (status == 8 || status == 0x48 || (status & 8) == 8) {
        return FLASH_STATUS_ERASE_OK;
    } else {
        return FLASH_STATUS_ERASE_ERROR;
    }
}

/**
 * Synchronous erasure of the entire flash memory.
 */
s32 Flash_EraseAll(void) {
    // start chip erase operation
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_CHIP_ERASE);
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_EXECUTE_ERASE);

    // wait for completion
    return Flash_EraseWait();
}

/**
 * Synchronous erasure of the flash sector containing `pageNum`.
 */
s32 Flash_EraseSectorSync(u32 pageNum) {
    // start sector erase operation
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_SECTOR_ERASE | pageNum);
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_EXECUTE_ERASE);

    // wait for completion
    return Flash_EraseWait();
}

STATIC s32 Flash_WriteArray(OSMesgQueue* mq, u32 pageNum) {
    OSTimer timer;
    u32 status;

    if (sFlashMgr.flashVersion == NEW_FLASH) {
        osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_PAGE_PROGRAM);
    }
    // start program page operation
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_PROGRAM_PAGE | pageNum);

    // wait for completion by polling write-busy flag
    do {
        // timer wait
        osSetTimer(&timer, OS_USEC_TO_CYCLES(200), 0, mq, NULL);
        osRecvMesg(mq, NULL, OS_MESG_BLOCK);

        // weird, flash status can be read without FLASH_CMD_STATUS ?
        osEPiReadIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, &status);
    } while ((status & FLASH_STATUS_WRITE_BUSY) == FLASH_STATUS_WRITE_BUSY);

    // check program operation status, clear status
    osEPiReadIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, &status);
    Flash_ClearStatus();

    status &= 0xFF;
    if (status == 4 || status == 0x44 || (status & 4) == 4) {
        return FLASH_STATUS_WRITE_OK;
    } else {
        return FLASH_STATUS_WRITE_ERROR;
    }
}

STATIC s32 Flash_WriteBuffer(OSMesgQueue* mq, void* addr) {
    OSIoMesg mb;

    // select page program mode
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_PAGE_PROGRAM);

    // DMA 128-byte page
    mb.hdr.pri = OS_MESG_PRI_NORMAL;
    mb.hdr.retQueue = mq;
    mb.dramAddr = addr;
    mb.devAddr = 0;
    mb.size = FLASH_PAGE_SIZE;

    osEPiStartDma(&sFlashMgr.piHandle, &mb, OS_WRITE);
    return osRecvMesg(mq, NULL, OS_MESG_BLOCK);
}

STATIC s32 Flash_WriteRequiresErase(void* curFlashData, void* newFlashData, u32 nPages) {
    u32 size;
    u32 i;

    for (i = 0; i < nPages * FLASH_PAGE_SIZE; i += 4) {
        if ((*(u32*)curFlashData & *(u32*)newFlashData) != *(u32*)newFlashData) {
            // trying to flip 0 -> 1, needs erasure
            return true;
        }
    }
    // no flips from 0 -> 1, can skip erasure
    return false;
}

STATIC s32 Flash_TryWrite(void* src, u32 pageNum, u32 nPages, s32 erase) {
#define RETRY_COUNT 3
    OSMesgQueue mq;
    OSMesg msg;
    s32 ret;
    s32 i;
    u32 j;

    osCreateMesgQueue(&mq, &msg, 1);
    osWritebackDCache(src, nPages * FLASH_PAGE_SIZE);

    for (i = 0; i < RETRY_COUNT; i++) {
        if (erase) {
            ret = Flash_EraseSectorSync(pageNum);
            if (ret != 0) {
                continue;
            }
        }

        for (j = 0; j < nPages; j++) {
            Flash_WriteBuffer(&mq, (u8*)src + j * FLASH_PAGE_SIZE);

            ret = Flash_WriteArray(&mq, pageNum + j);
            if (ret != 0) {
                break;
            }
        }
        if (j == nPages) {
            return 0;
        }
    }
    return ret;
}

s32 Flash_ReadPages(void* dst, u32 pageNum, u32 nPages);

STATIC s32 Flash_WritePagesImpl(void* src, u32 pageNum, u32 nPages) {
    void* oldData;
    size_t nBytes = nPages * FLASH_PAGE_SIZE;
    s32 ret;

    if (!sFlashMgr.isInit) {
        return -1;
    }

    // TODO currently we only handle writes that begin at the start of a sector and are fully contained in that
    // same sector. This is sufficient for save games as each can occupy a single sector. This avoids complications
    // with erasures.
    assert((pageNum % FLASH_PAGE_SIZE) == 0);
    assert(nPages <= FLASH_PAGES_PER_SECTOR);

    // try and allocate a buffer to read the old data into
    oldData = DebugArena_Malloc(nBytes);
    if (oldData == NULL) {
        // failed, go ahead and write the new data assuming we need to erase
        return Flash_TryWrite(src, pageNum, nPages, true);
    }

    // read old data into the buffer
    Flash_ReadPages(oldData, pageNum, nPages);

    // if the old data is the same as the new data, we're done
    if (bcmp(oldData, src, nBytes) == 0) {
        ret = 0;
        goto done;
    }

    // write new data, possibly with erasure
    ret = Flash_TryWrite(src, pageNum, nPages, Flash_WriteRequiresErase(oldData, src, nPages));
    if (ret != 0) {
        goto done;
    }

    // read back the data
    Flash_ReadPages(oldData, pageNum, nPages);

    // check it
    ret = (bcmp(oldData, src, nBytes) == 0) ? 0 : -1;

done:
    DebugArena_Free(oldData);
    return ret;
}

STATIC void Flash_ThreadEntry(void* arg) {
    while (true) {
        UNUSED s32 ret;
        FlashRequest* req;

        // wait for a request to arrive
        osRecvMesg(&sFlashMgr.requestQueue, (OSMesg*)&req, OS_MESG_BLOCK);

        // got a request
        switch (req->type) {
            case FLASH_REQ_WRITE:
                ret = Flash_WritePagesImpl(req->addr, req->pageNum, req->nPages);
                break;
            case FLASH_REQ_ERASE:
                ret = Flash_EraseSectorSync(req->pageNum);
                break;
            case FLASH_REQ_ERASE_ALL:
                ret = Flash_EraseAll();
                break;
        }

        // notify done
        osSendMesg(&req->retQueue, NULL, OS_MESG_BLOCK);
    }
}

/**
 * Synchronous read of `nPages` starting at `pageNum`.
 */
s32 Flash_ReadPages(void* dst, u32 pageNum, u32 nPages) {
    OSMesgQueue mq;
    OSMesg msg;
    OSIoMesg mb;
    u32 dummy;
    u32 endPage;
    u32 pageChunk;

    if (!sFlashMgr.isInit) {
        return -1;
    }
    if (nPages == 0) {
        return 0;
    }

    osCreateMesgQueue(&mq, &msg, 1);
    osInvalDCache(dst, nPages * FLASH_PAGE_SIZE);

    // select read array mode
    osEPiWriteIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress | FLASH_CMD_REG, FLASH_CMD_READ_ARRAY);
    // dummy read?
    osEPiReadIo(&sFlashMgr.piHandle, sFlashMgr.piHandle.baseAddress, &dummy);

    // DMA requested pages
    mb.hdr.pri = OS_MESG_PRI_NORMAL;
    mb.hdr.retQueue = &mq;
    mb.dramAddr = dst;

    endPage = pageNum + nPages - 1;

    if ((endPage & 0xF00) != (pageNum & 0xF00)) {
        pageChunk = 256 - (pageNum & 0xFF);
        nPages -= pageChunk;
        mb.size = pageChunk * FLASH_PAGE_SIZE;
        mb.devAddr = Flash_GetAddr(pageNum);
        osEPiStartDma(&sFlashMgr.piHandle, &mb, OS_READ);
        osRecvMesg(&mq, NULL, OS_MESG_BLOCK);
        pageNum = (pageNum + 256) & 0xF00;
        mb.dramAddr = (u8*)mb.dramAddr + mb.size;
    }

    while (nPages > 256) {
        pageChunk = 256;
        nPages -= 256;
        mb.size = pageChunk * FLASH_PAGE_SIZE;
        mb.devAddr = Flash_GetAddr(pageNum);
        osEPiStartDma(&sFlashMgr.piHandle, &mb, OS_READ);
        osRecvMesg(&mq, NULL, OS_MESG_BLOCK);
        pageNum += 256;
        mb.dramAddr = (u8*)mb.dramAddr + mb.size;
    }

    mb.size = nPages * FLASH_PAGE_SIZE;
    mb.devAddr = Flash_GetAddr(pageNum);

    osEPiStartDma(&sFlashMgr.piHandle, &mb, OS_READ);
    return osRecvMesg(&mq, NULL, OS_MESG_BLOCK);
}

/**
 * Check if asynchronous flash task `req` has completed.
 */
s32 Flash_IsDone(FlashRequest* req) {
    return !MQ_IS_EMPTY(&req->retQueue);
}

/**
 * Block the current thread until flash task `req` has completed.
 */
s32 Flash_WaitDone(FlashRequest* req) {
    return osRecvMesg(&req->retQueue, NULL, OS_MESG_BLOCK);
}

/**
 * Asynchronously write `nPages` of `addr` to `pageNum`.
 */
s32 Flash_WritePagesAsync(FlashRequest* req, void* addr, u32 pageNum, u32 nPages) {
    if (!sFlashMgr.isInit) {
        return -1;
    }
    if (nPages == 0) {
        return 0;
    }

    req->type = FLASH_REQ_WRITE;
    req->addr = addr;
    req->pageNum = pageNum;
    req->nPages = nPages;

    osCreateMesgQueue(&req->retQueue, &req->msg, 1);
    return osSendMesg(&sFlashMgr.requestQueue, req, OS_MESG_BLOCK);
}

/**
 * Synchronously write `nPages` of `addr` to `pageNum`.
 */
s32 Flash_WritePagesSync(void* addr, u32 pageNum, u32 nPages) {
    FlashRequest req;
    s32 ret;

    if (nPages == 0) {
        return 0;
    }
    ret = Flash_WritePagesAsync(&req, addr, pageNum, nPages);
    if (ret != 0) {
        return ret;
    }
    return Flash_WaitDone(&req);
}

/**
 * Asynchronously erase the sector containing `pageNum`.
 */
s32 Flash_EraseSectorAsync(FlashRequest* req, u32 pageNum) {
    if (!sFlashMgr.isInit) {
        return -1;
    }

    req->type = FLASH_REQ_ERASE;
    req->pageNum = pageNum;

    osCreateMesgQueue(&req->retQueue, &req->msg, 1);
    return osSendMesg(&sFlashMgr.requestQueue, req, OS_MESG_BLOCK);
}

/**
 * Asynchronously erase the entire flash memory.
 */
s32 Flash_EraseAllAsync(FlashRequest* req) {
    if (!sFlashMgr.isInit) {
        return -1;
    }

    req->type = FLASH_REQ_ERASE_ALL;

    osCreateMesgQueue(&req->retQueue, &req->msg, 1);
    return osSendMesg(&sFlashMgr.requestQueue, req, OS_MESG_BLOCK);
}

/**
 * Returns true if the flash driver has been initialized.
 */
s32 Flash_IsInit(void) {
    return sFlashMgr.isInit;
}

/**
 * Initializes the flash driver. This must be done before anything else.
 */
void Flash_Init(void) {
    u32 type;
    u32 vendor;

    if (sFlashMgr.isInit) {
        return;
    }

    sFlashMgr.piHandle.type = DEVICE_TYPE_FLASH;
    sFlashMgr.piHandle.baseAddress = PHYS_TO_K1(FLASH_START_ADDR);
    sFlashMgr.piHandle.latency = FLASH_LAT;
    sFlashMgr.piHandle.pulse = FLASH_PWD;
    sFlashMgr.piHandle.pageSize = FLASH_PGS;
    sFlashMgr.piHandle.relDuration = FLASH_REL;
    sFlashMgr.piHandle.domain = PI_DOMAIN2;
    sFlashMgr.piHandle.speed = 0;

    bzero(&sFlashMgr.piHandle.transferInfo, sizeof(__OSTranxInfo));

    { // osEPiLinkHandle(&sFlashMgr.piHandle);
        OSPiHandle* handle = &sFlashMgr.piHandle;

        register s32 saveMask = __osDisableInt();

        handle->next = __osPiTable;
        __osPiTable = handle;

        __osRestoreInt(saveMask);
    }

    Flash_ReadId(&type, &vendor);

    if (type != FLASH_TYPE) {
        goto flash_bad;
    }

    switch (vendor) {
        case FLASH_VERSION_MX_PROTO_A:
            sFlashMgr.flashVersion = OLD_FLASH;
            sFlashMgr.flashName = "MX PROTO A";
            break;
        case FLASH_VERSION_MX_A:
            sFlashMgr.flashVersion = OLD_FLASH;
            sFlashMgr.flashName = "MX A";
            break;
        case FLASH_VERSION_MX_C:
            sFlashMgr.flashVersion = OLD_FLASH;
            sFlashMgr.flashName = "MX C";
            break;
        case FLASH_VERSION_MEI:
            sFlashMgr.flashVersion = NEW_FLASH;
            sFlashMgr.flashName = "MEI (D)";
            break;
        case FLASH_VERSION_MX_B_AND_D:
            sFlashMgr.flashVersion = NEW_FLASH;
            sFlashMgr.flashName = "B or D";
            break;
        default:
        flash_bad : {
            char version[22];
            sprintf(version, "0x%80X 0x%08X", type, vendor);
            Fault_AddHungupAndCrashImpl("UNKNOWN FLASH", version);
        } break;
    }

    // osSyncPrintfUnused("[FLASH] type=0x%08X vendor=0x%08X name=\"%s\"\n", type, vendor, sFlashMgr.flashName);

    sFlashMgr.isInit = true;
    osCreateMesgQueue(&sFlashMgr.requestQueue, sFlashMgr.requestBuf, ARRAY_COUNT(sFlashMgr.requestBuf));

    StackCheck_Init(&sFlashMgr.stackInfo, sFlashMgr.stack, STACK_TOP(sFlashMgr.stack), 0, 0x100, "flash");
    osCreateThread(&sFlashMgr.thread, THREAD_ID_FLASH, Flash_ThreadEntry, NULL, STACK_TOP(sFlashMgr.stack),
                   THREAD_PRI_FLASH);
    osStartThread(&sFlashMgr.thread);
}
