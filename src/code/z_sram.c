#include "global.h"
#include "terminal.h"

static_assert(sizeof(Save) <= FLASH_SECTOR_SIZE, "Save must fit in one flash sector");

static SaveOptions sDefaultOptions = {
    0,
    0,
    0,
    { 0x98, 0x09, 0x10, 0x21, 'Z', 'E', 'L', 'D', 'A' },
};

#define SAVE_SIGN(save)                         \
    do {                                        \
        (save)->info.playerData.newf[0] = 'Z';  \
        (save)->info.playerData.newf[1] = 'E';  \
        (save)->info.playerData.newf[2] = 'L';  \
        (save)->info.playerData.newf[3] = 'D';  \
        (save)->info.playerData.newf[4] = 'A';  \
        (save)->info.playerData.newf[5] = 'Z';  \
    } while (0)

#define SAVE_CHECK_SIGNATURE(save)                                                          \
   (((save)->info.playerData.newf[0] == 'Z') && ((save)->info.playerData.newf[1] == 'E') && \
    ((save)->info.playerData.newf[2] == 'L') && ((save)->info.playerData.newf[3] == 'D') && \
    ((save)->info.playerData.newf[4] == 'A') && ((save)->info.playerData.newf[5] == 'Z'))   \

#define SAVE_SIGNATURE_INIT  { 'Z', 'E', 'L', 'D', 'A', 'Z' }
#define SAVE_SIGNATURE_EMPTY { '\0', '\0', '\0', '\0', '\0', '\0' }

static SavePlayerData sNewSavePlayerData = {
    SAVE_SIGNATURE_EMPTY,                               // newf
    0,                                                  // deaths
    { 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E }, // playerName
    0,                                                  // n64ddFlag
    0x30,                                               // healthCapacity
    0x30,                                               // defense
    0,                                                  // magicLevel
    MAGIC_NORMAL_METER,                                 // magic
    0,                                                  // rupees
    0,                                                  // swordHealth
    0,                                                  // naviTimer
    false,                                              // isMagicAcquired
    0,                                                  // unk_1F
    false,                                              // isDoubleMagicAcquired
    false,                                              // isDoubleDefenseAcquired
    0,                                                  // bgsFlag
    0,                                                  // ocarinaGameRoundNum
    {
        { ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE }, // buttonItems
        { SLOT_NONE, SLOT_NONE, SLOT_NONE },            // cButtonSlots
        0,                                              // equipment
    },                                                  // childEquips
    {
        { ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE }, // buttonItems
        { SLOT_NONE, SLOT_NONE, SLOT_NONE },            // cButtonSlots
        0,                                              // equipment
    },                                                  // adultEquips
    0,                                                  // unk_38
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },       // unk_3C
    SCENE_LINKS_HOUSE,                                  // savedSceneId
};

static ItemEquips sNewSaveEquips = {
    { ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE }, // buttonItems
    { SLOT_NONE, SLOT_NONE, SLOT_NONE },            // cButtonSlots
    0x1100,                                         // equipment
};

static Inventory sNewSaveInventory = {
    // items
    {
        ITEM_NONE, // SLOT_DEKU_STICK
        ITEM_NONE, // SLOT_DEKU_NUT
        ITEM_NONE, // SLOT_BOMB
        ITEM_NONE, // SLOT_BOW
        ITEM_NONE, // SLOT_ARROW_FIRE
        ITEM_NONE, // SLOT_DINS_FIRE
        ITEM_NONE, // SLOT_SLINGSHOT
        ITEM_NONE, // SLOT_OCARINA
        ITEM_NONE, // SLOT_BOMBCHU
        ITEM_NONE, // SLOT_HOOKSHOT
        ITEM_NONE, // SLOT_ARROW_ICE
        ITEM_NONE, // SLOT_FARORES_WIND
        ITEM_NONE, // SLOT_BOOMERANG
        ITEM_NONE, // SLOT_LENS_OF_TRUTH
        ITEM_NONE, // SLOT_MAGIC_BEAN
        ITEM_NONE, // SLOT_HAMMER
        ITEM_NONE, // SLOT_ARROW_LIGHT
        ITEM_NONE, // SLOT_NAYRUS_LOVE
        ITEM_NONE, // SLOT_BOTTLE_1
        ITEM_NONE, // SLOT_BOTTLE_2
        ITEM_NONE, // SLOT_BOTTLE_3
        ITEM_NONE, // SLOT_BOTTLE_4
        ITEM_NONE, // SLOT_TRADE_ADULT
        ITEM_NONE, // SLOT_TRADE_CHILD
    },
    // ammo
    {
        0, // SLOT_DEKU_STICK
        0, // SLOT_DEKU_NUT
        0, // SLOT_BOMB
        0, // SLOT_BOW
        0, // SLOT_ARROW_FIRE
        0, // SLOT_DINS_FIRE
        0, // SLOT_SLINGSHOT
        0, // SLOT_OCARINA
        0, // SLOT_BOMBCHU
        0, // SLOT_HOOKSHOT
        0, // SLOT_ARROW_ICE
        0, // SLOT_FARORES_WIND
        0, // SLOT_BOOMERANG
        0, // SLOT_LENS_OF_TRUTH
        0, // SLOT_MAGIC_BEAN
        0, // SLOT_HAMMER
    },
    // equipment
    (((1 << EQUIP_INV_TUNIC_KOKIRI) << (EQUIP_TYPE_TUNIC * 4)) |
     ((1 << EQUIP_INV_BOOTS_KOKIRI) << (EQUIP_TYPE_BOOTS * 4))),
    0,                                                              // upgrades
    0,                                                              // questItems
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // dungeonItems
    {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    }, // dungeonKeys
    0, // defenseHearts
    0, // gsTokens
};

