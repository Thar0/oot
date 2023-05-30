#ifndef FLASH_H
#define FLASH_H

#include "ultra64.h"

#define FLASH_PAGE_SIZE         0x80
#define FLASH_PAGES_PER_SECTOR  0x80
#define FLASH_NUM_SECTORS       8
#define FLASH_SECTOR_SIZE       (FLASH_PAGES_PER_SECTOR * FLASH_PAGE_SIZE)
#define FLASH_SIZE              (FLASH_NUM_SECTORS * FLASH_SECTOR_SIZE

#define FLASH_SECTOR_TO_PAGE(sectorNum) ((sectorNum) * FLASH_PAGE_SIZE)
#define FLASH_BYTES_TO_PAGES(nbytes)    (((nbytes) + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE)
#define FLASH_ROUNDUP_SIZE(nbytes)      (FLASH_BYTES_TO_PAGES(nbytes) * FLASH_PAGE_SIZE)

typedef struct {
    s32 type;
    void* addr;
    u32 pageNum;
    u32 nPages;
    OSMesgQueue retQueue;
    OSMesg msg;
} FlashRequest;

// Init

void Flash_Init(void);
s32 Flash_IsInit(void);

// Async Utilities

s32 Flash_IsDone(FlashRequest* req);
s32 Flash_WaitDone(FlashRequest* req);

// Reads

s32 Flash_ReadPages(void* dst, u32 pageNum, u32 nPages);

// Writes

s32 Flash_WritePagesAsync(FlashRequest* req, void* addr, u32 pageNum, u32 nPages);
s32 Flash_WritePagesSync(void* addr, u32 pageNum, u32 nPages);

// Erasures

s32 Flash_EraseSectorSync(u32 pageNum);
s32 Flash_EraseSectorAsync(FlashRequest* req, u32 pageNum);

s32 Flash_EraseAll(void);
s32 Flash_EraseAllAsync(FlashRequest* req);

#endif
