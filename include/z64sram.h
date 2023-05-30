#ifndef Z64SRAM_H
#define Z64SRAM_H

#include "assert.h"
#include "ultra64/ultratypes.h"
#include "flash.h"

typedef struct {
    u8 sound;
    u8 zTarget;
    u8 language;
    char magic[9];
} SaveOptions;
static_assert(sizeof(SaveOptions) <= FLASH_SECTOR_SIZE, "SaveOptions must fit in one flash sector");

typedef struct {
    union {
        SaveOptions options;
        // we need a dma-aligned buffer that is a multiple of the page size so that nothing is overwritten
        u8 optionsRawData[FLASH_ROUNDUP_SIZE(sizeof(SaveOptions))];
        u64 optionsDmaAlign;
    };
    u8* writeBuf;
    u8* readBuf;
    FlashRequest flashReqMain;
    FlashRequest flashReqBackup;
} SramContext;

#define SRAM_SIZE 0x8000
#define SRAM_HEADER_SIZE 0x10

typedef enum {
    /* 0x00 */ SRAM_HEADER_SOUND,
    /* 0x01 */ SRAM_HEADER_ZTARGET,
    /* 0x02 */ SRAM_HEADER_LANGUAGE,
    /* 0x03 */ SRAM_HEADER_MAGIC // must be the value of `sZeldaMagic` for save to be considered valid
} SramHeaderField;

#endif