/**
 *  Initialize new save.
 *  This save has an empty inventory with 3 hearts and single magic.
 */
void Sram_InitNewSave(Save* save, s32 fileNum) {
    bzero(&save->info, sizeof(SaveInfo));
    save->totalDays = 0;
    save->bgsDayCount = 0;

    save->info.playerData = sNewSavePlayerData;
    save->info.equips = sNewSaveEquips;
    save->info.inventory = sNewSaveInventory;

    save->info.checksum = 0;
    save->info.horseData.sceneId = SCENE_HYRULE_FIELD;
    save->info.horseData.pos.x = -1840;
    save->info.horseData.pos.y = 72;
    save->info.horseData.pos.z = 5497;
    save->info.horseData.angle = -0x6AD9;
    save->info.playerData.magicLevel = 0;
    save->info.infTable[INFTABLE_1DX_INDEX] = 1;
    save->info.sceneFlags[SCENE_WATER_TEMPLE].swch = 0x40000000;
}

static SavePlayerData sDebugSavePlayerData = {
    SAVE_SIGNATURE_INIT,                                // newf
    0,                                                  // deaths
    { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E }, // playerName ( "LINK" )
    0,                                                  // n64ddFlag
    0xE0,                                               // healthCapacity
    0xE0,                                               // health
    0,                                                  // magicLevel
    MAGIC_NORMAL_METER,                                 // magic
    150,                                                // rupees
    8,                                                  // swordHealth
    0,                                                  // naviTimer
    true,                                               // isMagicAcquired
    0,                                                  // unk_1F
    false,                                              // isDoubleMagicAcquired
    false,                                              // isDoubleDefenseAcquired
    0,                                                  // bgsFlag
    0,                                                  // ocarinaGameRoundNum
    {
        { ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE }, // buttonItems
        { SLOT_NONE, SLOT_NONE, SLOT_NONE },            // cButtonSlots
        0,                                              // equipment
    },                                                  // childEquips
    {
        { ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE }, // buttonItems
        { SLOT_NONE, SLOT_NONE, SLOT_NONE },            // cButtonSlots
        0,                                              // equipment
    },                                                  // adultEquips
    0,                                                  // unk_38
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },       // unk_3C
    SCENE_HYRULE_FIELD,                                 // savedSceneId
};

static ItemEquips sDebugSaveEquips = {
    { ITEM_SWORD_MASTER, ITEM_BOW, ITEM_BOMB, ITEM_OCARINA_FAIRY }, // buttonItems
    { SLOT_BOW, SLOT_BOMB, SLOT_OCARINA },                          // cButtonSlots
    // equipment
    (EQUIP_VALUE_SWORD_MASTER << (EQUIP_TYPE_SWORD * 4)) | (EQUIP_VALUE_SHIELD_HYLIAN << (EQUIP_TYPE_SHIELD * 4)) |
        (EQUIP_VALUE_TUNIC_KOKIRI << (EQUIP_TYPE_TUNIC * 4)) | (EQUIP_VALUE_BOOTS_KOKIRI << (EQUIP_TYPE_BOOTS * 4)),
};

static Inventory sDebugSaveInventory = {
    // items
    {
        ITEM_DEKU_STICK,          // SLOT_DEKU_STICK
        ITEM_DEKU_NUT,            // SLOT_DEKU_NUT
        ITEM_BOMB,                // SLOT_BOMB
        ITEM_BOW,                 // SLOT_BOW
        ITEM_ARROW_FIRE,          // SLOT_ARROW_FIRE
        ITEM_DINS_FIRE,           // SLOT_DINS_FIRE
        ITEM_SLINGSHOT,           // SLOT_SLINGSHOT
        ITEM_OCARINA_FAIRY,       // SLOT_OCARINA
        ITEM_BOMBCHU,             // SLOT_BOMBCHU
        ITEM_HOOKSHOT,            // SLOT_HOOKSHOT
        ITEM_ARROW_ICE,           // SLOT_ARROW_ICE
        ITEM_FARORES_WIND,        // SLOT_FARORES_WIND
        ITEM_BOOMERANG,           // SLOT_BOOMERANG
        ITEM_LENS_OF_TRUTH,       // SLOT_LENS_OF_TRUTH
        ITEM_MAGIC_BEAN,          // SLOT_MAGIC_BEAN
        ITEM_HAMMER,              // SLOT_HAMMER
        ITEM_ARROW_LIGHT,         // SLOT_ARROW_LIGHT
        ITEM_NAYRUS_LOVE,         // SLOT_NAYRUS_LOVE
        ITEM_BOTTLE_EMPTY,        // SLOT_BOTTLE_1
        ITEM_BOTTLE_POTION_RED,   // SLOT_BOTTLE_2
        ITEM_BOTTLE_POTION_GREEN, // SLOT_BOTTLE_3
        ITEM_BOTTLE_POTION_BLUE,  // SLOT_BOTTLE_4
        ITEM_POCKET_EGG,          // SLOT_TRADE_ADULT
        ITEM_WEIRD_EGG,           // SLOT_TRADE_CHILD
    },
    // ammo
    {
        50, // SLOT_DEKU_STICK
        50, // SLOT_DEKU_NUT
        10, // SLOT_BOMB
        30, // SLOT_BOW
        1,  // SLOT_ARROW_FIRE
        1,  // SLOT_DINS_FIRE
        30, // SLOT_SLINGSHOT
        1,  // SLOT_OCARINA
        50, // SLOT_BOMBCHU
        1,  // SLOT_HOOKSHOT
        1,  // SLOT_ARROW_ICE
        1,  // SLOT_FARORES_WIND
        1,  // SLOT_BOOMERANG
        1,  // SLOT_LENS_OF_TRUTH
        1,  // SLOT_MAGIC_BEAN
        1   // SLOT_HAMMER
    },
    // equipment
    ((((1 << EQUIP_INV_SWORD_KOKIRI) << (EQUIP_TYPE_SWORD * 4)) |
      ((1 << EQUIP_INV_SWORD_MASTER) << (EQUIP_TYPE_SWORD * 4)) |
      ((1 << EQUIP_INV_SWORD_BIGGORON) << (EQUIP_TYPE_SWORD * 4))) |
     (((1 << EQUIP_INV_SHIELD_DEKU) << (EQUIP_TYPE_SHIELD * 4)) |
      ((1 << EQUIP_INV_SHIELD_HYLIAN) << (EQUIP_TYPE_SHIELD * 4)) |
      ((1 << EQUIP_INV_SHIELD_MIRROR) << (EQUIP_TYPE_SHIELD * 4))) |
     (((1 << EQUIP_INV_TUNIC_KOKIRI) << (EQUIP_TYPE_TUNIC * 4)) |
      ((1 << EQUIP_INV_TUNIC_GORON) << (EQUIP_TYPE_TUNIC * 4)) |
      ((1 << EQUIP_INV_TUNIC_ZORA) << (EQUIP_TYPE_TUNIC * 4))) |
     (((1 << EQUIP_INV_BOOTS_KOKIRI) << (EQUIP_TYPE_BOOTS * 4)) |
      ((1 << EQUIP_INV_BOOTS_IRON) << (EQUIP_TYPE_BOOTS * 4)) |
      ((1 << EQUIP_INV_BOOTS_HOVER) << (EQUIP_TYPE_BOOTS * 4)))),
    0x125249,                                                       // upgrades
    0x1E3FFFF,                                                      // questItems
    { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // dungeonItems
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },    // dungeonKeys
    0,                                                              // defenseHearts
    0,                                                              // gsTokens
};

/**
 *  Initialize debug save. This is also used on the Title Screen
 *  This save has a mostly full inventory with 10 hearts and single magic.
 *
 *  Some noteable flags that are set:
 *  Showed Mido sword/shield, met Deku Tree, Deku Tree mouth opened,
 *  used blue warp in Gohmas room, Zelda fled castle, light arrow cutscene watched,
 *  and set water level in Water Temple to lowest level.
 */
void Sram_InitDebugSave(Save* save, s32 fileNum) {
    // this function should really accept the Save type directly, but needs the save structures merged first

    bzero(&save->info, sizeof(SaveInfo));
    save->totalDays = 0;
    save->bgsDayCount = 0;

    save->info.playerData = sDebugSavePlayerData;
    save->info.equips = sDebugSaveEquips;
    save->info.inventory = sDebugSaveInventory;

    save->info.checksum = 0;
    save->info.horseData.sceneId = SCENE_HYRULE_FIELD;
    save->info.horseData.pos.x = -1840;
    save->info.horseData.pos.y = 72;
    save->info.horseData.pos.z = 5497;
    save->info.horseData.angle = -0x6AD9;
    save->info.infTable[0] |= 0x5009;
    save->info.eventChkInf[0] |= 0x123F;
    SET_EVENTCHKINF(EVENTCHKINF_80);
    SET_EVENTCHKINF(EVENTCHKINF_C4);

    if (LINK_AGE_IN_YEARS == YEARS_CHILD) {
        save->info.equips.buttonItems[0] = ITEM_SWORD_KOKIRI;
        Inventory_ChangeEquipment(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI);
        if (fileNum == 0xFF) {
            save->info.equips.buttonItems[1] = ITEM_SLINGSHOT;
            save->info.equips.cButtonSlots[0] = SLOT_SLINGSHOT;
            Inventory_ChangeEquipment(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_DEKU);
        }
    }

    save->entranceIndex = ENTR_HYRULE_FIELD_0;
    save->info.playerData.magicLevel = 0;
    save->info.sceneFlags[SCENE_WATER_TEMPLE].swch = 0x40000000;
}

static s16 sDungeonEntrances[] = {
    ENTR_DEKU_TREE_0,                      // SCENE_DEKU_TREE
    ENTR_DODONGOS_CAVERN_0,                // SCENE_DODONGOS_CAVERN
    ENTR_JABU_JABU_0,                      // SCENE_JABU_JABU
    ENTR_FOREST_TEMPLE_0,                  // SCENE_FOREST_TEMPLE
    ENTR_FIRE_TEMPLE_0,                    // SCENE_FIRE_TEMPLE
    ENTR_WATER_TEMPLE_0,                   // SCENE_WATER_TEMPLE
    ENTR_SPIRIT_TEMPLE_0,                  // SCENE_SPIRIT_TEMPLE
    ENTR_SHADOW_TEMPLE_0,                  // SCENE_SHADOW_TEMPLE
    ENTR_BOTTOM_OF_THE_WELL_0,             // SCENE_BOTTOM_OF_THE_WELL
    ENTR_ICE_CAVERN_0,                     // SCENE_ICE_CAVERN
    ENTR_GANONS_TOWER_0,                   // SCENE_GANONS_TOWER
    ENTR_GERUDO_TRAINING_GROUND_0,         // SCENE_GERUDO_TRAINING_GROUND
    ENTR_THIEVES_HIDEOUT_0,                // SCENE_THIEVES_HIDEOUT
    ENTR_INSIDE_GANONS_CASTLE_0,           // SCENE_INSIDE_GANONS_CASTLE
    ENTR_GANONS_TOWER_COLLAPSE_INTERIOR_0, // SCENE_GANONS_TOWER_COLLAPSE_INTERIOR
    ENTR_INSIDE_GANONS_CASTLE_COLLAPSE_0,  // SCENE_INSIDE_GANONS_CASTLE_COLLAPSE
};

/**
 *  Copy save currently on the buffer to Save Context and complete various tasks to open the save.
 *  This includes:
 *  - Set proper entrance depending on where the game was saved
 *  - If health is less than 3 hearts, give 3 hearts
 *  - If either scarecrow song is set, copy them from save context to the proper location
 *  - Handle a case where the player saved and quit after zelda cutscene but didnt get the song
 *  - Give and equip master sword if player is adult and doesn't have master sword
 *  - Revert any trade items that spoil
 */
void Sram_OpenSave(FileSelectState* fileSelect) {
    u32 i;
    u32 j;

    gSaveContext.fileNum = fileSelect->buttonIndex;
    Flash_ReadPages(fileSelect->sramCtx.readBuf, FLASH_SECTOR_TO_PAGE(gSaveContext.fileNum),
                    FLASH_BYTES_TO_PAGES(sizeof(Save)));
    bcopy(fileSelect->sramCtx.readBuf, &gSaveContext.save, sizeof(Save));

    switch (gSaveContext.save.info.playerData.savedSceneId) {
        case SCENE_DEKU_TREE:
        case SCENE_DODONGOS_CAVERN:
        case SCENE_JABU_JABU:
        case SCENE_FOREST_TEMPLE:
        case SCENE_FIRE_TEMPLE:
        case SCENE_WATER_TEMPLE:
        case SCENE_SPIRIT_TEMPLE:
        case SCENE_SHADOW_TEMPLE:
        case SCENE_BOTTOM_OF_THE_WELL:
        case SCENE_ICE_CAVERN:
        case SCENE_GANONS_TOWER:
        case SCENE_GERUDO_TRAINING_GROUND:
        case SCENE_THIEVES_HIDEOUT:
        case SCENE_INSIDE_GANONS_CASTLE:
            gSaveContext.save.entranceIndex = sDungeonEntrances[gSaveContext.save.info.playerData.savedSceneId];
            break;

        case SCENE_DEKU_TREE_BOSS:
            gSaveContext.save.entranceIndex = ENTR_DEKU_TREE_0;
            break;

        case SCENE_DODONGOS_CAVERN_BOSS:
            gSaveContext.save.entranceIndex = ENTR_DODONGOS_CAVERN_0;
            break;

        case SCENE_JABU_JABU_BOSS:
            gSaveContext.save.entranceIndex = ENTR_JABU_JABU_0;
            break;

        case SCENE_FOREST_TEMPLE_BOSS:
            gSaveContext.save.entranceIndex = ENTR_FOREST_TEMPLE_0;
            break;

        case SCENE_FIRE_TEMPLE_BOSS:
            gSaveContext.save.entranceIndex = ENTR_FIRE_TEMPLE_0;
            break;

        case SCENE_WATER_TEMPLE_BOSS:
            gSaveContext.save.entranceIndex = ENTR_WATER_TEMPLE_0;
            break;

        case SCENE_SPIRIT_TEMPLE_BOSS:
            gSaveContext.save.entranceIndex = ENTR_SPIRIT_TEMPLE_0;
            break;

        case SCENE_SHADOW_TEMPLE_BOSS:
            gSaveContext.save.entranceIndex = ENTR_SHADOW_TEMPLE_0;
            break;

        case SCENE_GANONS_TOWER_COLLAPSE_INTERIOR:
        case SCENE_INSIDE_GANONS_CASTLE_COLLAPSE:
        case SCENE_GANONDORF_BOSS:
        case SCENE_GANONS_TOWER_COLLAPSE_EXTERIOR:
        case SCENE_GANON_BOSS:
            gSaveContext.save.entranceIndex = ENTR_GANONS_TOWER_0;
            break;

        default:
            if (gSaveContext.save.info.playerData.savedSceneId != SCENE_LINKS_HOUSE) {
                gSaveContext.save.entranceIndex =
                    (LINK_AGE_IN_YEARS == YEARS_CHILD) ? ENTR_LINKS_HOUSE_0 : ENTR_TEMPLE_OF_TIME_7;
            } else {
                gSaveContext.save.entranceIndex = ENTR_LINKS_HOUSE_0;
            }
            break;
    }

    if (gSaveContext.save.info.playerData.health < 0x30) {
        gSaveContext.save.info.playerData.health = 0x30;
    }

    if (gSaveContext.save.info.scarecrowLongSongSet) {
        MemCpy(gScarecrowLongSongPtr, gSaveContext.save.info.scarecrowLongSong,
               sizeof(gSaveContext.save.info.scarecrowLongSong));
    }

    if (gSaveContext.save.info.scarecrowSpawnSongSet) {
        MemCpy(gScarecrowSpawnSongPtr, gSaveContext.save.info.scarecrowSpawnSong,
               sizeof(gSaveContext.save.info.scarecrowSpawnSong));
    }

    // if zelda cutscene has been watched but lullaby was not obtained, restore cutscene and take away letter
    if (GET_EVENTCHKINF(EVENTCHKINF_40) && !CHECK_QUEST_ITEM(QUEST_SONG_LULLABY)) {
        gSaveContext.save.info.eventChkInf[EVENTCHKINF_40_INDEX] &= ~EVENTCHKINF_40_MASK;

        INV_CONTENT(ITEM_ZELDAS_LETTER) = ITEM_CHICKEN;

        for (j = 1; j < 4; j++) {
            if (gSaveContext.save.info.equips.buttonItems[j] == ITEM_ZELDAS_LETTER) {
                gSaveContext.save.info.equips.buttonItems[j] = ITEM_CHICKEN;
            }
        }
    }

    if (LINK_AGE_IN_YEARS == YEARS_ADULT && !CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER)) {
        gSaveContext.save.info.inventory.equipment |= OWNED_EQUIP_FLAG(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER);
        gSaveContext.save.info.equips.buttonItems[0] = ITEM_SWORD_MASTER;
        gSaveContext.save.info.equips.equipment &= ~(0xF << (EQUIP_TYPE_SWORD * 4));
        gSaveContext.save.info.equips.equipment |= EQUIP_VALUE_SWORD_MASTER << (EQUIP_TYPE_SWORD * 4);
    }

    for (i = 0; i < ARRAY_COUNT(gSpoilingItems); i++) {
        if (INV_CONTENT(ITEM_TRADE_ADULT) == gSpoilingItems[i]) {
            INV_CONTENT(gSpoilingItemReverts[i]) = gSpoilingItemReverts[i];

            for (j = 1; j < 4; j++) {
                if (gSaveContext.save.info.equips.buttonItems[j] == gSpoilingItems[i]) {
                    gSaveContext.save.info.equips.buttonItems[j] = gSpoilingItemReverts[i];
                }
            }
        }
    }

    gSaveContext.save.info.playerData.magicLevel = 0;
}

u16 Sram_CalcChecksum(Save* save) {
    u32 offset;
    u16 checksum = 0;
    u16* ptr = (u16*)save;

    save->info.checksum = 0;
    for (offset = 0; offset < sizeof(Save); offset += sizeof(u16)) {
        checksum += *ptr++;
    }
    return checksum;
}

/**
 *  Write the contents of the Save to a main and backup slot in SRAM.
 */
void Sram_WriteSave(SramContext* sramCtx) {
    // Compute checksum
    gSaveContext.save.info.checksum = Sram_CalcChecksum(&gSaveContext.save);

    // Write save + backup
    bcopy(&gSaveContext.save, sramCtx->writeBuf, sizeof(Save));
    Flash_WritePagesAsync(&sramCtx->flashReqMain, sramCtx->writeBuf, FLASH_SECTOR_TO_PAGE(gSaveContext.fileNum),
                          FLASH_BYTES_TO_PAGES(sizeof(Save)));
    Flash_WritePagesAsync(&sramCtx->flashReqBackup, sramCtx->writeBuf, FLASH_SECTOR_TO_PAGE(gSaveContext.fileNum + 3),
                          FLASH_BYTES_TO_PAGES(sizeof(Save)));
}

/**
 *  For all 3 slots, verify that the checksum is correct. If corrupted, attempt to load a backup save.
 *  If backup is also corrupted, default to a new save (or debug save for slot 0 on debug rom).
 *
 *  After verifying all 3 saves, pass relevant data to File Select to be displayed.
 */
void Sram_VerifyAndLoadAllSaves(FileSelectState* fileSelect, SramContext* sramCtx) {
    Save* tempSave = (Save*)sramCtx->readBuf;
    u16 oldChecksum;
    u16 newChecksum;
    s32 slotNum;

    for (slotNum = 0; slotNum < 3; slotNum++) {
        // check save
        Flash_ReadPages(tempSave, FLASH_SECTOR_TO_PAGE(slotNum), FLASH_BYTES_TO_PAGES(sizeof(Save)));

        oldChecksum = tempSave->info.checksum;
        newChecksum = Sram_CalcChecksum(tempSave);

        if (newChecksum != oldChecksum) {
            // checksum didnt match, try backup save
            Flash_ReadPages(tempSave, FLASH_SECTOR_TO_PAGE(slotNum + 3), FLASH_BYTES_TO_PAGES(sizeof(Save)));
            oldChecksum = tempSave->info.checksum;
            newChecksum = Sram_CalcChecksum(tempSave);

            if (newChecksum != oldChecksum) {
                // backup failed, reset file
                tempSave->entranceIndex = 0;
                tempSave->linkAge = 0;
                tempSave->cutsceneIndex = 0;
                tempSave->dayTime = 0;
                tempSave->nightFlag = 0;
                tempSave->totalDays = 0;
                tempSave->bgsDayCount = 0;

#if OOT_DEBUG
                if (slotNum == 0) {
                    Sram_InitDebugSave(tempSave, slotNum);
                    SAVE_SIGN(tempSave);
                } else {
                    Sram_InitNewSave(tempSave, slotNum);
                }
#else
                Sram_InitNewSave(tempSave, slotNum);
#endif
                tempSave->info.checksum = Sram_CalcChecksum(tempSave);

                // write the backup
                Flash_WritePagesSync(tempSave, FLASH_SECTOR_TO_PAGE(slotNum + 3), FLASH_BYTES_TO_PAGES(sizeof(Save)));
            }

            // write the file
            Flash_WritePagesSync(tempSave, FLASH_SECTOR_TO_PAGE(slotNum), FLASH_BYTES_TO_PAGES(sizeof(Save)));
        }

        // update file select info
        fileSelect->deaths[slotNum] = tempSave->info.playerData.deaths;
        fileSelect->healthCapacities[slotNum] = tempSave->info.playerData.healthCapacity;
        fileSelect->questItems[slotNum] = tempSave->info.inventory.questItems;
        fileSelect->n64ddFlags[slotNum] = tempSave->info.playerData.n64ddFlag;
        fileSelect->defense[slotNum] = tempSave->info.inventory.defenseHearts;
        fileSelect->health[slotNum] = tempSave->info.playerData.health;
        fileSelect->fileOccupied[slotNum] = SAVE_CHECK_SIGNATURE(tempSave);
        MemCpy(fileSelect->fileNames[slotNum], tempSave->info.playerData.playerName,
               sizeof(tempSave->info.playerData.playerName));
    }
}

void Sram_InitSave(FileSelectState* fileSelect, SramContext* sramCtx) {
#if OOT_DEBUG
    if (fileSelect->buttonIndex != 0) {
        Sram_InitNewSave(&gSaveContext.save, fileSelect->buttonIndex);
    } else {
        Sram_InitDebugSave(&gSaveContext.save, fileSelect->buttonIndex);
    }
#else
    Sram_InitNewSave(&gSaveContext.save, fileSelect->buttonIndex);
#endif

    gSaveContext.save.entranceIndex = ENTR_LINKS_HOUSE_0;
    gSaveContext.save.linkAge = LINK_AGE_CHILD;
    gSaveContext.save.dayTime = CLOCK_TIME(10, 0);
    gSaveContext.save.cutsceneIndex = 0xFFF1;

#if OOT_DEBUG
    if (fileSelect->buttonIndex == 0) {
        gSaveContext.save.cutsceneIndex = 0;
    }
#endif

    // TODO is buttonIndex == gSaveContext.fileNum here? if so we don't need to do the memcpy lower down
    MemCpy(gSaveContext.save.info.playerData.playerName, fileSelect->fileNames[fileSelect->buttonIndex],
           sizeof(gSaveContext.save.info.playerData.playerName));

    SAVE_SIGN(&gSaveContext.save);

    gSaveContext.save.info.playerData.n64ddFlag = fileSelect->n64ddFlag;

    // Write save + backup
    Sram_WriteSave(sramCtx);

    // Update file select info
    fileSelect->deaths[gSaveContext.fileNum] = gSaveContext.save.info.playerData.deaths;
    fileSelect->healthCapacities[gSaveContext.fileNum] = gSaveContext.save.info.playerData.healthCapacity;
    fileSelect->questItems[gSaveContext.fileNum] = gSaveContext.save.info.inventory.questItems;
    fileSelect->n64ddFlags[gSaveContext.fileNum] = gSaveContext.save.info.playerData.n64ddFlag;
    fileSelect->defense[gSaveContext.fileNum] = gSaveContext.save.info.inventory.defenseHearts;
    fileSelect->health[gSaveContext.fileNum] = gSaveContext.save.info.playerData.health;
    fileSelect->fileOccupied[gSaveContext.fileNum] = true;
    MemCpy(fileSelect->fileNames[gSaveContext.fileNum], gSaveContext.save.info.playerData.playerName,
           sizeof(gSaveContext.save.info.playerData.playerName));
}

void Sram_EraseSave(FileSelectState* fileSelect, SramContext* sramCtx) {
    Sram_InitNewSave(&gSaveContext.save, fileSelect->selectedFileIndex);

    // write save and backup
    bcopy(&gSaveContext.save, sramCtx->writeBuf, sizeof(Save));
    Flash_WritePagesAsync(&sramCtx->flashReqMain, sramCtx->writeBuf,
                          FLASH_SECTOR_TO_PAGE(fileSelect->selectedFileIndex), FLASH_BYTES_TO_PAGES(sizeof(Save)));
    Flash_WritePagesAsync(&sramCtx->flashReqBackup, sramCtx->writeBuf,
                          FLASH_SECTOR_TO_PAGE(fileSelect->selectedFileIndex + 3), FLASH_BYTES_TO_PAGES(sizeof(Save)));

    // update file select info
    fileSelect->fileOccupied[fileSelect->selectedFileIndex] = false;
}

void Sram_CopySave(FileSelectState* fileSelect, SramContext* sramCtx) {
    // read save to copy
    Flash_ReadPages(sramCtx->writeBuf, FLASH_SECTOR_TO_PAGE(fileSelect->selectedFileIndex),
                    FLASH_BYTES_TO_PAGES(sizeof(Save)));
    // write save
    Flash_WritePagesAsync(&sramCtx->flashReqMain, sramCtx->writeBuf,
                          FLASH_SECTOR_TO_PAGE(fileSelect->copyDestFileIndex), FLASH_BYTES_TO_PAGES(sizeof(Save)));
    // write backup
    Flash_WritePagesAsync(&sramCtx->flashReqBackup, sramCtx->writeBuf,
                          FLASH_SECTOR_TO_PAGE(fileSelect->copyDestFileIndex + 3), FLASH_BYTES_TO_PAGES(sizeof(Save)));

    // update file select info
    fileSelect->deaths[fileSelect->copyDestFileIndex] = fileSelect->deaths[fileSelect->selectedFileIndex];
    fileSelect->healthCapacities[fileSelect->copyDestFileIndex] =
        fileSelect->healthCapacities[fileSelect->selectedFileIndex];
    fileSelect->questItems[fileSelect->copyDestFileIndex] = fileSelect->questItems[fileSelect->selectedFileIndex];
    fileSelect->n64ddFlags[fileSelect->copyDestFileIndex] = fileSelect->n64ddFlags[fileSelect->selectedFileIndex];
    fileSelect->defense[fileSelect->copyDestFileIndex] = fileSelect->defense[fileSelect->selectedFileIndex];
    fileSelect->health[fileSelect->copyDestFileIndex] = fileSelect->health[fileSelect->selectedFileIndex];
    fileSelect->fileOccupied[fileSelect->copyDestFileIndex] = true;
    MemCpy(fileSelect->fileNames[fileSelect->copyDestFileIndex], fileSelect->fileNames[fileSelect->selectedFileIndex],
           sizeof(fileSelect->fileNames[fileSelect->copyDestFileIndex]));
}

/**
 *  Write the first 16 bytes of the read buffer to the SRAM header
 */
void Sram_WriteOptions(SramContext* sramCtx) {
    Flash_WritePagesAsync(&sramCtx->flashReqMain, &sramCtx->options, FLASH_SECTOR_TO_PAGE(6),
                          FLASH_BYTES_TO_PAGES(sizeof(SaveOptions)));
}

void Sram_ReadOptions(SramContext* sramCtx) {
    Flash_ReadPages(&sramCtx->options, FLASH_SECTOR_TO_PAGE(6), FLASH_BYTES_TO_PAGES(sizeof(SaveOptions)));
}

void Sram_InitSram(GameState* gameState, SramContext* sramCtx) {
    s32 doWrite = false;
    s32 i;

    Sram_ReadOptions(sramCtx);

    for (i = 0; i < ARRAY_COUNT(sDefaultOptions.magic); i++) {
        if (sramCtx->options.magic[i] != sDefaultOptions.magic[i]) {
            // bad magic value, reset options
            gSaveContext.language = sramCtx->options.language;
            sramCtx->options = sDefaultOptions;
            sramCtx->options.language = gSaveContext.language;
            doWrite = true;
        }
    }

    // move options to save context
    gSaveContext.audioSetting = sramCtx->options.sound & 3;
    gSaveContext.zTargetSetting = sramCtx->options.zTarget & 1;
    gSaveContext.language = sramCtx->options.language;
    if (gSaveContext.language >= LANGUAGE_MAX) {
        // invalid language value, reset it
        sramCtx->options.language = gSaveContext.language = LANGUAGE_ENG;
        doWrite = true;
    }

#if OOT_DEBUG
    if (CHECK_BTN_ANY(gameState->input[2].cur.button, BTN_DRIGHT)) {
        // synchronous erase all, debug feature
        Flash_EraseAll();
        doWrite = false;
    }
#endif

    if (doWrite) {
        Sram_WriteOptions(sramCtx);
        Flash_WaitDone(&sramCtx->flashReqMain);
    }

    func_800F6700(gSaveContext.audioSetting);
}

void Sram_AllocWriteBuf(GameState* gameState, SramContext* sramCtx) {
    // The write buffer is used when performing async writes. The data is copied into the write buffer to ensure
    // it does not change while the write is occurring.
    sramCtx->writeBuf = GAME_STATE_ALLOC(gameState, FLASH_ROUNDUP_SIZE(sizeof(Save)), __FILE__, __LINE__);
    assert(sramCtx->writeBuf != NULL);
}

void Sram_AllocReadBuf(GameState* gameState, SramContext* sramCtx) {
    // The read buffer is used when reading into the save context as flash reads occur in multiples of the flash
    // page size. The read buffer would be unnecessary if sizeof(Save) were a multiple of the flash page size.
    sramCtx->readBuf = GAME_STATE_ALLOC(gameState, FLASH_ROUNDUP_SIZE(sizeof(Save)), __FILE__, __LINE__);
    assert(sramCtx->readBuf != NULL);
}
