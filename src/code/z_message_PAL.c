#include "global.h"

#include "message_data_fmt.h"

#define NON_CONST(x, type) (*(type*)(&x))

// TODO move these prototypes

//void func_8006EE50(MessageData*, u16, u16);

void func_800ECC04(u16); // original name : Na_StartOcarinaSinglePlayCheck2
void func_800ED93C(u8,u8);
void func_800ED858(u8);
SubMessageContext_E2B8* func_800EE3D4(void); // original name : Na_StopOcarinaMode
SubMessageContext_E2B8* func_800EE3F8(void);

void func_801069B0(void);
void func_80106AA8(GlobalContext* globalCtx);
u8 func_80106BC8(GlobalContext* globalCtx);
u8 func_80106C88(GlobalContext* globalCtx);
void func_80106CCC(GlobalContext* globalCtx);
void func_80106D40(GlobalContext* globalCtx, u8 arg1);
void func_80106F1C(GlobalContext* globalCtx, void* textureImage, Gfx** p);

// TODO const data / rodata

typedef struct {
    /* 0x0000 */ u16 textId;
    /* 0x0002 */ u8 xy; // TODO name better
    /* 0x0004 */ u32 segment;
} MessageTableEntry; // size = 0x8

extern const MessageTableEntry D_8014B320[]; // English message entry table
extern const u32 D_8014F548[]; // German message entry table
extern const u32 D_80151658[]; // French message entry table
extern const MessageTableEntry D_80153768[]; // ?

extern const s16 D_8014B2F8;
extern const s16 D_8014B2FC;

extern const MessageTableEntry* D_801538F0;
extern const u32* D_801538F4;
extern const u32* D_801538F8;
extern const MessageTableEntry* D_801538FC;

extern u16 D_8014B31C;
extern s16 D_8014B310;

extern const s16 D_80153958[]; // original name : onpu_buf (note_buf)

// extern const s16 D_80153984;

extern const s16 D_80153C40[4];
extern const s16 D_80153C48[4];

/* extern const s16 D_80153CF8;
extern const s32 D_80153CFC; */

/* const u32 D_8014B2F0 = 0x00000000;

const u32 D_8014B2F4 = 0x00000000;

const s32 D_8014B2F8 = 0x00000000;

const s32 D_8014B2FC = 0x00000000;

const u32 D_8014B300 = 0x00000000;

const u32 D_8014B304 = 0x00000000;

const u32 D_8014B308[] = {
    0x00000000, 0x00000000,
};

const u32 D_8014B310 = 0x00FF0000;

const u32 D_8014B314 = 0x00000000;

const u32 D_8014B318 = 0x00000000;

const u32 D_8014B31C = 0x00000000;

#include "z_message_tables.c"

const MessageTableEntry* D_801538F0 = &D_8014B320;

const TextSegment* D_801538F4 = &D_8014F548;

const TextSegment* D_801538F8 = &D_80151658;

const TextSegment* D_801538FC = &D_80153768;

const u32 D_80153900[] = {
    0x00FF00FF, 0x00FF0032, 0x00140000, 0x00FF003C, 0x000000FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF,
    0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF,
};

const u32 D_80153930[] = {
    0x00000000, 0x000000DC, 0x00960000, 0x00000000, 0x00000000, 0x00000000,
};

const u8 D_80153948[] = {
    0x00, 0x01, 0x00,
};

const u8 D_8015394B = 0x02;

const u8 D_8015394C = 0x00;

const u32 D_80153950[] = {
	0x00000000, 0x00000000,
};

const Struct_801069B0 D_80153958 = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const u32 D_8015396C[] = {
    0x00000001, 0x00020003, 0x00040005, 0x00080007, 0x00060009,
};

const u32 D_80153980 = 0x000A000B;

// const s16 D_80153984 = 0x0000;

const u32 D_80153988[] = {
    0x3F99999A, 0x3FC00000, 0x3FE66666, 0x40000000, 0x40066666, 0x400CCCCD, 0x40066666, 0x40000000,
};

const u32 D_801539A8[] = {
    0x3F19999A, 0x3F400000, 0x3F666666, 0x3F800000, 0x3F866666, 0x3F8CCCCD, 0x3F866666, 0x3F800000,
};

const u32 D_801539C8[] = {
    0x000000C8, 0x00500032, 0x00FF0082,
};

const u32 D_801539D4[] = {
    0x00000000, 0x00000000, 0x00FF0082,
};

const u32 D_801539E0 = 0x00000000;

const u32 D_801539E4 = 0x00C80000;

const u32 D_801539E8 = 0x00500000;

const u32 D_801539EC = 0x000C0000;

const u32 D_801539F0 = 0x00000000;

const u32 D_801539F4 = 0x00000000;

const u32 D_801539F8 = 0x00000000;

const u32 D_801539FC[] = {
    0x00000000, 0x41000000, 0x41000000, 0x40C00000, 0x41100000, 0x41100000, 0x41600000, 0x41400000,
    0x40400000, 0x40E00000, 0x40E00000, 0x40E00000, 0x41100000, 0x40800000, 0x40C00000, 0x40800000,
    0x41100000, 0x41200000, 0x40A00000, 0x41100000, 0x41100000, 0x41200000, 0x41100000, 0x41100000,
    0x41100000, 0x41100000, 0x41100000, 0x40C00000, 0x40C00000, 0x41100000, 0x41300000, 0x41100000,
    0x41300000, 0x41500000, 0x41400000, 0x41100000, 0x41300000, 0x41300000, 0x41000000, 0x41000000,
    0x41400000, 0x41200000, 0x40800000, 0x41000000, 0x41200000, 0x41000000, 0x41500000, 0x41300000,
    0x41500000, 0x41100000, 0x41500000, 0x41200000, 0x41200000, 0x41100000, 0x41200000, 0x41300000,
    0x41700000, 0x41300000, 0x41200000, 0x41200000, 0x40E00000, 0x41200000, 0x40E00000, 0x41200000,
    0x41100000, 0x40A00000, 0x41000000, 0x41100000, 0x41000000, 0x41100000, 0x41100000, 0x40C00000,
    0x41100000, 0x41000000, 0x40800000, 0x40C00000, 0x41000000, 0x40800000, 0x41400000, 0x41100000,
    0x41100000, 0x41100000, 0x41100000, 0x40E00000, 0x41000000, 0x40E00000, 0x41000000, 0x41100000,
    0x41400000, 0x41000000, 0x41100000, 0x41000000, 0x40E00000, 0x40A00000, 0x40E00000, 0x41200000,
    0x41200000, 0x41400000, 0x40C00000, 0x41400000, 0x41400000, 0x41300000, 0x41000000, 0x41000000,
    0x41000000, 0x40C00000, 0x40C00000, 0x41500000, 0x41500000, 0x41200000, 0x41200000, 0x41200000,
    0x41100000, 0x41000000, 0x41000000, 0x41000000, 0x41000000, 0x41000000, 0x41100000, 0x41100000,
    0x41100000, 0x41100000, 0x40C00000, 0x41100000, 0x41100000, 0x41100000, 0x41100000, 0x41100000,
    0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000,
    0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000, 0x41600000,
    0x41600000,
};

const s16 D_80153C40[] = {
    0x004A, 0x004A, 0x004A, 0x0000,
};

const s16 D_80153C48[] = {
    0x0048, 0x0048, 0x0048, 0x0000,
};

const u16 D_80153C50 = 0x0000;

const u16 D_80153C52 = 0x0001;

const u16 D_80153C54 = 0x0003;

const u16 D_80153C56 = 0x0002;

const u32 D_80153C58[] = {
    0x00010002, 0x00040008, 0x00100020, 0x01000080,
};

const u32 D_80153C68[] = {
    0x00400200, 0x04000800, 0x10000000, 0x01990198,
};

const u32 D_80153C78[] = {
    0x018A017E, 0x018A018B, 0x01CB0000, 0x00000000, 0x00000000, 0x00010000, 0x00000000,
};

const u32 D_80153C94[] = {
    0x02002940, 0x02002A40, 0x02002B40, 0x02002C40, 0x02002D40,
};

const u32 D_80153CA8[] = {
    0x005000FF, 0x00960064, 0x00FF00C8,
};

const u32 D_80153CB4[] = {
    0x000A000A, 0x000A0032, 0x00FF0032,
};

const u32 D_80153CC0[] = {
    0x00FF00FF, 0x003200FF, 0x00FF00B4,
};

const u32 D_80153CCC[] = {
    0x000A000A, 0x000A006E, 0x006E0032,
};

const u32 D_80153CD8 = 0x000C0000;

const u32 D_80153CDC = 0x00010000;

const u32 D_80153CE0[] = {
    0x00340033, 0x00350036, 0x00370025, 0x00440045, 0x00460047, 0x00480049,
};

//const s16 D_80153CF8 = 0x00000000;

//const u32 D_80153CFC = 0x00000000;

const u32 D_80153D00[] = {
    0x00220022, 0x00220022, 0x00220022,
};

const u32 D_80153D0C[] = {
    0x008E008E, 0x008E008E, 0x00AE008E,
};

const u32 D_80153D18[] = {
    0x00260026, 0x00260026, 0x00AE0026,
};

const u32 D_80153D24[] = {
    0x005A005A, 0x005A005A, 0x00AE005A,
};

const u32 D_80153D30[] = {
    0x003B003B, 0x003B003B, 0x0022003B, 0x04000400, 0x02000000, 0x10380008, 0x200A088B, 0x00070009,
    0x000A107E, 0x20082007, 0x00150016, 0x00170003, 0x0000270B, 0x00C8012C, 0x012DFFDA, 0x00140016,
    0x00140016,
};

const u32 D_80153D74 = 0x00000000;

const u32 D_80153D78[] = { // msgno
    0x00000000, 0x00000000,
};

//const u32 D_80153D80 = 0x00000000;

const u32 D_80153D84[] = {
    0x00000000, 0x00000000, 0x00000000,
}; */

// bss
s32 D_801759A0;
s32 D_801759A4;
s16 D_801759A8;
s16 D_801759AA;
s16 D_801759AC;
s16 D_801759AE;
s16 D_801759B0;
s16 D_801759B2;
s16 D_801759B4;
s16 D_801759B6;
s16 D_801759B8;
s16 D_801759BA;
s16 D_801759BC;
s16 D_801759BE;

// Segment addresses

extern UNK_TYPE D_02002E40;

extern const u8 D_8015394C[];

#define NON_CONST_ARR(x, type) ((type)(x))

void func_801069B0(void) {
    s16* D_80153958_NonConst;

    VREG(45) = 0xBD;
    VREG(46) = 0xB8;
    VREG(47) = 0xB3;
    VREG(48) = 0xAE;
    VREG(49) = 0xA9;
    NON_CONST_ARR(D_8015394C, u8*)[0] = 0xFF;
    D_80153958_NonConst = &NON_CONST_ARR(D_80153958, s16*)[0];
    D_80153958_NonConst[8] = 0;
    D_80153958_NonConst[7] = D_80153958_NonConst[8];
    D_80153958_NonConst[6] = D_80153958_NonConst[8];
    D_80153958_NonConst[5] = D_80153958_NonConst[8];
    D_80153958_NonConst[4] = D_80153958_NonConst[8];
    D_80153958_NonConst[3] = D_80153958_NonConst[8];
    D_80153958_NonConst[2] = D_80153958_NonConst[8];
    D_80153958_NonConst[1] = D_80153958_NonConst[8];
    D_80153958_NonConst[0] = D_80153958_NonConst[8];
    D_801759A8 = 0x50;
    D_801759AC = 0xFF;
    D_801759AA = 0x96;
    D_801759AE = 0xA;
    D_801759B2 = 0xA;
    D_801759B0 = 0xA;
    D_801759B4 = 0xFF;
    D_801759B8 = 0xFF;
    D_801759B6 = 0x32;
    D_801759BA = 0xA;
    D_801759BE = 0xA;
    D_801759BC = 0xA;
}

void func_80106AA8(GlobalContext* globalCtx) {
    MessageContext* msgCtx;

    globalCtx->msgCtx.msgMode++;
    msgCtx = &globalCtx->msgCtx;
    if (globalCtx->msgCtx.msgMode == 0x2E) {
        func_800ED858(1);
        msgCtx->unk_E2B8 = func_800EE3D4();
        msgCtx->unk_E2B8->unk_2 = NON_CONST(D_8014B2F8, s16) = 0;
        func_800ECC04(0xA000);
        msgCtx->unk_E3D2 = msgCtx->unk_E3D4;
    } else if (msgCtx->msgMode == 0x2C) {
        func_800ED858(6);
        msgCtx->unk_E2B8 = func_800EE3F8();
        msgCtx->unk_E2B8->unk_2 = NON_CONST(D_8014B2F8, s16) = 0;
        func_800ED93C(0xE, 1);
        msgCtx->unk_E3E7 = 2;
    }
    func_801069B0();
}

u8 func_80106BC8(GlobalContext* globalCtx) {
    Input* curInput = &globalCtx->state.input[0];
    if (CHECK_BTN_ALL(curInput->press.button, BTN_A) || 
        CHECK_BTN_ALL(curInput->press.button, BTN_B) || 
        CHECK_BTN_ALL(curInput->press.button, BTN_CUP)) {
        Audio_PlaySoundGeneral(NA_SE_SY_MESSAGE_PASS, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
    }
    return CHECK_BTN_ALL(curInput->press.button, BTN_A) || 
           CHECK_BTN_ALL(curInput->press.button, BTN_B) || 
           CHECK_BTN_ALL(curInput->press.button, BTN_CUP);
}

u8 func_80106C88(GlobalContext* globalCtx) {
    Input* curInput = &globalCtx->state.input[0];
    return CHECK_BTN_ALL(curInput->press.button, BTN_A) || 
           CHECK_BTN_ALL(curInput->press.button, BTN_B) || 
           CHECK_BTN_ALL(curInput->press.button, BTN_CUP);
}

void func_80106CCC(GlobalContext* globalCtx) {
    MessageContext* msgCtx = &globalCtx->msgCtx;
    if (msgCtx->unk_E300 != 0) {
        msgCtx->unk_E3E7 = 2;
        msgCtx->msgMode = 0x36;
        msgCtx->unk_E3E4 = 0;
        Audio_PlaySoundGeneral(0, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
    }
}

#ifdef NON_MATCHING
// Matches but data instead of rodata
void func_80106D40(GlobalContext* globalCtx, u8 arg1) {
    static s16 D_80153984 = 0x0000;

    MessageContext* msgCtx;
    Input* curInput = &globalCtx->state.input[0];

    msgCtx = &globalCtx->msgCtx;
    if (curInput->rel.stick_y >= 0x1E && D_80153984 == 0) {
        D_80153984 = 1;
        msgCtx->choiceIndex--;
        if (msgCtx->choiceIndex >= 0x81) {
            msgCtx->choiceIndex = 0;
        } else {
            Audio_PlaySoundGeneral(NA_SE_SY_CURSOR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
        }
    } else if (curInput->rel.stick_y < -0x1D && D_80153984 == 0) {
        D_80153984 = 1;
        msgCtx->choiceIndex++;
        if (msgCtx->choiceIndex > arg1) {
            msgCtx->choiceIndex = arg1;
        } else {
            Audio_PlaySoundGeneral(NA_SE_SY_CURSOR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
        }
    } else if (ABS(curInput->rel.stick_y) < 0x1E) {
        D_80153984 = 0;
    }
    msgCtx->unk_E3D8 = XREG(66);
    msgCtx->unk_E3DA = (arg1 == 1) ? gGameInfo->data[msgCtx->choiceIndex + 0x584] : gGameInfo->data[msgCtx->choiceIndex + 0x583];
}
#else
void func_80106D40(GlobalContext* globalCtx, u8 arg1);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_80106D40.s")
#endif

void func_80106F1C(GlobalContext* globalCtx, void* textureImage, Gfx** p) {
    MessageContext* msgCtx;
    Gfx* gfx;
    s16 unk_E3D8;
    s16 unk_E3DA;

    gfx = *p;
    msgCtx = &globalCtx->msgCtx;
    unk_E3D8 = msgCtx->unk_E3D8;
    unk_E3DA = msgCtx->unk_E3DA;
    gDPPipeSync(gfx++);
    D_801759A0 = (XREG(57) / 100.0f) * 16.0f;
    D_801759A4 = 1024.0f / (XREG(57) / 100.0f);
    gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, textureImage);
    gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, 
                G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 63, 2048);
    gDPPipeSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, 
                G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD, 
                G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, 0x003C, 0x003C);
    if (msgCtx->unk_E2FD != 5) {
        gDPSetPrimColor(gfx++, 0, 0, 0x00, 0x00, 0x00, msgCtx->unk_E3E2);
        gSPTextureRectangle(gfx++, 
                    (unk_E3D8 + XREG(60)) * 4, 
                    (unk_E3DA + XREG(60)) * 4, 
                    (unk_E3D8 + XREG(60) + D_801759A0) * 4, 
                    (unk_E3DA + XREG(60) + D_801759A0) * 4, G_TX_RENDERTILE, 0, 0, D_801759A4, D_801759A4);
    }
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, msgCtx->unk_E3DC, msgCtx->unk_E3DE, msgCtx->unk_E3E0, msgCtx->unk_E3E2);
    gSPTextureRectangle(gfx++, 
                unk_E3D8 * 4, 
                unk_E3DA * 4, 
                (unk_E3D8 + D_801759A0) * 4, 
                (unk_E3DA + D_801759A0) * 4, G_TX_RENDERTILE, 0, 0, D_801759A4, D_801759A4);
    *p = gfx;
}

#ifdef NON_MATCHING
// regalloc only, rodata unmigrated
void func_80107244(MessageContext *msgCtx) {
    static const f32 D_80153988[] = {
        1.20000004768f, 1.5f, 1.79999995232f, 2.0f, 2.09999990463f, 2.20000004768f, 2.09999990463f, 2.0f,
    };
    static const f32 D_801539A8[] = {
        0.600000023842f, 0.75f, 0.899999976158f, 1.0f, 1.04999995232f, 1.10000002384f, 1.04999995232f, 1.0f,
    };
    f32 temp_f0;
    f32 temp_f18;
    f32 temp_f14;
    f32 temp_f4;
    f32 temp_f2;
    f32 temp_f16;

    temp_f2 = D_80153988[msgCtx->unk_E3E7];
    temp_f16 = D_801539A8[msgCtx->unk_E3E7];
    temp_f0 = (temp_f2 + temp_f2) * XREG(74);
    temp_f14 = temp_f16 * XREG(75);
    temp_f18 = XREG(76) / (temp_f2 + temp_f2);
    temp_f4 = XREG(77) / temp_f16;
    VREG(1) = XREG(73) + (XREG(73) - (s16)(temp_f16 * XREG(73) + 0.5f)) / 2;
    msgCtx->unk_E406 += msgCtx->unk_E404 / 8;
    msgCtx->unk_E3E7++;
    if (msgCtx->unk_E3E7 == 8) {
        VREG(0) = XREG(72);
        VREG(1) = XREG(73);
        msgCtx->msgMode = 3;
        msgCtx->unk_E406 = msgCtx->unk_E404;
    }
    YREG(22) = (s16)(temp_f0 + 0.5f) / 2;
    YREG(23) = (s16)(temp_f14 + 0.5f);
    YREG(16) = temp_f18 + 0.5f;
    YREG(17) = temp_f4 + 0.5f;
    VREG(0) = (XREG(72) + XREG(74)) - (YREG(22) / 2);
}
#else
void func_80107244(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_80107244.s")
#endif

void func_80107448(GlobalContext* globalCtx, u16 textId) {
    s32 foundSeg;
    s32 nextSeg;
    MessageTableEntry* messageTableEntry;
    u32* languageSegmentTable;
    Font* font;
    u32 seg;

    messageTableEntry = D_801538F0;
    if (gSaveContext.language == LANGUAGE_ENG) {
        seg = messageTableEntry->segment;
        while (messageTableEntry->textId != 0xFFFF) {
            font = &globalCtx->msgCtx.font;
            if (messageTableEntry->textId == textId) {
                foundSeg = messageTableEntry->segment;
                font->xy = messageTableEntry->xy;
                messageTableEntry++;
                nextSeg = messageTableEntry->segment;
                font->msgOffset = foundSeg - seg;
                font->msgLength = nextSeg - foundSeg;
                // Message found!!!
                osSyncPrintf(" メッセージが,見つかった！！！ = %x  (data=%x) (data0=%x) (data1=%x) (data2=%x) (data3=%x)\n",
                            textId, font->msgOffset, font->msgLength, foundSeg, seg, nextSeg);
                return;
            }
            messageTableEntry++;
        }
    } else {
        languageSegmentTable = (gSaveContext.language == LANGUAGE_GER) ? D_801538F4 : D_801538F8;
        seg = messageTableEntry->segment;
        while (messageTableEntry->textId != 0xFFFF) {
            font = &globalCtx->msgCtx.font;
            if (messageTableEntry->textId == textId) {
                foundSeg = *languageSegmentTable;
                font->xy = messageTableEntry->xy;
                languageSegmentTable++;
                nextSeg = *languageSegmentTable;
                font->msgOffset = foundSeg - seg;
                font->msgLength = nextSeg - foundSeg;
                // Message found!!!
                osSyncPrintf(" メッセージが,見つかった！！！ = %x  (data=%x) (data0=%x) (data1=%x) (data2=%x) (data3=%x)\n",
                            textId, font->msgOffset, font->msgLength, foundSeg, seg, nextSeg);
                return;
            }
            messageTableEntry++;
            languageSegmentTable++;
        }
    }
    // Message not found!!!
    osSyncPrintf(" メッセージが,見つからなかった！！！ = %x\n", textId);
    font = &globalCtx->msgCtx.font;
    messageTableEntry = D_801538F0;
    if (gSaveContext.language == LANGUAGE_ENG) {
        foundSeg = messageTableEntry->segment;
        font->xy = messageTableEntry->xy;
        messageTableEntry++;
        nextSeg = messageTableEntry->segment;
    } else {
        languageSegmentTable = (gSaveContext.language == LANGUAGE_GER) ? D_801538F4 : D_801538F8;
        foundSeg = *languageSegmentTable;
        font->xy = messageTableEntry->xy;
        languageSegmentTable++;
        nextSeg = *languageSegmentTable;
    }
    font->msgOffset = foundSeg - seg;
    font->msgLength = nextSeg - foundSeg;
}

void func_80107628(GlobalContext* globalCtx, u16 textId) {
    u32 foundSeg;
    u32 nextSeg;
    u32 seg;
    MessageTableEntry* messageTableEntry;
    Font* font;

    messageTableEntry = D_801538FC;
    seg = messageTableEntry->segment;
    while (messageTableEntry->textId != 0xFFFF) {
        font = &globalCtx->msgCtx.font;
        if (messageTableEntry->textId == textId) {
            foundSeg = messageTableEntry->segment;
            font->xy = messageTableEntry->xy;
            messageTableEntry++;
            nextSeg = messageTableEntry->segment;
            font->msgOffset = foundSeg - seg;
            font->msgLength = nextSeg - foundSeg;
            // Message found!!!
            osSyncPrintf(" メッセージが,見つかった！！！ = %x  (data=%x) (data0=%x) (data1=%x) (data2=%x) (data3=%x)\n", 
                        textId, font->msgOffset, font->msgLength, foundSeg, seg, nextSeg);
            return;
        }
        messageTableEntry++;
    }
}

void func_801076CC(MessageContext* msgCtx, u16 textId) {
    switch (textId) {
        case 1:
            if (msgCtx->unk_E2FD == 1) {
                msgCtx->unk_E3DC = 0xFF;
                msgCtx->unk_E3DE = 0x78;
                msgCtx->unk_E3E0 = 0;
            } else {
                msgCtx->unk_E3DC = 0xFF;
                msgCtx->unk_E3DE = 0x3C;
                msgCtx->unk_E3E0 = 0x3C;
            }
            break;
        case 2:
            if (msgCtx->unk_E2FD == 1) {
                msgCtx->unk_E3DC = VREG(33);
                msgCtx->unk_E3DE = VREG(34);
                msgCtx->unk_E3E0 = VREG(35);
            } else {
                msgCtx->unk_E3DC = VREG(36);
                msgCtx->unk_E3DE = VREG(37);
                msgCtx->unk_E3E0 = VREG(38);
            }
            break;
        case 3:
            if (msgCtx->unk_E2FD == 1) {
                msgCtx->unk_E3DC = 0x50;
                msgCtx->unk_E3DE = 0x6E;
                msgCtx->unk_E3E0 = 0xFF;
            } else {
                msgCtx->unk_E3DC = 0x50;
                msgCtx->unk_E3DE = 0x5A;
                msgCtx->unk_E3E0 = 0xFF;
            }
            break;
        case 4:
            if (msgCtx->unk_E2FD == 1) {
                msgCtx->unk_E3DC = 0x5A;
                msgCtx->unk_E3DE = 0xB4;
                msgCtx->unk_E3E0 = 0xFF;
            } else if (msgCtx->unk_E2FD == 5) {
                msgCtx->unk_E3DC = 0x50;
                msgCtx->unk_E3DE = 0x96;
                msgCtx->unk_E3E0 = 0xB4;
            } else {
                msgCtx->unk_E3DC = 0x64;
                msgCtx->unk_E3DE = 0xB4;
                msgCtx->unk_E3E0 = 0xFF;
            }
            break;
        case 5:
            if (msgCtx->unk_E2FD == 1) {
                msgCtx->unk_E3DC = 0xD2;
                msgCtx->unk_E3DE = 0x64;
                msgCtx->unk_E3E0 = 0xFF;
            } else {
                msgCtx->unk_E3DC = 0xFF;
                msgCtx->unk_E3DE = 0x96;
                msgCtx->unk_E3E0 = 0xB4;
            }
            break;
        case 6:
            if (msgCtx->unk_E2FD == 1) {
                msgCtx->unk_E3DC = 0xFF;
                msgCtx->unk_E3DE = 0xFF;
                msgCtx->unk_E3E0 = 0x1E;
            } else {
                msgCtx->unk_E3DC = 0xE1;
                msgCtx->unk_E3DE = 0xFF;
                msgCtx->unk_E3E0 = 0x32;
            }
            break;
        case 7:
            msgCtx->unk_E3DC = msgCtx->unk_E3DE = msgCtx->unk_E3E0 = 0;
            break;
        case 0: // required to match
        default:
            if (msgCtx->unk_E2FD == 5) {
                msgCtx->unk_E3DC = msgCtx->unk_E3DE = msgCtx->unk_E3E0 = 0;
            } else {
                msgCtx->unk_E3DC =  msgCtx->unk_E3DE = msgCtx->unk_E3E0 = 0xFF;
            }
            break;
    }
}

extern Vec3s D_801539C8[]; // struct array where each struct is 0x6 in size
extern Vec3s D_801539D4[]; // struct array where each struct is 0x6 in size

extern s16 D_801539E0;
extern s16 D_801539E4;
extern s16 D_801539E8;

extern s16 D_801539EC;

extern s16 D_801539F0;

extern s16 D_801539F4;
extern s16 D_801539F8;
extern s16 D_801539FC;

extern s16 D_8014B308;

#ifdef NON_EQUIVALENT
// A lot
void func_80107980(GlobalContext *globalCtx, Gfx **p, s16 arg2, s16 arg3) {
    s16 sp60;

    Gfx* gfx;

    s16 temp_a2;

    s16 temp_t5;
    Vec3s *temp_a0;
    Vec3s *temp_a1_2;
    s32 phi_v1;
    s16 phi_a3;

    s16 temp_t1;
    s16 temp_t2;
    s16 temp_a3;

    s16 phi_t5;
    s16 phi_t1;
    s16 phi_t2;

    gfx = *p;
    if (D_8014B308 == 0) {
        temp_a0 = &D_801539C8[D_801539F0];

        temp_t2 = D_801539E0;
        phi_v1 = ABS(temp_t2 - temp_a0->x);
        temp_t5 = phi_v1 / D_801539EC;

        temp_t1 = D_801539E4;
        phi_v1 = ABS(temp_t1 - temp_a0->y);
        sp60 = phi_v1 / D_801539EC;
        
        phi_a3 = D_801539E8;
        phi_v1 = ABS(phi_a3 - temp_a0->z);

        if (temp_t2 >= temp_a0->x) {
            temp_t2 -= temp_t5;
        } else {
            temp_t2 += temp_t5;
        }

        if (temp_t1 >= temp_a0->y) {
            temp_t1 -= sp60;
        } else {
            temp_t1 += sp60;
        }

        if (phi_a3 >= temp_a0->z) {
            phi_a3 -= phi_v1 / D_801539EC;
        } else {
            phi_a3 += phi_v1 / D_801539EC;
        }
        D_801539E0 = temp_t2;
        D_801539E4 = temp_t1;
        D_801539E8 = phi_a3;

        temp_a1_2 = &D_801539D4[D_801539F0];

        phi_t5 = D_801539F4;
        phi_v1 = ABS(phi_t5 - temp_a1_2->x);

        phi_t1 = D_801539F8;
        temp_a2 = phi_v1 / D_801539EC;
        phi_v1 = ABS(phi_t1 - temp_a1_2->y);

        phi_t2 = D_801539FC;
        temp_a3 = phi_v1 / D_801539EC;
        phi_v1 = ABS(phi_t2 - temp_a1_2->z);

        if (phi_t5 >= temp_a1_2->x) {
            phi_t5 -= temp_a2;
        } else {
            phi_t5 += temp_a2;
        }

        if (phi_t1 >= temp_a1_2->y) {
            phi_t1 -= temp_a3;
        } else {
            phi_t1 += temp_a3;
        }

        if (phi_t2 >= temp_a1_2->z) {
            phi_t2 -= phi_v1 / D_801539EC;
        } else {
            phi_t2 += phi_v1 / D_801539EC;
        }

        D_801539EC--;
        if (D_801539EC == 0) {
            D_801539E0 = temp_a0->x;
            D_801539E4 = temp_a0->y;
            D_801539E8 = temp_a0->z;
            D_801539F0 ^= 1;
            D_801539EC = 0xC;
            phi_t5 = temp_a1_2->x;
            phi_t1 = temp_a1_2->y;
            phi_t2 = temp_a1_2->z;
        }

        gDPPipeSync(gfx++);
        gDPSetCombineLERP(gfx++, 
                        PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, 
                        PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetPrimColor(gfx++, 0, 0, D_801539E0, D_801539E4, D_801539E8, 255);
        gDPSetEnvColor(gfx++, phi_t5, phi_t1, phi_t2, 255);

        D_801539F4 = phi_t5;
        D_801539F8 = phi_t1;
        D_801539FC = phi_t2;

        gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, globalCtx->msgCtx.font.iconBuf);
        gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, 
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD, 
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
        gDPLoadSync(gfx++);
        gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 63, 2048);
        gDPPipeSync(gfx++);
        gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, 
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD, 
                        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, 60, 60);

        D_801759A0 = (XREG(57) / 100.0f) * 16.0f;
        D_801759A4 = 1024.0f / (XREG(57) / 100.0f);

        gSPTextureRectangle(gfx++, arg2 * 4, arg3 * 4, (arg2 + D_801759A0) * 4, (arg3 + D_801759A0) * 4 , G_TX_RENDERTILE, 0, 0, D_801759A4, D_801759A4);

        globalCtx->msgCtx.unk_E3E7++;
        *p = gfx;
    }
}
#else
void func_80107980(GlobalContext *globalCtx, Gfx **p, s16 arg2, s16 arg3);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_80107980.s")
#endif

u16 func_801080B4(GlobalContext* globalCtx, u16 arg1, Gfx** p, u16 arg3) {
    s32 pad;
    Gfx* gfx = *p;
    MessageContext* msgCtx = &globalCtx->msgCtx;

    // clang-format off
    // same line required to match
    if (msgCtx->msgMode == 6) { Audio_PlaySoundGeneral(0, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8); }
    // clang-format on

    gDPPipeSync(gfx++);
    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0xFF, 0xFF, msgCtx->unk_E3E2);

    if (arg1 >= 0x66) {
        gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 1, (u32)msgCtx->textboxSegment + 0x1000);
        gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 0, 0x0000, G_TX_LOADTILE, 0, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPLoadSync(gfx++);
        gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 575, 171);
        gDPPipeSync(gfx++);
        gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 6, 0x0000, G_TX_RENDERTILE, 0, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, 0x005C, 0x005C);
    } else {
        gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 1, (u32)msgCtx->textboxSegment + 0x1000);
        gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 0, 0x0000, G_TX_LOADTILE, 0, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPLoadSync(gfx++);
        gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 1023, 128);
        gDPPipeSync(gfx++);
        gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 8, 0x0000, G_TX_RENDERTILE, 0, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, 0x007C, 0x007C);
    }
    gSPTextureRectangle(gfx++, 
                    (msgCtx->unk_E3D8 + YREG(71))*4, 
                    YREG(72)*4, 
                    (msgCtx->unk_E3D8 + YREG(71) + YREG(75))*4, 
                    (YREG(72) + YREG(75))*4, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400);
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

    msgCtx->unk_E3D8 += 0x20;
    arg3++;
    *p = gfx;
    return arg3;
}

void func_801083F8(GlobalContext* globalCtx) {
    MessageContext* msgCtx = &globalCtx->msgCtx;

    if (globalCtx->msgCtx.msgMode == 6) {
        globalCtx->msgCtx.unk_E3EE = 1;
        if (msgCtx->unk_E3F0 == 0x2A) {
            msgCtx->msgMode = 0x21;
            // Recording Start / Recording Start / Recording Start / Recording Start ->
            osSyncPrintf("録音開始 録音開始 録音開始 録音開始  -> ");
        } else if (msgCtx->unk_E3F0 == 0x2B) {
            // Recording Playback / Recording Playback / Recording Playback / Recording Playback ->
            osSyncPrintf("録音再生 録音再生 録音再生 録音再生  -> ");
            func_800ED858(1);
            func_800ED858(1);
            msgCtx->unk_E2B8 = func_800EE3F8();
            NON_CONST(D_8014B2F8, s16) = NON_CONST(D_8014B2FC, s16) = 0;
            msgCtx->unk_E2B8->unk_2 = D_8014B2F8;
            func_801069B0();
            msgCtx->unk_E3E7 = 3;
            msgCtx->msgMode = 0x23;
            func_800ED93C(0xF, 1);
        } else if (msgCtx->unk_E3F0 == 0x2C) {
            msgCtx->msgMode = 0x24;
            // 8 Note Recording Start / 8 Note Recording Start / 8 Note Recording Start ->
            osSyncPrintf("８音録音開始 ８音録音開始 ８音録音開始  -> ");
        } else if (msgCtx->unk_E3F0 == 0x2D) {
            // 8 Note Playback / 8 Note Playback / 8 Note Playback ->
            osSyncPrintf("８音再生 ８音再生 ８音再生  -> ");
            func_800ED858(1);
            func_800ED858(1);
            msgCtx->unk_E2B8 = func_800EE3F8();
            NON_CONST(D_8014B2F8, s16) = NON_CONST(D_8014B2FC, s16) = 0;
            msgCtx->unk_E2B8->unk_2 = D_8014B2F8;
            func_801069B0();
            msgCtx->unk_E3E7 = 3;
            msgCtx->msgMode = 0x28;
            func_800ED93C(0xD, 1);
        } else if (msgCtx->unk_E3F0 == 0x2E) {
            msgCtx->msgMode = 0x29;
            // Musical Round Start / Musical Round Start / Musical Round Start / Musical Round Start ->
            osSyncPrintf("輪唱開始 輪唱開始 輪唱開始 輪唱開始  -> ");
        } else if (msgCtx->unk_E3F0 == 0x2F) {
            msgCtx->msgMode = 0x31;
            // Frog Chorus / Frog Chorus ->
            osSyncPrintf("カエルの合唱 カエルの合唱  -> ");
        } else {
            // Ocarina（%d）
            osSyncPrintf("オカリナ（%d） ", msgCtx->unk_E3F0);
            if (msgCtx->unk_E3F0 == 0 || msgCtx->unk_E3F0 == 1 || msgCtx->unk_E3F0 >= 0x22) {
                msgCtx->msgMode = 9;
                osSyncPrintf("000000000000  -> ");
            } else if (msgCtx->unk_E3F0 >= 2 && msgCtx->unk_E3F0 < 0xE) {
                msgCtx->msgMode = 0xA;
                osSyncPrintf("111111111111  -> ");
            } else {
                msgCtx->msgMode = 0xB;
                osSyncPrintf("222222222222  -> ");
            }
        }
        osSyncPrintf("msg_mode=%d\n", msgCtx->msgMode);
    }
}

typedef struct {
    s16 r,g,b;
} ColorRBG16; // size = 0x6

extern const ColorRBG16 D_80153930[];
extern const ColorRBG16 D_80153900[];

extern const s16 D_80153948[];
extern const f32 D_80153980[];

extern u8 D_8014B300;
extern s16 D_8014B318;

#ifdef NON_EQUIVALENT
// Weirdness with the nested loop, somewhat ok otherwise
void func_801086B0(GlobalContext *globalCtx, Gfx **gfxP) {
    u16 sp128;
    Gfx *sp120;
    Font *sp58;
    s32 sp50;
    MessageContext *temp_s1;
    s32 temp_a0;
    u16 temp_a3;
    s32 temp_s2;
    s32 temp_t7_2;
    s32 temp_v0_18;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_7;
    u16 temp_v0_19;
    u32 temp_t8;
    u8 temp_a1;
    u8 temp_v0;
    u8 temp_v0_2;
    u8 temp_v0_5;
    u8 temp_v0_6;
    u16 phi_s2;
    u16 phi_a0;
    s32 phi_t0;
    s32 phi_t1;
    s32 phi_t2;
    s32 phi_t3;
    s32 phi_t4;
    s32 phi_t5;
    s32 phi_v1;
    ColorRBG16* temp_v0_8;
    ColorRBG16* temp_v0_13;
    Font* font;

    sp120 = *gfxP;
    globalCtx->msgCtx.unk_E3D8 = XREG(54);
    temp_s1 = &globalCtx->msgCtx;
    if (D_8014B308 == 0) {
        temp_s1->unk_E3DA = XREG(55);
    } else {
        temp_s1->unk_E3DA = YREG(1);
    }
    if (temp_s1->unk_E2FD == 5) {
        temp_s1->unk_E3DC = temp_s1->unk_E3DE = temp_s1->unk_E3E0 = 0;
    } else {
        temp_s1->unk_E3DC = temp_s1->unk_E3DE = temp_s1->unk_E3E0 = 0xFF;
    }
    temp_s1->unk_E3D0 = 0;
    sp128 = 0;

    for (phi_s2 = 0; phi_s2 < temp_s1->unk_E3D2; phi_s2++) {
        phi_t0 = 9;
        phi_t1 = 0xA;
        phi_t2 = 0xB;
        phi_t3 = 0xC;
        phi_t4 = 0xD;
        phi_t5 = 4;
        temp_v0 = temp_s1->unk_E306[phi_s2];
        temp_t8 = temp_v0 - 1;
        switch(temp_t8) {
        case 0:
            temp_s1->unk_E3D8 = XREG(54);
            if ((temp_s1->unk_E3E6 == 1) || (temp_s1->unk_E3E6 == 3)) {
                temp_s1->unk_E3D8 += 0x20;
            }
            if ((temp_s1->unk_E3E6 == 2) && (temp_s1->unk_E3DA != XREG(55))) {
                temp_s1->unk_E3D8 += 0x20;
            }
            temp_s1->unk_E3DA += XREG(56);
            break;
        case 4:
            func_801076CC(temp_s1, temp_s1->unk_E306[++phi_s2] & 0xF);
            phi_t0 = 9;
            phi_t1 = 0xA;
            phi_t2 = 0xB;
            phi_t3 = 0xC;
            phi_t4 = 0xD;
            phi_t5 = 4;
            break;
        case 31:
            temp_s1->unk_E3D8 += 6;
            break;
        case 3:
            if (temp_s1->msgMode == 6) {
                if ((u8) D_8014B300 == 0) {
                    font = &globalCtx->msgCtx.font;
                    Audio_PlaySoundGeneral(0, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    temp_s1->msgMode = 0x34;
                    Font_LoadMessageBoxEndIcon(font, 0);
                } else {
                    temp_s1->msgMode = 4;
                    temp_s1->unk_E3D6 = 0;
                    temp_s1->unk_E3CE++;
                }
            }
            *gfxP = sp120;
            return;
        case 5:
            temp_s1->unk_E3D8 += temp_s1->unk_E306[++phi_s2];
            break;
        case 6:
            temp_s1->unk_E3E4 = 0x30;
            if (temp_s1->msgMode == 6) {
                font = &globalCtx->msgCtx.font;
                Audio_PlaySoundGeneral(0, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                temp_s1->msgMode = 0x35;
                Font_LoadMessageBoxEndIcon(font, 0);
            }
            *gfxP = sp120;
            return;
        case 7:
            if (temp_s1->unk_E3D2 == (phi_s2 + 1)) {
                phi_a0 = phi_s2;
                if (temp_s1->msgMode == 6 || (temp_s1->msgMode >= 9 && temp_s1->msgMode < 0x21)) {
                    while (true) {
                        temp_v0_6 = temp_s1->unk_E306[phi_a0];
                        if (temp_s1->unk_E306[phi_a0] == 6) {
                            phi_a0 += 2;
                            continue;
                        }
                        if ((temp_s1->unk_E306[phi_a0] != phi_t0) && 
                            (temp_s1->unk_E306[phi_a0] != phi_t1) && 
                            (temp_s1->unk_E306[phi_a0] != phi_t2) && 
                            (temp_s1->unk_E306[phi_a0] != phi_t3) && 
                            (temp_s1->unk_E306[phi_a0] != phi_t4) && 
                            (temp_s1->unk_E306[phi_a0] != phi_t5) && 
                            (temp_s1->unk_E306[phi_a0] != 2)) {
                            phi_a0++;
                            continue;
                        }
                        break;
                    }
                    phi_s2 = phi_a0 - 1;
                    temp_s1->unk_E3D2 = phi_s2 + 1;
                }
            }
        case 8:
            break;
        case 12:
            if (temp_s1->unk_E3D2 == (phi_s2 + 1)) {
                if (temp_s1->msgMode == 6) {
                    font = &globalCtx->msgCtx.font;
                    temp_s1->msgMode = 7;
                    Font_LoadMessageBoxEndIcon(font, 0);
                }
                *gfxP = sp120;
                return;
            }
            break;
        case 11:
            if (temp_s1->msgMode == 6) {
                temp_s1->unk_E3E7 = temp_s1->unk_E306[phi_s2 + 1];
                temp_s1->msgMode = 8;
            }
            *gfxP = sp120;
            return;
        case 16:
            if (temp_s1->msgMode == 6) {
                temp_s1->msgMode = 0x35;
                temp_s1->unk_E3E4 = 0x60;
                osSyncPrintf("タイマー (%x) (%x)", temp_s1->unk_E306[phi_s2 + 1], temp_s1->unk_E306[phi_s2 + 2]); // timer
                temp_s1->unk_E3E7 = temp_s1->unk_E306[phi_s2 + 1] << 8;
                temp_s1->unk_E3E7 |= temp_s1->unk_E306[phi_s2 + 2];
                temp_a1 = temp_s1->unk_E3E7;
                osSyncPrintf("合計wct=%x(%d)\n", temp_a1, temp_a1); // total wct
            }
            *gfxP = sp120;
            return;
        case 17:
            if ((temp_s1->msgMode == 6) && (D_8014B318 == 0)) {
                D_8014B318 = 1;
                osSyncPrintf("サウンド（ＳＥ）\n"); // sfx
                Audio_PlaySoundGeneral((temp_s1->unk_E306[phi_s2 + 1] << 8) | temp_s1->unk_E306[phi_s2 + 2], &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                phi_t0 = 9;
                phi_t1 = 0xA;
                phi_t2 = 0xB;
                phi_t3 = 0xC;
                phi_t4 = 0xD;
                phi_t5 = 4;
            }
            phi_s2 += 2;
            break;
        case 18:
            phi_s2 = func_801080B4(globalCtx, temp_s1->unk_E306[phi_s2 + 1], &sp120, phi_s2);
            phi_t0 = 9;
            phi_t1 = 0xA;
            phi_t2 = 0xB;
            phi_t3 = 0xC;
            phi_t4 = 0xD;
            phi_t5 = 4;
            break;
        case 20:
            if (temp_s1->msgMode == 6) {
                Audio_PlaySoundGeneral(0, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                phi_t0 = 9;
                phi_t1 = 0xA;
                phi_t2 = 0xB;
                phi_t3 = 0xC;
                phi_t4 = 0xD;
                phi_t5 = 4;
            }
            gDPPipeSync(sp120++);
            gDPSetCombineMode(sp120++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetPrimColor(sp120++, 0, 0, D_80153930[temp_s1->unk_E3F9].r, D_80153930[temp_s1->unk_E3F9].g, D_80153930[temp_s1->unk_E3F9].b, temp_s1->unk_E3E2);
            gDPSetTextureImage(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 1, ((u32)temp_s1->textboxSegment + 0x1000));
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPLoadSync(sp120++);
            gDPLoadBlock(sp120++, G_TX_LOADTILE, 0, 0, 1151, 343);
            gDPPipeSync(sp120++);
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_4b, 6, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPSetTileSize(sp120++, G_TX_RENDERTILE, 0, 0, 380, 188);
            gSPTextureRectangle(sp120++, 
                            (temp_s1->unk_E3D8 + 1) * 4, (D_80153948[temp_s1->unk_E3FA] + XREG(61)) * 4, 
                            (temp_s1->unk_E3D8 + 0x61) * 4, (D_80153948[temp_s1->unk_E3FA] + XREG(61) + 0x30) * 4, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400);
            gDPSetTextureImage(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 1, ((u32)temp_s1->textboxSegment + 0x1900));
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPLoadSync(sp120++);
            gDPLoadBlock(sp120++, G_TX_LOADTILE, 0, 0, 1151, 342);
            gDPPipeSync(sp120++);
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_4b, 6, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPSetTileSize(sp120++, G_TX_RENDERTILE, 0, 0, 0x017C, 0x00BC);
            gSPTextureRectangle(sp120++, 
                            (temp_s1->unk_E3D8 + 0x61) * 4, (D_80153948[temp_s1->unk_E3FA] + XREG(61)) * 4, 
                            (temp_s1->unk_E3D8 + 0xC2) * 4, (D_80153948[temp_s1->unk_E3FA] + XREG(61) + 0x30) * 4, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400);
            gDPPipeSync(sp120++);
            temp_v0_13 = &D_80153900[temp_s1->unk_E3F8];
            gDPSetPrimColor(sp120++, 0, 0, temp_v0_13->r, temp_v0_13->g, temp_v0_13->b, temp_s1->unk_E3E2);
            gDPSetTextureImage(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 1, ((u32)temp_s1->textboxSegment + 0x1000));
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPLoadSync(sp120++);
            gDPLoadBlock(sp120++, G_TX_LOADTILE, 0, 0, 1151, 342);
            gDPPipeSync(sp120++);
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_4b, 6, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPSetTileSize(sp120++, G_TX_RENDERTILE, 0, 0, 380, 188);
            gSPTextureRectangle(sp120++, 
                            temp_s1->unk_E3D8 * 4, XREG(61) * 4, 
                            (temp_s1->unk_E3D8 + 0x60) * 4, (XREG(61) + 0x30) * 4, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400);
            gDPSetTextureImage(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 1, ((u32)temp_s1->textboxSegment + 0x1900));
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPLoadSync(sp120++);
            gDPLoadBlock(sp120++, G_TX_LOADTILE, 0, 0, 1151, 342);
            gDPPipeSync(sp120++);
            gDPSetTile(sp120++, G_IM_FMT_I, G_IM_SIZ_4b, 6, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
            gDPSetTileSize(sp120++, G_TX_RENDERTILE, 0, 0, 380, 188);
            gSPTextureRectangle(sp120++, 
                            (temp_s1->unk_E3D8 + 0x60) * 4, XREG(61) * 4, 
                            (temp_s1->unk_E3D8 + 0xC0) * 4, (XREG(61) + 0x30) * 4, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400);
            gDPPipeSync(sp120++);
            gDPSetCombineLERP(sp120++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

            temp_s1->unk_E3D8 += 0x20;
            break;
        case 19:
            temp_s1->unk_E3EA = temp_s1->unk_E306[++phi_s2];
            break;
        case 25:
            temp_s1->unk_E3D6 = 1;
            break;
        case 26:
            temp_s1->unk_E3E4 = 0x10;
            if (temp_s1->msgMode == 6) {
                temp_s1->unk_E3E7 = 4;
                temp_s1->choiceIndex = 0;
                temp_s1->unk_E2FA = temp_s1->unk_E2F8;
                Font_LoadMessageBoxEndIcon(&globalCtx->msgCtx.font, 2);
                phi_t0 = 9;
                phi_t1 = 0xA;
                phi_t2 = 0xB;
                phi_t3 = 0xC;
                phi_t4 = 0xD;
                phi_t5 = 4;
            }
            break;
        case 27:
            temp_s1->unk_E3E4 = 0x20;
            if (temp_s1->msgMode == 6) {
                temp_s1->unk_E3E7 = 4;
                temp_s1->choiceIndex = 0;
                temp_s1->unk_E2FA = temp_s1->unk_E2F8;
                Font_LoadMessageBoxEndIcon(&globalCtx->msgCtx.font, 2);
                phi_t0 = 9;
                phi_t1 = 0xA;
                phi_t2 = 0xB;
                phi_t3 = 0xC;
                phi_t4 = 0xD;
                phi_t5 = 4;
            }
            break;
        case 1:
            if (temp_s1->msgMode == 6) {
                temp_s1->msgMode = 0x35;
                if (temp_s1->unk_E3E4 == 0) {
                    font = &globalCtx->msgCtx.font;
                    Audio_PlaySoundGeneral(NA_SE_SY_MESSAGE_END, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    Font_LoadMessageBoxEndIcon(font, 1);
                    if (globalCtx->csCtx.state == 0) {
                        Interface_SetDoAction(globalCtx, 3);
                    }
                }
            }
            *gfxP = sp120;
            return;
        case 15:
            if (temp_s1->unk_E3D2 == (phi_s2 + 1)) {
                func_801083F8(globalCtx);
                *gfxP = sp120;
                return;
            }
            break;
        case 13:
            if (temp_s1->msgMode == 6) {
                temp_s1->msgMode = 0x35;
                temp_s1->unk_E3E4 = 0x60;
                temp_s1->unk_E3E7 = temp_s1->unk_E306[(u16)(phi_s2 + 1)];
                Font_LoadMessageBoxEndIcon(&globalCtx->msgCtx.font, 1);
                if (globalCtx->csCtx.state == 0) {
                    Interface_SetDoAction(globalCtx, 3);
                }
            }
            *gfxP = sp120;
            return;
        case 9:
            if (temp_s1->msgMode == 6) {
                Audio_PlaySoundGeneral(0, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                temp_s1->msgMode = 0x35;
                temp_s1->unk_E3E4 = 0x40;
            }
            *gfxP = sp120;
            return;
        case 10:
            if (temp_s1->msgMode == 6) {
                temp_s1->msgMode = 0x35;
                temp_s1->unk_E3E4 = 0x50;
                Font_LoadMessageBoxEndIcon(&globalCtx->msgCtx.font, 0);
                Audio_PlaySoundGeneral(0x482E, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
            }
            break;
        default:
            font = &globalCtx->msgCtx.font;
            if ((temp_s1->msgMode == 6) && (temp_s1->unk_E3D2 == (phi_s2 + 1)) && (temp_s1->unk_E3E8 == temp_s1->unk_E3EA)) {
                Audio_PlaySoundGeneral(0, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
            }
            func_80106F1C(globalCtx, (u32)font + sp128 + 8, &sp120);
            sp128 += 128;
            temp_s1->unk_E3D8 += (s16)(D_80153980[temp_v0] * (XREG(57) / 100.0f));
            phi_t0 = 9;
            phi_t1 = 0xA;
            phi_t2 = 0xB;
            phi_t3 = 0xC;
            phi_t4 = 0xD;
            phi_t5 = 4;
            break;
        }
    }
    if (temp_s1->unk_E3E8 == 0) {
        temp_s1->unk_E3D2 = phi_s2 + 1;
        temp_s1->unk_E3E8 = temp_s1->unk_E3EA;
    } else {
        temp_s1->unk_E3E8--;
    }
    *gfxP = sp120;
}
#else
void func_801086B0(GlobalContext* globalCtx, Gfx** gfxP);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_801086B0.s")
#endif

void func_80109968(GlobalContext* globalCtx, u16 arg1, s16 arg2) {
    MessageContext* msgCtx;
    InterfaceContext* interfaceCtx;

    msgCtx = &globalCtx->msgCtx;
    interfaceCtx = &globalCtx->interfaceCtx;
    if (arg1 == 0x76) {
        interfaceCtx->unk_140[30] = 0xFF;
        interfaceCtx->unk_140[31] = 0xFF;
    }
    if (arg1 < 0x66) {
        YREG(71) = XREG(54) - D_80153C40[gSaveContext.language];
        YREG(72) = arg2 + 6;
        YREG(75) = 32;
        DmaMgr_SendRequest1((u32)msgCtx->textboxSegment + 0x1000, 
                            (u32)_icon_item_staticSegmentRomStart + (arg1 * 0x1000), 
                            0x1000, "../z_message_PAL.c", 1473);
        // Item 32-0
        osSyncPrintf("アイテム32-0\n");
    } else {
        YREG(71) = XREG(54) - D_80153C48[gSaveContext.language];
        YREG(72) = arg2 + 10;
        YREG(75) = 24;
        DmaMgr_SendRequest1((u32)msgCtx->textboxSegment + 0x1000, 
                            (u32)_icon_item_24_staticSegmentRomStart + 0xFFFC6A00 + (arg1 * 0x900), 
                            0x900, "../z_message_PAL.c", 1482);
        // Item 24＝%d (%d) {%d}
        osSyncPrintf("アイテム24＝%d (%d) {%d}\n", arg1, arg1 - 0x6C, 0x54);
    }
    msgCtx->unk_E3CE++;
    msgCtx->unk_E3E6 = 1;
}

extern s16 D_8014B304;
extern s16 D_8014B2F0;

// Message_ExpandControlCodes / Message_Decode
#ifdef NON_EQUIVALENT
// Plenty, but general structure seems good
void func_80109B3C(GlobalContext* globalCtx) {
    s16 numLines;
    s16 sp7C[5];
    s16 temp_v0;
    char temp_s1;
    char character;
    Font* font;
    MessageContext* msgCtx;
    s16 phi_s6;
    s16 nameLen;
    s16 phi_s3;
    char phi_s0;
    s32 phi_s5;
    char phi_s1;
    s16 phi_v0;
    s32 phi_s3_2;
    s32 phi_a1_2;
    s32 phi_s3_3;
    s32 phi_v1_3;
    s32 phi_s3_4;
    u32 phi_v0_7;
    u32 phi_s0_3;
    s32 phi_v1_4;
    s32 phi_s3_5;
    s32 phi_s5_5;
    s16 phi_v1_5;
    s32 phi_s3_6;
    f32 phi_f10;
    s32 phi_s3_7;
    s32 phi_s5_8;
    s16 phi_v1_8;
    s32 phi_v0_18;
    s32 phi_s3_8;

    numLines = 0;
    globalCtx->msgCtx.unk_E3D6 = globalCtx->msgCtx.unk_E3EA = globalCtx->msgCtx.unk_E3E8 = 0;
    D_8014B2F0 = 0;
    msgCtx = &globalCtx->msgCtx;
    font = &globalCtx->msgCtx.font;
    phi_s6 = 0;
    phi_s5_8 = 0;

    while (true) {
        character = font->msgBuf[msgCtx->unk_E3CE];
        msgCtx->unk_E306[phi_s6] = character;
        
        if (character == MSG_CHR_BOX_BREAK || character == MSG_CHR_TEXTID || 
                        character == MSG_CHR_BOX_BREAK_DELAYED || character == MSG_CHR_EVENT || 
                        character == MSG_CHR_END) {
            msgCtx->msgMode = 6;
            msgCtx->unk_E3D2 = 1;
            XREG(55) = VREG(1) + 8;
            osSyncPrintf("ＪＪ＝%d\n", numLines);
            if (msgCtx->unk_E2FD != 4) {
                if (numLines == 0) {
                    XREG(55) = VREG(1) + 0x1A;
                } else if (numLines == 1) {
                    XREG(55) = VREG(1) + 0x14;
                } else if (numLines == 2) {
                    XREG(55) = VREG(1) + 0x10;
                }
            }
            if (character == MSG_CHR_TEXTID) {
                osSyncPrintf("NZ_NEXTMSG=%x, %x, %x\n", 
                            font->msgBuf[msgCtx->unk_E3CE], 
                            font->msgBuf[msgCtx->unk_E3CE+1], 
                            font->msgBuf[msgCtx->unk_E3CE+2]);
                phi_s6++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE + 1];
                phi_s6++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE + 2];
                D_8014B304 = ((font->msgBuf[msgCtx->unk_E3CE + 1] << 8) | msgCtx->unk_E306[phi_s6]); // create next text id
            }
            if (character == MSG_CHR_BOX_BREAK_DELAYED) {
                phi_s6++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE + 1];
                msgCtx->unk_E3CE += 2;
            }
            msgCtx->unk_E3D4 = phi_s6;
            if (D_8014B300 != 0) {
                msgCtx->unk_E3D2 = msgCtx->unk_E3D4;
            }
            break;
        } else if (character == MSG_CHR_NAME) {
            for (nameLen = 8; nameLen > 0; nameLen--) {
                if (gSaveContext.playerName[nameLen - 1] != '\x3E') {
                    break;
                }
            }
            osSyncPrintf("\n名前 ＝ "); // Name ＝
            phi_s3 = 0;
            for (phi_s3 = 0; phi_s3 < nameLen; phi_s3++) {
                temp_s1 = gSaveContext.playerName[phi_s3];
                if (temp_s1 == 0x3E) {
                    phi_s0 = phi_s1 = 0x20;
                } else if (temp_s1 == 0x40) {
                    phi_s0 = phi_s1 = 0x2E;
                } else if (temp_s1 == 0x3F) {
                    phi_s0 = phi_s1 = 0x2D;
                } else if (temp_s1 < 0x0A) {
                    phi_s0 = phi_s1 = temp_s1 + 0x30;
                } else if (temp_s1 < 0x24) {
                    phi_s0 = phi_s1 = temp_s1 + 0x37;
                } else if (temp_s1 < 0x3E) {
                    phi_s0 = phi_s1 = temp_s1 + 0x3D;
                } else {
                    phi_s0 = phi_s1 = temp_s1;
                }
                if (phi_s0 != '\x20') {
                    Font_LoadChar(font, phi_s0 - 0x20, phi_s5_8);
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                }
                osSyncPrintf("%x ", phi_s0);
                msgCtx->unk_E306[phi_s6] = phi_s1;
                phi_s6++;
            }
            phi_s6--;
        } else if (character == MSG_CHR_MARATHON_TIME || character == MSG_CHR_RACE_TIME) {
            osSyncPrintf("\nＥＶＥＮＴタイマー ＝ "); // ＥＶＥＮＴ TIMER ＝
            phi_v0 = (character == MSG_CHR_RACE_TIME) ? gSaveContext.timer1Value : gSaveContext.timer2Value;
            for (sp7C[0] = 0, sp7C[1] = 0; phi_v0 >= 60; phi_v0 -= 60) {
                sp7C[1]++;
                if (sp7C[1] >= 10) {
                    sp7C[0]++;
                    sp7C[1] -= 10;
                }
            }
            sp7C[2] = 0;
            for (sp7C[3] = 0; phi_v0 >= 10; phi_v0 -= 10) {
                sp7C[3]++;
            }

            for (phi_s3 = 0; phi_s3 < 4; phi_s3++) {
                Font_LoadChar(font->msgData.offset, sp7C[phi_s3] + 0x10, phi_s5_8);
                phi_s5_8 += FONT_CHAR_TEX_SIZE;
                msgCtx->unk_E306[phi_s6] = sp7C[phi_s3] + 0x30;
                phi_s6++;
                if (phi_s3 == 1) {
                    Font_LoadChar(font->msgData.offset, 2, phi_s5_8);
                    msgCtx->unk_E306[phi_s6] = 0x22;
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                    phi_s6++;
                } else if (phi_s3 == 3) {
                    Font_LoadChar(font->msgData.offset, 2, phi_s5_8);
                    msgCtx->unk_E306[phi_s6] = 0x22;
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                }
            }
        } else if (character == MSG_CHR_SCORE) {
            osSyncPrintf("\n流鏑馬スコア ＝ %d\n", gSaveContext.minigameScore); // Horseback Archery Score ＝ %d
            phi_v0 = gSaveContext.minigameScore;
            for (sp7C[0] = 0; phi_v0 >= 1000; phi_v0 -= 1000) { // thousands
                sp7C[0]++;
            }
            for (sp7C[1] = 0; phi_v0 >= 100; phi_v0 -= 100) { // hundreds
                sp7C[1]++;
            }
            for (sp7C[2] = 0; phi_v0 >= 10; phi_v0 -= 10) { // tens
                sp7C[2]++;
            }
            sp7C[3] = phi_v0; // ones

            for (phi_s3 = 0; phi_s3 < 4; phi_s3++) {
                if (phi_s3 == 3 || sp7C[phi_s3] != 0) { // if the digit is 'ones' or the digit is nonzero
                    Font_LoadChar(font->msgData.offset, sp7C[phi_s3] + 0x10, phi_s5_8);
                    msgCtx->unk_E306[phi_s6] = sp7C[phi_s3] + 0x30;
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                    phi_s6++;
                }
            }
            phi_s6--;
        } else if (character == MSG_CHR_TOKENS) {
            osSyncPrintf("\n金スタ合計数 ＝ %d", gSaveContext.gsTokens); // Total Gold Skulltula Tokens ＝ %d
            phi_v0 = gSaveContext.gsTokens;
            for (sp7C[0] = 0; phi_v0 >= 100; phi_v0 -= 100) { // hundreds
                sp7C[0]++;
            }
            for (sp7C[1] = 0; phi_v0 >= 10; phi_v0 -= 10) { // tens
                sp7C[1]++;
            }
            sp7C[2] = phi_v0; // ones

            for (phi_s3 = 0; phi_s3 < 3; phi_s3++) {
                if (phi_s3 == 2 || sp7C[phi_s3] != 0) { // if the digit is 'ones' or the digit is nonzero
                    Font_LoadChar(font->msgData.offset, sp7C[phi_s3] + 0x10, phi_s5_8);
                    msgCtx->unk_E306[phi_s6] = sp7C[phi_s3] + 0x30;
                    osSyncPrintf("%x(%x) ", sp7C[phi_s3] + 0x10, sp7C[phi_s3]);
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                    phi_s6++;
                }
            }
            phi_s6--;
        } else if (character == MSG_CHR_FISH_INFO) {
            osSyncPrintf("\n釣り堀魚サイズ ＝ "); // Fishing Pond Fish Size ＝
            phi_v0 = gSaveContext.minigameScore;
            for (sp7C[0] = 0; phi_v0 >= 10; phi_v0 -= 10) { // tens
                sp7C[0]++;
            }
            sp7C[1] = phi_v0; // ones

            for (phi_s3 = 0; phi_s3 < 2; phi_s3++) {
                if (phi_s3 == 1 || sp7C[phi_s3] != 0) {
                    Font_LoadChar(font->msgData.offset, sp7C[phi_s3] + 0x10, phi_s5_8);
                    msgCtx->unk_E306[phi_s6] = sp7C[phi_s3] + 0x30;
                    osSyncPrintf("%x(%x) ", sp7C[phi_s3] + 0x10, sp7C[phi_s3]);
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                    phi_s6++;
                }
            }
            phi_s6--;
        } else if (character == MSG_CHR_HIGHSCORE) {
            msgCtx->unk_E3CE++;
            phi_s0_3 = HI_SCORE(font->msgBuf[msgCtx->unk_E3CE]);
            osSyncPrintf("ランキング＝%d\n", font->msgBuf[msgCtx->unk_E3CE]); // Ranking＝%d
            phi_v0_7 = font->msgBuf[msgCtx->unk_E3CE];
            if (phi_v0_7 == 2) {
                if (LINK_AGE_IN_YEARS == YEARS_CHILD) {
                    phi_s0_3 &= 0x7F;
                } else {
                    osSyncPrintf("HI_SCORE( kanfont->mbuff.nes_mes_buf[message->rdp] & 0xff000000 ) = %x\n", 
                                HI_SCORE(font->msgBuf[msgCtx->unk_E3CE]) & 0xFF000000);
                    phi_s0_3 = ((HI_SCORE(font->msgBuf[msgCtx->unk_E3CE]) & 0xFF000000) >> 0x18) & 0x7F;
                }
                phi_s0_3 = SQ(phi_s0_3) * 0.0036f + 0.5f;
                osSyncPrintf("score=%d\n", phi_s0_3);
            }
            switch (phi_v0_7) {
            case 0:
            case 1:
            case 2:
                for (sp7C[0] = 0; phi_s0_3 >= 1000; phi_s0_3 -= 1000) { // thousands
                    sp7C[0]++;
                }
                for (sp7C[1] = 0; phi_s0_3 >= 100; phi_s0_3 -= 100) { // hundreds
                    sp7C[1]++;
                }
                for (sp7C[2] = 0; phi_s0_3 >= 10; phi_s0_3 -= 10) { // tens
                    sp7C[2]++;
                }
                sp7C[3] = phi_s0_3; // ones

                for (phi_s3 = 0; phi_s3 < 4; phi_s3++) {
                    if (phi_s3 == 3 || sp7C[phi_s3] != 0) {
                        Font_LoadChar(font->msgData.offset, sp7C[phi_s3] + 0x10, phi_s5_8);
                        msgCtx->unk_E306[phi_s6] = sp7C[phi_s3] + 0x30;
                        phi_s5_8 += FONT_CHAR_TEX_SIZE;
                        phi_s6++;
                    }
                }
                phi_s6--;
                break;
            case 5:
                break;
            case 3:
            case 4:
            case 6:
                for (sp7C[0] = 0, sp7C[1] = 0; phi_s0_3 >= 60; phi_s0_3 -= 60) {
                    sp7C[1]++;
                    if (sp7C[1] >= 10) {
                        sp7C[0]++;
                        sp7C[1] -= 10;
                    }
                }
                for (sp7C[2] = 0; phi_s0_3 >= 10; phi_s0_3 -= 10) {
                    sp7C[2]++;
                }
                sp7C[3] = phi_s0_3;

                for (phi_s3 = 0; phi_s3 < 4; phi_s3++) {
                    Font_LoadChar(font->msgData.offset, sp7C[phi_s3] + 0x10, phi_s5_8);
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                    msgCtx->unk_E306[phi_s6] = sp7C[phi_s3] + 0x30;
                    phi_s6++;
                    if (phi_s3 == 1) {
                        Font_LoadChar(font->msgData.offset, 2, phi_s5_8);
                        msgCtx->unk_E306[phi_s6] = 0x22;
                        phi_s5_8 += FONT_CHAR_TEX_SIZE;
                        phi_s6++;
                    } else if (phi_s3 == 3) {
                        Font_LoadChar(font->msgData.offset, 2, phi_s5_8);
                        msgCtx->unk_E306[phi_s6] = 0x22;
                        phi_s5_8 += FONT_CHAR_TEX_SIZE;
                    }
                }
                break;
            }
        } else if (character == MSG_CHR_TIME) {
            osSyncPrintf("\nゼルダ時間 ＝ "); // Zelda Time ＝
            phi_f10 = gSaveContext.dayTime;
            temp_v0 = phi_f10 / 60.0f;
            for (sp7C[0] = 0; temp_v0; temp_v0 -= 10) {
                sp7C[0]++;
            }
            sp7C[1] = temp_v0;
            temp_v0 = (s16)phi_f10 % 60;
            for (sp7C[2] = 0; temp_v0 >= 10; temp_v0 -= 10) {
                sp7C[2]++;
            }
            sp7C[3] = temp_v0;

            for (phi_s3 = 0; phi_s3 < 4; phi_s3++) {
                Font_LoadChar(font->msgData.offset, sp7C[phi_s3] + 0x10, phi_s5_8);
                phi_s5_8 += FONT_CHAR_TEX_SIZE;
                msgCtx->unk_E306[phi_s6] = sp7C[phi_s3] + 0x30;
                phi_s6++;
                if (phi_s3 == 1) {
                    Font_LoadChar(font->msgData.offset, 0x1A, phi_s5_8);
                    msgCtx->unk_E306[phi_s6] = 0x3A;
                    phi_s5_8 += FONT_CHAR_TEX_SIZE;
                    phi_s6++;
                }
            }
            phi_s6--;
        } else if (character == MSG_CHR_ITEM_ICON) {
            phi_s6++;
            msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE + 1];
            osSyncPrintf("ITEM_NO=(%d) (%d)\n", msgCtx->unk_E306[phi_s6], font->msgBuf[msgCtx->unk_E3CE + 1]);
            func_80109968(globalCtx, font->msgBuf[msgCtx->unk_E3CE + 1], VREG(1) + 0xA);
        } else if (character == MSG_CHR_BACKGROUND) {
            msgCtx->unk_E3F6 = font->msgBuf[msgCtx->unk_E3CE + 1];
            msgCtx->unk_E3F8 = (font->msgBuf[msgCtx->unk_E3CE + 2] & 0xF0) >> 4;
            msgCtx->unk_E3F9 = (font->msgBuf[msgCtx->unk_E3CE + 2] & 0x0F);
            msgCtx->unk_E3FA = (font->msgBuf[msgCtx->unk_E3CE + 3] & 0xF0) >> 4;
            msgCtx->unk_E3FB = (font->msgBuf[msgCtx->unk_E3CE + 3] & 0x0F);
            numLines = 2;
            DmaMgr_SendRequest1((u32)(msgCtx->textboxSegment) + 0x1000, 
                            (u32) (_message_texture_staticSegmentRomStart + (msgCtx->unk_E3F6 * 0x900)), 0x900U, 
                            "../z_message_PAL.c", 1830);
            DmaMgr_SendRequest1((u32)(msgCtx->textboxSegment) + 0x1900, 
                            (u32) ((_message_texture_staticSegmentRomStart + (msgCtx->unk_E3F6 * 0x900)) + 0x900), 0x900U, 
                            "../z_message_PAL.c", 1834);
            msgCtx->unk_E3CE+= 3;
            XREG(61) += 8;
            XREG(54) = 50;
        } else if (character == MSG_CHR_COLOR) {
            phi_s6++;
            msgCtx->unk_E3CE++;
            msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE];
        } else if (character == MSG_CHR_NEWLINE) {
            numLines++;
        } else if (character != MSG_CHR_QUICKTEXT_ENABLE && character != MSG_CHR_QUICKTEXT_DISABLE && 
                    character != MSG_CHR_AWAIT_BUTTON_PRESS && character != MSG_CHR_OCARINA && 
                    character != MSG_CHR_PERSISTENT && character != MSG_CHR_UNSKIPPABLE) {
            if (character == MSG_CHR_FADE) {
                D_8014B2F0 = 1;
                osSyncPrintf("NZ_TIMER_END (key_off_flag=%d)\n", D_8014B2F0);
                phi_s6++;
                msgCtx->unk_E3CE++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE];
            } else if (character == MSG_CHR_FADE2) {
                D_8014B2F0 = 1;
                osSyncPrintf("NZ_BGM (key_off_flag=%d)\n", D_8014B2F0);
                phi_s6++;
                msgCtx->unk_E3CE++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE];
                phi_s6++;
                msgCtx->unk_E3CE++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE];
            } else if ((character == MSG_CHR_SHIFT) || (character == MSG_CHR_TEXT_SPEED)) {
                phi_s6++;
                msgCtx->unk_E3CE++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE];
            } else if (character == MSG_CHR_SFX) {
                phi_s6++;
                msgCtx->unk_E3CE++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE];
                phi_s6++;
                msgCtx->unk_E3CE++;
                msgCtx->unk_E306[phi_s6] = font->msgBuf[msgCtx->unk_E3CE];
            } else if (character == MSG_CHR_TWO_CHOICE) {
                msgCtx->unk_E3E6 = 2;
            } else if (character == MSG_CHR_THREE_CHOICE) {
                msgCtx->unk_E3E6 = 3;
            } else if (character != 0x20) { // not space
                Font_LoadChar(font->msgData.offset, character - 0x20, phi_s5_8);
                phi_s5_8 += FONT_CHAR_TEX_SIZE;
            }
        }
        msgCtx->unk_E3CE++;
        phi_s6++;
    }
}
#else
void func_80109B3C(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_80109B3C.s")
#endif

extern volatile s8 D_8014B2F4;

#ifdef NON_MATCHING
// (ro)data loads/stores at the start, rodata unmigrated
void func_8010B0C0(GlobalContext* globalCtx, u16 textId) {
    static const s16 D_80153C50[] = {
        0x0000, 0x0001, 0x0003, 0x0002,
    };

    MessageContext* msgCtx = &globalCtx->msgCtx;
    Font* sp30 = &msgCtx->font;
    s16 tmp;

    if (msgCtx->msgMode == 0) {
        gSaveContext.unk_13EE = gSaveContext.unk_13EA;
    }
    if (YREG(15) == 0x10) {
        Interface_ChangeAlpha(5);
    }

    D_8014B308 = 0;
    D_8014B2F4 = D_8014B300 = D_8014B308;
    D_8014B318 = D_8014B308 & 0xFF;

    if (textId >= 0x0500 && textId < 0x0600) { // text ids 0500 to 0600 are reserved for credits
        D_8014B308 = 1;
        XREG(57) = 0x55;
        XREG(56) = 6;
        XREG(54) = 0x14;
        YREG(1) = 0x30;
    } else {
        XREG(57) = 0x4B;
        XREG(56) = 0xC;
        XREG(54) = 0x41;
    }
    if (textId == 0xC2 || textId == 0xFA) { // C2 = One piece of heart , FA = WINNER One piece of heart
        // Increments text id based on piece of heart count, assumes the piece of heart text is all
        // in order and that you don't have more than the intended amount of heart pieces.
        textId += (gSaveContext.inventory.questItems & 0xF0000000 & 0xF0000000) >> 0x1C;
    } else if (msgCtx->unk_E2F8 == 0xC && CHECK_OWNED_EQUIP(EQUIP_SWORD,2)) {
        textId = 0xB; // Traded Giant's Knife for Biggoron Sword
    } else if (msgCtx->unk_E2F8 == 0xB4 && (gSaveContext.eventChkInf[9] & 0x40)) {
        textId = 0xB5; // Destroyed Gold Skulltula
    }
    // Ocarina Staff + Dialog
    if (textId == 0x4077 || // Pierre?
        textId == 0x407A || // Pierre?
        textId == 0x2061 || // Learning Epona's Song
        textId == 0x5035 || // Guru-Guru in Windmill
        textId == 0x40AC) { // Ocarina Frog Minigame
        Interface_ChangeAlpha(1);
    }
    msgCtx->unk_E2F8 = textId;
    if (textId == 0x2030) { // Talking to Ingo as adult in Lon Lon Ranch for the first time before freeing Epona
        osSyncPrintf("\x1b[33m"); // VT_FGCOL(yellow)
        osSyncPrintf("？？？？？？？？？？？？？？？？  z_message.c  ？？？？？？？？？？？？？？？？？？\n");
        osSyncPrintf("\x1b[m"); // VT_RST
        gSaveContext.eventInf[0] = gSaveContext.eventInf[1] =  gSaveContext.eventInf[2] = gSaveContext.eventInf[3] = 0;
    }
    
    if (D_8014B308 != 0) {
        func_80107628(globalCtx, textId);
        msgCtx->unk_E300 = sp30->msgLength;
        DmaMgr_SendRequest1((u32)sp30->msgBuf, 
                        (u32)(_staff_message_data_staticSegmentRomStart + sp30->msgOffset), 
                        sp30->msgLength, "../z_message_PAL.c", 1954);
    } else {
        if (gSaveContext.language == LANGUAGE_ENG) {
            func_80107448(globalCtx, textId);
            msgCtx->unk_E300 = sp30->msgLength;
            DmaMgr_SendRequest1((u32)sp30->msgBuf, 
                            (u32)(_nes_message_data_staticSegmentRomStart + sp30->msgOffset), 
                            sp30->msgLength, "../z_message_PAL.c", 1966);
        } else if (gSaveContext.language == LANGUAGE_GER) {
            func_80107448(globalCtx, textId);
            msgCtx->unk_E300 = sp30->msgLength;
            DmaMgr_SendRequest1((u32)sp30->msgBuf, 
                            (u32)(_ger_message_data_staticSegmentRomStart + sp30->msgOffset), 
                            sp30->msgLength, "../z_message_PAL.c", 1978);
        } else {
            func_80107448(globalCtx, textId);
            msgCtx->unk_E300 = sp30->msgLength;
            DmaMgr_SendRequest1((u32)sp30->msgBuf, 
                            (u32)(_fra_message_data_staticSegmentRomStart + sp30->msgOffset), 
                            sp30->msgLength, "../z_message_PAL.c", 1990);
        }
    }
    msgCtx->unk_E2FC = sp30->xy;
    msgCtx->unk_E2FD = msgCtx->unk_E2FC >> 4;
    msgCtx->unk_E2FE = msgCtx->unk_E2FC & 0xF;
    tmp = msgCtx->unk_E2FD;
    // Text Box Type＝%d
    osSyncPrintf("吹き出し種類＝%d\n",  msgCtx->unk_E2FD);
    if (tmp < 4) {
        DmaMgr_SendRequest1(msgCtx->textboxSegment, 
                        (u32)(_message_staticSegmentRomStart + (D_80153C50[tmp] * 0x1000)), 
                        0x1000, "../z_message_PAL.c", 2006);
        if (tmp == 0) {
            msgCtx->unk_E3FE = 0;
            msgCtx->unk_E400 = 0;
            msgCtx->unk_E402 = 0;
        } else if (tmp == 1) {
            msgCtx->unk_E3FE = 0x46;
            msgCtx->unk_E400 = 0x32;
            msgCtx->unk_E402 = 0x1E;
        } else if (tmp == 2) {
            msgCtx->unk_E3FE = 0;
            msgCtx->unk_E400 = 0xA;
            msgCtx->unk_E402 = 0x32;
        } else {
            msgCtx->unk_E3FE = 0xFF;
            msgCtx->unk_E400 = 0;
            msgCtx->unk_E402 = 0;
        }
        if (tmp == 1) {
            msgCtx->unk_E404 = 0xE6;
        } else if (tmp == 3) {
            msgCtx->unk_E404 = 0xB4;
        } else {
            msgCtx->unk_E404 = 0xAA;
        }
        msgCtx->unk_E406 = 0;
    }
    msgCtx->unk_E3E6 = msgCtx->unk_E3D6 = msgCtx->unk_E3E4 = 0;
    msgCtx->unk_E3CE = msgCtx->unk_E3D0 = msgCtx->unk_E3D2 = 0;
}
#else
void func_8010B0C0(GlobalContext*, u16);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_8010B0C0.s")
#endif

// Message_Begin
void func_8010B680(GlobalContext* globalCtx, u16 textId, Actor* actor) {
    MessageContext* msgCtx = &globalCtx->msgCtx;

    osSyncPrintf("\x1b[34m");
    // Message＝%x(%d)
    osSyncPrintf("めっせーじ＝%x(%d)\n", textId, actor);
    osSyncPrintf("\x1b[m");
    msgCtx->unk_E3F0 = 0xFFFF;
    func_8010B0C0(globalCtx, textId);
    msgCtx->unk_E408 = actor;
    msgCtx->msgMode = 1;
    msgCtx->unk_E3E7 = 0;
    msgCtx->unk_E3E8 = 0;
    globalCtx->msgCtx.unk_E3EE = 0;
}

// Message_Continue
void func_8010B720(GlobalContext* globalCtx, u16 textId) {
    MessageContext* msgCtx = &globalCtx->msgCtx;

    osSyncPrintf("\x1b[32m");
    // Message＝%x message->msg_data
    osSyncPrintf("めっせーじ＝%x  message->msg_data\n", textId, msgCtx->unk_E300);
    osSyncPrintf("\x1b[m");
    msgCtx->unk_E300 = 0;
    func_8010B0C0(globalCtx, textId);
    msgCtx->msgMode = 0;
    msgCtx->unk_E406 = msgCtx->unk_E404;
    msgCtx->msgMode = 5;
    msgCtx->unk_E3E7 = 3;
    msgCtx->unk_E3E4 = msgCtx->unk_E3CE = msgCtx->unk_E3D0 = msgCtx->unk_E3D2 = msgCtx->unk_E3E8 = 0;
    msgCtx->unk_E3E2 = 0xFF;
    if (YREG(31) == 0 && globalCtx->interfaceCtx.unk_1FA == 0) {
        Interface_SetDoAction(globalCtx, 0x10);
    }
    msgCtx->unk_E406 = msgCtx->unk_E404;
}

#ifdef NON_MATCHING
// Matches but rodata unmigrated
static const s16 D_8014B314[1] = {0};
void func_8010B820(GlobalContext* globalCtx, u16 ocarinaActionId) {
    static const u16 D_80153C58[] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x10, 0x80,
    };

    MessageContext* msgCtx = &globalCtx->msgCtx;
    s32 phi_s0_2;
    s16 phi_v0_3;
    s16 phi_s0;
    s16 sp4A;
    s32 phi_s0_3;

    osSyncPrintf("\x1b[32m");
    for (phi_s0 = D_8014B31C = 0; phi_s0 < 0xC; phi_s0++) {
        if (CHECK_QUEST_ITEM(phi_s0 + 0x6)) {
            osSyncPrintf("ocarina_check_bit[%d]=%x\n", phi_s0, D_80153C58[phi_s0]);
            D_8014B31C |= D_80153C58[phi_s0];
        }
    }
    if (gSaveContext.scarecrowSpawnSongSet != 0) {
        D_8014B31C |= 0x1000;
    }
    osSyncPrintf("ocarina_bit = %x\n", D_8014B31C);
    osSyncPrintf("\x1b[m");
    NON_CONST(D_8014B314,s16) = CHECK_QUEST_ITEM(QUEST_SONG_SUN);
    msgCtx->unk_E2B8 = func_800EE3C8();
    msgCtx->unk_E2B8->unk_2 = NON_CONST(D_8014B2F8,s16) = 0;
    NON_CONST(D_8014B2FC,s16) = 0;
    func_801069B0();
    msgCtx->unk_E410 = 0xFF;
    D_8014B310 = msgCtx->unk_E3F2 = msgCtx->unk_E410;

    // ☆☆☆☆☆ Ocarina Number＝%d(%d) ☆☆☆☆☆
    osSyncPrintf("\x1b[31m☆☆☆☆☆ オカリナ番号＝%d(%d) ☆☆☆☆☆\n\x1b[m", ocarinaActionId, 2);
    sp4A = 0;
    if (ocarinaActionId >= 0x893) {
        func_8010B0C0(globalCtx, ocarinaActionId);
        phi_s0_2 = ocarinaActionId + 0x86E;
    } else if (ocarinaActionId == 0x2E) {
        msgCtx->unk_E3F0 = ocarinaActionId;
        func_8010B0C0(globalCtx, 0x86D);
        phi_s0_2 = ocarinaActionId + 0x86E;
    } else if (ocarinaActionId == 1 || ocarinaActionId >= 0x22) {
        osSyncPrintf("ocarina_set 000000000000000000  = %d\n", ocarinaActionId);
        msgCtx->unk_E3F0 = ocarinaActionId;
        if (ocarinaActionId >= 0x22 && ocarinaActionId < 0x28) {
            Audio_PlaySoundGeneral(NA_SE_SY_TRE_BOX_APPEAR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
        }
        if (ocarinaActionId == 0x2D) {
            func_8010B0C0(globalCtx, 0x86F);
            phi_s0_2 = ocarinaActionId + 0x86E;
        } else {
            func_8010B0C0(globalCtx, 0x86E);
            phi_s0_2 = ocarinaActionId + 0x86E;
        }
    } else {
        msgCtx->unk_E3F0 = ocarinaActionId;
        sp4A = 1;
        if (ocarinaActionId >= 0xF) {
            osSyncPrintf("222222222\n");
            func_8010B0C0(globalCtx, 0x86D);
            phi_s0_2 = ocarinaActionId + 0x86E;
        } else {
            osSyncPrintf("333333333\n");
            phi_s0_2 = ocarinaActionId + 0x86E;
            func_8010B0C0(globalCtx, phi_s0_2);
        }
    }
    msgCtx->unk_E408 = NULL;
    // Ocarina Mode
    osSyncPrintf("オカリナモード = %d  (%x)\n", msgCtx->unk_E3F0, phi_s0_2);
    msgCtx->unk_E3E8 = 0;
    globalCtx->msgCtx.unk_E3EE = 0;
    VREG(0) = 0x22;
    VREG(1) = 0x8E;
    YREG(16) = 0x200;
    YREG(17) = 0x200;
    YREG(22) = 0x100;
    YREG(23) = 0x40;
    msgCtx->unk_E3E7 = 0;
    if (YREG(12) != 0) {
        msgCtx->msgMode = 4;
    } else {
        msgCtx->unk_E3E7 =2;
        msgCtx->msgMode = 5;
    }
    msgCtx->unk_E406 = msgCtx->unk_E404;
    if (sp4A == 0) {
        Interface_LoadActionLabelB(globalCtx, 0x12);
        sp4A = gSaveContext.unk_13EA;
        Interface_ChangeAlpha(0xA);
        gSaveContext.unk_13EA = sp4A;
    }
    // Music Performance Start
    osSyncPrintf("演奏開始\n");
    if (ocarinaActionId == 1 || ocarinaActionId == 0x30) {
        msgCtx->msgMode = 9;
        msgCtx->unk_E2FD = 0x63;
    } else if (ocarinaActionId == 0x2F) {
        msgCtx->msgMode = 0x31;
        msgCtx->unk_E2FD = 2;
    } else if (ocarinaActionId == 0x2E) {
        Interface_ChangeAlpha(1);
        func_80109B3C(globalCtx);
        msgCtx->msgMode = 0x29;
    } else if (ocarinaActionId == 0x2B) {
        // ?????Recording Playback / Recording Playback / Recording Playback / Recording Playback ->
        osSyncPrintf("?????録音再生 録音再生 録音再生 録音再生  -> ");
        func_800ED858(1);
        func_800ED858(1);
        msgCtx->unk_E2B8 = func_800EE3F8();
        NON_CONST(D_8014B2F8,s16) = NON_CONST(D_8014B2FC,s16) = 0;
        msgCtx->unk_E2B8->unk_2 = D_8014B2F8;
        func_801069B0();
        msgCtx->unk_E3E7 = 3;
        msgCtx->msgMode = 0x23;
        func_800ED93C(0xF, 1);
        gSaveContext.unk_13EA = 0;
        Interface_ChangeAlpha(1);
    }
    for (phi_s0_3 = 0, phi_v0_3 = 0; phi_v0_3 < 0x30; phi_v0_3++, phi_s0_3 += 0x80) {
        func_8006EE50(&globalCtx->msgCtx.font, 0x8140, phi_s0_3);
    }
}
#else
void func_8010B820(GlobalContext*, u16);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_8010B820.s")
#endif

void func_8010BD58(GlobalContext* globalCtx, u16 arg1) {
    globalCtx->msgCtx.unk_E40E = 0;
    func_8010B820(globalCtx, arg1);
}

void func_8010BD88(GlobalContext* globalCtx, u16 arg1) {
    globalCtx->msgCtx.unk_E40E = 1;
    func_8010B820(globalCtx, arg1);
}

// Talk states?
#ifdef NON_MATCHING
// Tiny regalloc at the end
u8 func_8010BDBC(MessageContext* msgCtx) {
    if (msgCtx->unk_E300 == 0) {
        return 0;
    } else if (msgCtx->msgMode == 0x35) {
        if (msgCtx->unk_E3E4 == 0x30) {
            return 1;
        } else if (msgCtx->unk_E3E4 == 0x10 || msgCtx->unk_E3E4 == 0x20) {
            return 4;
        } else if (msgCtx->unk_E3E4 == 0x50 || msgCtx->unk_E3E4 == 0x40) {
            return 5;
        } else if (msgCtx->unk_E3E4 == 0x60) {
            return 3;
        } else {
            return 6;
        }
    } else if (msgCtx->msgMode == 0x34) {
        return 10;
    } else if (msgCtx->msgMode == 0x1A) {
        return 7;
    } else if (msgCtx->unk_E3EE == 3) {
        return 8;
    } else if (msgCtx->msgMode == 0x1F) {
        return 9;
    } else if (msgCtx->msgMode == 0x36 && msgCtx->unk_E3E7 == 1) {
        return 2;
    } else {
        return 3;
    }
}
#else
u8 func_8010BDBC(MessageContext* msgCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_8010BDBC.s")
#endif

#ifdef NON_MATCHING
// Very small reorderings, regalloc
void func_8010BED8(GlobalContext* globalCtx, Gfx** p) {
    MessageContext* msgCtx = &globalCtx->msgCtx;
    Gfx* gfx = *p;

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, msgCtx->unk_E3FE, msgCtx->unk_E400, msgCtx->unk_E402, msgCtx->unk_E406);

    if (msgCtx->unk_E2FD == 0 || msgCtx->unk_E2FD == 2) {
        gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, (u32)msgCtx->textboxSegment);
        gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_MIRROR | G_TX_WRAP, 7, G_TX_NOLOD);
        gDPLoadSync(gfx++);
        gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 2047, 256);
        gDPPipeSync(gfx++);
        gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_4b, 8, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_MIRROR | G_TX_WRAP, 7, G_TX_NOLOD);
        gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, 0x01FC, 0x00FC);
    } else {
        if (msgCtx->unk_E2FD == 3) {
            gDPSetEnvColor(gfx++, 0x00, 0x00, 0x00, 0xFF);
        } else {
            gDPSetEnvColor(gfx++, 0x32, 0x14, 0x00, 0xFF);
        }
        gDPSetTextureImage(gfx++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, (u32)msgCtx->textboxSegment);
        gDPSetTile(gfx++, G_IM_FMT_IA, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_MIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_MIRROR | G_TX_WRAP, 7, G_TX_NOLOD);
        gDPLoadSync(gfx++);
        gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 2047, 256);
        gDPPipeSync(gfx++);
        gDPSetTile(gfx++, G_IM_FMT_IA, G_IM_SIZ_4b, 8, 0x0000, G_TX_RENDERTILE, 0, G_TX_MIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_MIRROR | G_TX_WRAP, 7, G_TX_NOLOD);
        gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, 0x01FC, 0x00FC);
    }
    gSPTextureRectangle(gfx++, VREG(0)*4, VREG(1)*4, (VREG(0) + YREG(22))*4, (VREG(1) + YREG(23))*4, 0, 0, 0, YREG(16)*2, YREG(17)*2);
    if (msgCtx->unk_E2FD == 3) {
        gDPPipeSync(gfx++);
        gDPSetCombineLERP(gfx++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0x64, 0x00, 0xFF);
        gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, &D_02002E40);
        gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_MIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_MIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        if (1) {}
        gDPLoadSync(gfx++);
        gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 127, 2048);
        gDPPipeSync(gfx++);
        gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_MIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_MIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, 0x003C, 0x007C);
        gSPTextureRectangle(gfx++, VREG(7)*4, VREG(8)*4, (u16)(VREG(7) + 0x10)*4, (u16)(VREG(8) + 0x20)*4, 0, 0, 0, 0x0400, 0x0400);
    }
    *p = gfx;
}
#else
void func_8010BED8(GlobalContext* globalCtx, Gfx** p);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_8010BED8.s")
#endif

// Message_SetView ?
void func_8010C358(View* view) {
    SET_FULLSCREEN_VIEWPORT(view);
    func_800AB2C4(view);
}

extern s32 D_80153C94[];

extern s16 D_80153C68[];

extern s16 D_80153C78[];

extern s16 D_80153CE0[];

extern s16 D_8015396C[];

extern s16 D_80153CDC;
extern s16 D_80153CD8;

extern Vec3s D_80153CA8[];
extern Vec3s D_80153CB4[];
extern Vec3s D_80153CC0[];
extern Vec3s D_80153CCC[];

typedef struct {
    u8 len;
    u8 notesIdx[8];
} OcarinaSongInfo;

extern OcarinaSongInfo D_80131C00[];

// Message_DrawMain ?
void func_8010C39C(GlobalContext* globalCtx, Gfx** p);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_8010C39C.s")
/*
void func_8010C39C(GlobalContext *globalCtx, Gfx **p) {
    Player *sp148;
    Gfx *sp140;
    u16 sp138;
    u16 sp132;
    Font *sp90;
    s32 sp88;
    s32 sp80;
    s32 sp7C;
    s32 sp78;
    s32 sp70;
    s32 sp6C;
    s32 sp64;
    s32 sp5C;
    s32 sp54;
    MessageContext *temp_s1;
    s16 temp_a0_3;
    s16 temp_a0_4;
    s16 temp_a0_5;
    s16 temp_a0_6;
    s16 temp_a0_7;
    s16 temp_a0_8;
    s16 temp_a1_10;
    s16 temp_a1_11;
    s16 temp_a1_12;
    s16 temp_a1_5;
    s16 temp_a1_6;
    s16 temp_a1_7;
    s16 temp_a1_8;
    s16 temp_a1_9;
    s16 temp_a2_2;
    s16 temp_a2_3;
    s16 temp_a2_4;
    s16 temp_a2_5;
    s16 temp_a3_3;
    s16 temp_a3_4;
    s16 temp_a3_5;
    s16 temp_a3_6;
    s16 temp_ra;
    s16 temp_t0_2;
    s16 temp_t0_3;
    s16 temp_t0_4;
    s16 temp_t0_5;
    s16 temp_t1;
    s16 temp_t1_2;
    s16 temp_t1_3;
    s16 temp_t1_4;
    s16 temp_t2;
    s16 temp_t3;
    s16 temp_t3_2;
    s16 temp_t3_3;
    s16 temp_t4;
    s16 temp_t5;
    s16 temp_t8;
    s16 temp_t8_2;
    s16 temp_t9_4;
    s16 temp_t9_5;
    s16 temp_v0_10;
    s16 temp_v0_27;
    s16 temp_v1_22;
    s32 temp_a3_10;
    s32 temp_a3_12;
    s32 temp_a3_20;
    s32 temp_a3_21;
    s32 temp_a3_7;
    s32 temp_t9_3;
    s32 temp_v0_12;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s32 temp_v1_12;
    s32 temp_v1_13;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 temp_v1_9;
    u16 temp_a3;
    u16 temp_a3_11;
    u16 temp_a3_14;
    u16 temp_v0_13;
    u16 temp_v0_14;
    u16 temp_v0_23;
    u16 temp_v0_24;
    u16 temp_v0_6;
    u16 temp_v0_8;
    u16 temp_v0_9;
    u16 temp_v1_14;
    u16 temp_v1_15;
    u16 temp_v1_16;
    u32 temp_t9;
    u8 temp_a0;
    u8 temp_a0_2;
    u8 temp_a2;
    u8 temp_a2_6;
    u8 temp_a2_7;
    u8 temp_a2_8;
    u8 temp_a2_9;
    u8 temp_t0_9;
    u8 temp_v0_11;
    u8 temp_v0_17;
    u8 temp_v0_18;
    u8 temp_v0_19;
    u8 temp_v0_20;
    u8 temp_v0_21;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 temp_v0_7;
    u8 temp_v1;
    u8 temp_v1_17;
    u8 temp_v1_18;
    u8 temp_v1_19;
    u8 temp_v1_20;
    u8 temp_v1_21;
    u8 temp_v1_23;
    u8 temp_v1_24;
    u8 temp_v1_25;
    u8 temp_v1_26;
    u8 temp_v1_27;
    u8 temp_v1_28;
    u8 temp_a0_9;
    u16 phi_a2;
    u16 phi_a3;
    s8 phi_t8;
    u8 phi_a0;
    SubMessageContext_E2B8 *phi_a3_2;
    s32 phi_v0;
    s32 phi_v0_2;
    s32 phi_v0_3;
    s32 phi_v0_4;
    s32 phi_v0_5;
    s16 phi_t9;
    s32 phi_v0_6;
    s32 phi_v0_7;
    s16 phi_t9_2;
    s32 phi_v0_8;
    s32 phi_v0_9;
    s16 phi_t8_2;
    s32 phi_v1;
    u16 phi_a3_3;
    u16 phi_v0_10;
    s32 phi_a3_4;
    u16 phi_a3_5;
    u16 phi_v0_11;
    u16 phi_a3_6;
    u16 phi_a3_7;
    s32 phi_v0_12;
    s32 phi_a3_8;
    u16 phi_a1;
    u16 phi_a3_9;
    u16 phi_a1_2;
    s32 phi_v0_13;
    s32 phi_v0_14;
    s32 phi_v0_15;

    sp148 = PLAYER;
    sp140 = *p;
    temp_s1 = &globalCtx->msgCtx;

    gSPSegment(sp140++, 0x02, globalCtx->interfaceCtx.parameterSegment);
    gSPSegment(sp140++, 0x07, temp_s1->textboxSegment);

    if (temp_s1->unk_E300 != 0) {
        if (temp_s1->unk_E3F0 != 0x2F) {
            if ((temp_s1->msgMode != 0x17) && (temp_s1->msgMode >= 2) && (temp_s1->msgMode < 0x36) && (temp_s1->unk_E2FD < 4)) {
                func_8010C358(temp_s1);
                func_8009457C(&sp140);
                func_8010BED8(globalCtx, &sp140);
            }
        }
        func_8009457C(&sp140);

        gDPSetAlphaCompare(sp140++, G_AC_NONE);
        gDPSetCombineLERP(sp140++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

        switch (temp_s1->msgMode) {
            default:
                break;
            case 5:
                if (temp_s1->unk_E3E7 == 1) {
                    for (phi_a3 = 0; phi_a3 < 0x30; phi_a3++) {
                        func_8006EE50(&globalCtx->msgCtx.font, 0x8140, phi_a2 * 0x80);
                    }
                    func_801086B0(globalCtx, &sp140);
                }
                break;
            case 6:
            case 8:
                func_801086B0(globalCtx, &sp140);
                break;
            case 7:
            case 52:
                func_801086B0(globalCtx, &sp140);
                func_80107980(globalCtx, &sp140, XREG(64), XREG(65));
                break;
            case 9:
            case 10:
            case 11:
                func_800ED858(1);
                temp_s1->unk_E2B8 = func_800EE3D4();
                temp_s1->unk_E2B8->unk_2 = NON_CONST(D_8014B2F8,s16) = 0;
                globalCtx->msgCtx.unk_E3EE = 1;
                func_801069B0();
                D_80153CDC = 1;
                D_80153CD8 = 3;
                if (temp_s1->msgMode == 9) {
                    if ((temp_s1->unk_E3F0 == 0) || (temp_s1->unk_E3F0 == 1) || (temp_s1->unk_E3F0 == 0x2C) || (temp_s1->unk_E3F0 == 0x30) || (temp_s1->unk_E3F0 >= 0x22)) {
                        if ((temp_s1->unk_E3F0 == 1) || (temp_s1->unk_E3F0 == 0x30)) {
                            func_800ECC04(D_8014B31C + 0xC000);
                        } else {
                            osSyncPrintf("台上演奏\n");
                            func_800ECC04(D_8014B31C);
                        }
                    } else {
                        osSyncPrintf("Na_StartOcarinaSinglePlayCheck2( message->ocarina_no );\n");
                        func_800ECC04((1 << temp_s1->unk_E3F0) + 0x8000);
                    }
                    temp_s1->msgMode = 0xC;
                } else {
                    if (temp_s1->msgMode == 0xA) {
                        temp_s1->unk_E3E7 = 0x14;
                        phi_t8 = 0x18;
                    } else {
                        func_800ECC04((1 << (temp_s1->unk_E3F0 + 0x11)) + 0x8000);
                        osSyncPrintf("演奏チェック=%d\n", temp_s1->unk_E3F0 - 0xF);
                        phi_t8 = 0x1B;
                    }
                    temp_s1->unk_E305 = phi_t8;
                }
                if ((temp_s1->unk_E3F0 != 1) && (temp_s1->unk_E3F0 != 0x30)) {
                    func_801086B0(globalCtx, &sp140);
                }
                break;
            case 12:
                temp_s1->unk_E2B8 = func_800EE3D4();
                if (temp_s1->unk_E2B8->unk_2 != 0) {
                    osSyncPrintf("locate=%d  onpu_pt=%d\n", temp_s1->unk_E2B8->unk_2, D_8014B2F8, temp_s1->unk_E2B8);
                    if (temp_s1->unk_E2B8->unk_2 == 1) {
                        if (D_8014B2F8 == 8) {
                            NON_CONST(D_8014B2F8,s16) = 0;
                        }
                    }
                    if (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2 - 1] = temp_s1->unk_E2B8->unk_0;
                        temp_s1->unk_E410 = temp_s1->unk_E2B8->unk_0;
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2] = 0xFF;
                        NON_CONST(D_8014B2F8,s16)++;
                    }
                }
                temp_s1->unk_E2B8->unk_1 = temp_s1->unk_E2B8->unk_1;
                if (temp_s1->unk_E2B8->unk_1 < 0xD) {
                    if ((temp_s1->unk_E2B8->unk_1 == 0xC) || (((&gBitFlags[6])[D_8015396C[temp_s1->unk_E2B8->unk_1]] & gSaveContext.inventory.questItems) != 0)) {
                        temp_s1->unk_E3F2 = temp_s1->unk_E2B8->unk_1;
                        D_8014B310 = temp_s1->unk_E2B8->unk_1;
                        temp_s1->msgMode = 0xD;
                        temp_s1->unk_E3E7 = 0x14;
                        if (temp_s1->unk_E3F0 == 0x30) {
                            if ((temp_s1->unk_E2B8->unk_1 < 6) || (0xC == temp_s1->unk_E2B8->unk_1)) {
                                func_800ED858(0);
                                Audio_PlaySoundGeneral(NA_SE_SY_OCARINA_ERROR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                                temp_s1->msgMode = 9;
                            } else {
                                osSyncPrintf("Ocarina_Flog 正解模範演奏=%x\n", temp_s1->unk_E3EC);
                                func_8010B720(globalCtx, 0x86F);
                                temp_s1->msgMode = 0x11;
                                temp_s1->unk_E2FD = 3;
                                temp_s1->unk_E3E7 = 0xA;
                                Audio_PlaySoundGeneral(NA_SE_SY_TRE_BOX_APPEAR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                                Interface_ChangeAlpha(1);
                            }
                        } else if (temp_s1->unk_E3F0 == 0x28) {
                            if (temp_s1->unk_E2B8->unk_1 < 0xC) {
                                func_800ED858(0);
                                Audio_PlaySoundGeneral(NA_SE_SY_OCARINA_ERROR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                                temp_s1->unk_E3E7 = 0xA;
                                temp_s1->msgMode = 0xE;
                            } else {
                                osSyncPrintf("Ocarina_Flog 正解模範演奏=%x\n", temp_s1->unk_E3EC);
                                func_8010B720(globalCtx, 0x86F);
                                temp_s1->msgMode = 0x11;
                                temp_s1->unk_E2FD = 3;
                                temp_s1->unk_E3E7 = 0xA;
                                Audio_PlaySoundGeneral(NA_SE_SY_TRE_BOX_APPEAR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                                Interface_ChangeAlpha(1);
                            }
                        } else if (temp_v0_8 == 1) {
                            osSyncPrintf("Ocarina_Free 正解模範演奏=%x\n", temp_s1->unk_E3EC);
                            func_8010B720(globalCtx, 0x86F);
                            temp_s1->msgMode = 0x11;
                            temp_s1->unk_E2FD = 3;
                            temp_s1->unk_E3E7 = 0xA;
                            Audio_PlaySoundGeneral(NA_SE_SY_TRE_BOX_APPEAR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                        } else {
                            Audio_PlaySoundGeneral(NA_SE_SY_TRE_BOX_APPEAR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                        }
                        Interface_ChangeAlpha(1);
                    } else {
                        func_800ED858(0);
                        Audio_PlaySoundGeneral(NA_SE_SY_OCARINA_ERROR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                        temp_s1->msgMode = 9;
                    }
                } else if (temp_a0_2 == 0xFF) {
                    func_800ED858(0);
                    Audio_PlaySoundGeneral(NA_SE_SY_OCARINA_ERROR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    temp_s1->unk_E3E7 = 0xA;
                    temp_s1->msgMode = 0xE;
                } else if (~(globalCtx->state.input[0].press.button | ~0x4000) == 0) {
                    func_800ED858(0);
                    globalCtx->msgCtx.unk_E3EE = 4;
                    func_80106CCC(globalCtx);
                }
                if ((temp_s1->unk_E3F0 != 1) && (temp_s1->unk_E3F0 != 0x30)) {
                    func_801086B0(globalCtx, &sp140);
                }
                break;
            case 13:
            case 28:
            case 39:
                temp_t9_3 = D_80153CDC * 6;
                temp_a1 = temp_t9_3 + &D_80153CA8;
                temp_a2_2 = D_801759A8;
                temp_t8 = temp_a1->unk0;
                sp88 = temp_t9_3;
                temp_v1_2 = temp_a2_2 - temp_t8;
                sp80 = temp_t8;
                if (temp_v1_2 >= 0) {
                    phi_v0 = temp_v1_2;
                } else {
                    phi_v0 = -temp_v1_2;
                }
                temp_t2 = D_80153CD8;
                temp_a3_3 = D_801759AC;
                temp_t9_4 = temp_a1->unk2;
                temp_v1_3 = temp_a3_3 - temp_t9_4;
                temp_t0_2 = phi_v0 / temp_t2;
                sp78 = temp_t9_4;
                if (temp_v1_3 >= 0) {
                    phi_v0_2 = temp_v1_3;
                } else {
                    phi_v0_2 = -temp_v1_3;
                }
                temp_t1 = D_801759AA;
                temp_t3 = temp_a1->unk4;
                temp_v1_4 = temp_t1 - temp_t3;
                temp_a0_3 = phi_v0_2 / temp_t2;
                if (temp_v1_4 >= 0) {
                    phi_v0_3 = temp_v1_4;
                } else {
                    phi_v0_3 = -temp_v1_4;
                }
                if (temp_a2_2 >= sp80) {
                    D_801759A8 = temp_a2_2 - temp_t0_2;
                } else {
                    D_801759A8 = temp_a2_2 + temp_t0_2;
                }
                if (temp_a3_3 >= sp78) {
                    D_801759AC = temp_a3_3 - temp_a0_3;
                } else {
                    D_801759AC = temp_a3_3 + temp_a0_3;
                }
                if (temp_t1 >= temp_t3) {
                    D_801759AA = temp_t1 - (phi_v0_3 / temp_t2);
                } else {
                    D_801759AA = temp_t1 + (phi_v0_3 / temp_t2);
                }
                sp70 = temp_t3;
                temp_a1_2 = sp88 + &D_80153CB4;
                temp_t8_2 = temp_a1_2->unk0;
                temp_a2_3 = D_801759AE;
                sp7C = temp_t8_2;
                temp_v1_5 = temp_a2_3 - temp_t8_2;
                if (temp_v1_5 >= 0) {
                    phi_v0_4 = temp_v1_5;
                } else {
                    phi_v0_4 = -temp_v1_5;
                }
                temp_a3_4 = D_801759B2;
                temp_t9_5 = temp_a1_2->unk2;
                temp_v1_6 = temp_a3_4 - temp_t9_5;
                temp_t0_3 = phi_v0_4 / temp_t2;
                sp6C = temp_t9_5;
                if (temp_v1_6 >= 0) {
                    phi_v0_5 = temp_v1_6;
                } else {
                    phi_v0_5 = -temp_v1_6;
                }
                temp_t1_2 = D_801759B0;
                temp_t3_2 = temp_a1_2->unk4;
                temp_v1_7 = temp_t1_2 - temp_t3_2;
                temp_a0_4 = phi_v0_5 / temp_t2;
                sp64 = temp_t3_2;
                if (temp_v1_7 >= 0) {
                    phi_v0_13 = temp_v1_7;
                } else {
                    phi_v0_13 = -temp_v1_7;
                }
                temp_t3_3 = D_801759BA;
                if (temp_t3_3 >= sp7C) {
                    D_801759AE = temp_a2_3 - temp_t0_3;
                } else {
                    D_801759AE = temp_a2_3 + temp_t0_3;
                }
                temp_t4 = D_801759BE;
                if (temp_t4 >= sp6C) {
                    D_801759B2 = temp_a3_4 - temp_a0_4;
                } else {
                    D_801759B2 = temp_a3_4 + temp_a0_4;
                }
                temp_t5 = D_801759BC;
                if (temp_t5 >= sp64) {
                    phi_t9 = temp_t1_2 - (phi_v0_13 / temp_t2);
                } else {
                    phi_t9 = temp_t1_2 + (phi_v0_13 / temp_t2);
                }
                D_801759B0 = phi_t9;
                temp_a1_3 = sp88 + &D_80153CC0;
                temp_v0_10 = temp_a1_3->unk0;
                temp_a2_4 = D_801759B4;
                sp5C = temp_v0_10;
                temp_v1_8 = temp_a2_4 - temp_v0_10;
                if (temp_v1_8 >= 0) {
                    phi_v0_6 = temp_v1_8;
                } else {
                    phi_v0_6 = -temp_v1_8;
                }
                temp_a3_5 = D_801759B8;
                temp_a0_5 = temp_a1_3->unk2;
                temp_v1_9 = temp_a3_5 - temp_a0_5;
                temp_t0_4 = phi_v0_6 / temp_t2;
                sp54 = temp_a0_5;
                if (temp_v1_9 >= 0) {
                    phi_v0_7 = temp_v1_9;
                } else {
                    phi_v0_7 = -temp_v1_9;
                }
                temp_t1_3 = D_801759B6;
                temp_ra = temp_a1_3->unk4;
                temp_v1_10 = temp_t1_3 - temp_ra;
                temp_a0_6 = phi_v0_7 / temp_t2;
                if (temp_v1_10 >= 0) {
                    phi_v0_14 = temp_v1_10;
                } else {
                    phi_v0_14 = -temp_v1_10;
                }
                if (temp_a2_4 >= sp5C) {
                    D_801759B4 = temp_a2_4 - temp_t0_4;
                } else {
                    D_801759B4 = temp_a2_4 + temp_t0_4;
                }
                if (temp_a3_5 >= sp54) {
                    D_801759B8 = temp_a3_5 - temp_a0_6;
                } else {
                    D_801759B8 = temp_a3_5 + temp_a0_6;
                }
                if (temp_t1_3 >= temp_ra) {
                    phi_t9_2 = temp_t1_3 - (phi_v0_14 / temp_t2);
                } else {
                    phi_t9_2 = temp_t1_3 + (phi_v0_14 / temp_t2);
                }
                D_801759B6 = phi_t9_2;
                temp_a1_4 = sp88 + &D_80153CCC;
                temp_t1_4 = temp_a1_4->unk0;
                temp_v1_11 = temp_t3_3 - temp_t1_4;
                if (temp_v1_11 >= 0) {
                    phi_v0_8 = temp_v1_11;
                } else {
                    phi_v0_8 = -temp_v1_11;
                }
                temp_a2_5 = temp_a1_4->unk2;
                temp_v1_12 = temp_t4 - temp_a2_5;
                temp_t0_5 = phi_v0_8 / temp_t2;
                if (temp_v1_12 >= 0) {
                    phi_v0_9 = temp_v1_12;
                } else {
                    phi_v0_9 = -temp_v1_12;
                }
                temp_a3_6 = temp_a1_4->unk4;
                temp_v1_13 = temp_t5 - temp_a3_6;
                temp_a0_7 = phi_v0_9 / temp_t2;
                if (temp_v1_13 >= 0) {
                    phi_v0_15 = temp_v1_13;
                } else {
                    phi_v0_15 = -temp_v1_13;
                }
                if (temp_t3_3 >= temp_t1_4) {
                    D_801759BA = temp_t3_3 - temp_t0_5;
                } else {
                    D_801759BA = temp_t3_3 + temp_t0_5;
                }
                if (temp_t4 >= temp_a2_5) {
                    D_801759BE = temp_t4 - temp_a0_7;
                } else {
                    D_801759BE = temp_t4 + temp_a0_7;
                }
                if (temp_t5 >= temp_a3_6) {
                    phi_t8_2 = temp_t5 - (phi_v0_15 / temp_t2);
                } else {
                    phi_t8_2 = temp_t5 + (phi_v0_15 / temp_t2);
                }
                D_801759BC = phi_t8_2;
                temp_t2 = temp_t2 - 1;
                D_80153CD8 = temp_t2;
                if (temp_t2 == 0) {
                    D_801759A8 = sp80;
                    D_801759AC = sp78;
                    D_801759AA = sp70;
                    D_801759AE = sp7C;
                    D_801759B2 = sp6C;
                    D_801759B0 = sp64;
                    D_801759B4 = sp5C;
                    D_801759B8 = sp54;
                    D_801759B6 = temp_ra;
                    D_801759BA = temp_t1_4;
                    D_801759BE = temp_a2_5;
                    D_801759BC = temp_a3_6;
                    D_80153CDC ^= 1;
                    D_80153CD8 = 3;
                }
                temp_s1->unk_E3E7--;
                if (temp_s1->unk_E3E7 == 0) {
                    func_800ED858(0);
                    if (temp_s1->msgMode == 0xD) {
                        osSyncPrintf("正解模範演奏=%x\n", temp_s1->unk_E3EC);
                        func_8010B720(globalCtx, 0x86F);
                        temp_s1->msgMode = 0x11;
                        temp_s1->unk_E2FD = 3;
                        temp_s1->unk_E3E7 = 1;
                    } else if (temp_s1->msgMode == 0x1C) {
                        if (temp_s1->unk_E3EC >= 6) {
                            func_8010B720(globalCtx, 0x86F);
                            temp_s1->msgMode = 0x11;
                            temp_s1->unk_E2FD = 3;
                            temp_s1->unk_E3E7 = 1;
                        } else {
                            func_80106CCC(globalCtx);
                            globalCtx->msgCtx.unk_E3EE = 4;
                        }
                    } else {
                        func_80106CCC(globalCtx);
                        globalCtx->msgCtx.unk_E3EE = 3;
                    }
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 14:
            case 29:
                func_801086B0(globalCtx, &sp140);
            case 15:
                temp_s1->unk_E3E7--;
                if (temp_s1->unk_E3E7 == 0) {
                    VREG(51) = 1;
                    if (temp_s1->msgMode == 0x1D) {
                        osSyncPrintf("ここここここ\n");
                        func_8010B720(globalCtx, 0x88B);
                        func_80109B3C(globalCtx);
                        temp_s1->msgMode = 0x1E;
                    } else {
                        temp_s1->msgMode = 0x10;
                    }
                    osSyncPrintf("キャンセル\n");
                }
                break;
            case 16:
            case 30:
                for (phi_a3_3 = 0; phi_a3_3 < 5; phi_a3_3++) {
                    (&VREG(35))[phi_a3_3] += VREG(51);
                }
                VREG(51) += 2;
                if (VREG(51) >= 0x226) {
                    NON_CONST(D_8015394C,u8*)[0] = 0xFF;
                    temp_at = D_80153958 + 8;
                    *temp_at = 0;
                    *temp_at = 0;
                    *temp_at = 0;
                    *temp_at = 0;
                    *temp_at = 0;
                    *D_80153958 = 0;
                    *D_80153958 = 0;
                    *D_80153958 = 0;
                    *D_80153958 = 0;
                    if (temp_s1->msgMode == 0x1E) {
                        temp_s1->msgMode = 0x1F;
                    } else {
                        temp_s1->msgMode = 9;
                    }
                }
                break;
            case 17:
                temp_s1->unk_E3E7--;
                if (temp_s1->unk_E3E7 == 0) {
                    func_800ED858(0);
                    osSyncPrintf("\x1b[32m");
                    osSyncPrintf("Na_StopOcarinaMode();\n");
                    osSyncPrintf("Na_StopOcarinaMode();\n");
                    osSyncPrintf("Na_StopOcarinaMode();\n");
                    osSyncPrintf("\x1b[m");
                    func_80109B3C(globalCtx);
                    temp_s1->msgMode = 0x12;
                    temp_s1->unk_E2B8 = func_800EE3D4();
                    NON_CONST(D_8014B2F8,s16) = 0;
                    temp_s1->unk_E2B8->unk_2 = D_8014B2F8;
                    func_801069B0();
                    if (temp_s1->unk_E3EC >= 6 && temp_s1->unk_E3EC < 0xD) {
                        Actor_Spawn(&globalCtx->actorCtx, globalCtx, D_80153C68[temp_s1->unk_E3EC], 
                                    sp148->actor.posRot.pos.x, sp148->actor.posRot.pos.y, sp148->actor.posRot.pos.z, 
                                    0, 0, 0, D_80153C78[temp_s1->unk_E3EC]);
                    }
                }
                break;
            case 18:
                func_801086B0(globalCtx, &sp140);
                func_800ED858(1);
                func_800ED858(1);
                func_800ED93C(temp_s1->unk_E3EC + 1, 1);
                if (0xC != temp_s1->unk_E3EC) {
                    func_800F5C64(D_80153CE0[temp_s1->unk_E3EC]);
                    func_800F7260(0x20);
                }
                globalCtx->msgCtx.unk_E3EE = 1;
                if (temp_s1->unk_E3F0 == 1) {
                    temp_s1->unk_E3F0 = 0x29;
                }
                if (temp_s1->unk_E3F0 == 0x30) {
                    temp_s1->unk_E3F0 = 0x31;
                }
                NON_CONST(D_8014B2F8,s16) = 0;
                temp_s1->msgMode = 0x13;
                break;
            case 24:
                temp_s1->unk_E3E7--;
                if (temp_s1->unk_E3E7 == 0) {
                    osSyncPrintf("ocarina_no=%d  選曲=%d\n", temp_s1->unk_E3F0, 0x16);
                    if (temp_s1->unk_E3F0 < 8) {
                        func_800ED858(4);
                    } else if (temp_s1->unk_E3F0 == 9) {
                        func_800ED858(2);
                    } else if (temp_s1->unk_E3F0 == 0xA) {
                        func_800ED858(3);
                    } else if (temp_s1->unk_E3F0 == 0xD) {
                        func_800ED858(5);
                    } else {
                        func_800ED858(1);
                    }
                    osSyncPrintf("模範演奏=%x\n", temp_s1->unk_E3F0 - 2);
                    func_800ED93C(temp_s1->unk_E3F0 - 1, 2);
                    NON_CONST(D_8014B2F8,s16) = 0;
                    temp_s1->msgMode = 0x19;
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 20:
                func_8010B720(globalCtx, temp_s1->unk_E3EC + 0x893);
                func_80109B3C(globalCtx);
                temp_s1->msgMode = 0x15;
                temp_s1->unk_E3E7 = 0x14;
                func_801086B0(globalCtx, &sp140);
                break;
            case 21:
                temp_s1->unk_E3E7--;
                if (temp_s1->unk_E3E7 == 0) {
                    temp_s1->msgMode = 0x16;
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 22:
                func_800ED858(0);
                func_801069B0();
                temp_s1->msgMode = 0x17;
                temp_s1->unk_E3E7 = 2;
                func_801086B0(globalCtx, &sp140);
                break;
            case 23:
                temp_s1->unk_E3E7--;
                if (temp_s1->unk_E3E7 == 0) {
                    if (temp_s1->unk_E3EC < 6 && ((temp_s1->unk_E3F0 < 0xF) || (temp_s1->unk_E3F0 >= 0x15))) {
                        if ((temp_s1->unk_E40C != 0) || (globalCtx->interfaceCtx.restrictions.warpSongs == 3)) {
                            func_8010B680(globalCtx, 0x88C, NULL);
                            globalCtx->msgCtx.unk_E3EE = 4;
                        } else if ((gSaveContext.eventInf[0] & 0xF) != 1) {
                            func_8010B680(globalCtx, temp_s1->unk_E3EC + 0x88D, NULL);
                            globalCtx->msgCtx.unk_E3EE = 1;
                        } else {
                            func_80106CCC(globalCtx);
                        }
                    }
                    func_80106CCC(globalCtx);
                    if (temp_s1->unk_E3EC == 7) {
                        DREG(53) = 1;
                    }
                    osSyncPrintf("\x1b[33m");
                    osSyncPrintf("☆☆☆ocarina=%d   message->ocarina_no=%d  ", temp_s1->unk_E3EC, temp_s1->unk_E3F0);
                    if (temp_s1->unk_E3F0 == 0x29) {
                        if (0xC == temp_s1->unk_E3EC) {
                            globalCtx->msgCtx.unk_E3EE = 0xB;
                        } else {
                            globalCtx->msgCtx.unk_E3EE = 1;
                        }
                    } else if (temp_s1->unk_E3F0 >= 0x1C) {
                        osSyncPrintf("\x1b[33m");
                        osSyncPrintf("Ocarina_PC_Wind=%d(%d) ☆☆☆   ", 0x1C, temp_s1->unk_E3F0 - 0x1C);
                        if (temp_s1->unk_E3F0 == (temp_s1->unk_E3EC + 0x1C)) {
                            globalCtx->msgCtx.unk_E3EE = 3;
                        } else {
                            globalCtx->msgCtx.unk_E3EE = temp_s1->unk_E3EC - 1;
                        }
                    } else {
                        osSyncPrintf("\x1b[32m");
                        osSyncPrintf("Ocarina_C_Wind=%d(%d) ☆☆☆   ", 0xF, temp_s1->unk_E3F0 - 0xF);
                        if (temp_s1->unk_E3F0 == (temp_s1->unk_E3EC + 0xF)) {
                            globalCtx->msgCtx.unk_E3EE = 3;
                        } else {
                            globalCtx->msgCtx.unk_E3EE = 4;
                        }
                    }
                    osSyncPrintf("\x1b[m");
                    osSyncPrintf("→  OCARINA_MODE=%d\n", globalCtx->msgCtx.unk_E3EE);
                }
                break;
            case 19:
            case 25:
                temp_s1->unk_E2B8 = func_800EE3F8();
                if (temp_s1->unk_E2B8->unk_1 == 0) {
                    if (temp_s1->msgMode == 0x13) {
                        temp_s1->msgMode = 0x14;
                    } else {
                        temp_s1->msgMode = 0x1A;
                    }
                    osSyncPrintf("onpu_buff[%d]=%x\n", temp_s1->unk_E2B8->unk_2, NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2]);
                } else {
                    if ((D_8014B2F8 != 0) && (temp_s1->unk_E2B8->unk_2 == 1)) {
                        NON_CONST(D_8014B2F8, s16) = 0;
                    }
                    if ((temp_s1->unk_E2B8->unk_2 != 0) && (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1))) {
                        temp_v0_17 = temp_s1->unk_E2B8->unk_0;
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2 - 1] = temp_v0_17;
                        temp_s1->unk_E410 = temp_v0_17;
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2] = 0xFF;
                        NON_CONST(D_8014B2F8, s16)++;
                    }
                }
            case 26:
                func_801086B0(globalCtx, &sp140);
                break;
            case 27:
                temp_s1->unk_E2B8 = func_800EE3D4();
                if (temp_s1->unk_E2B8->unk_2 != 0) {
                    if (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2 - 1] = temp_s1->unk_E2B8->unk_0;
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2] = 0xFF;
                        NON_CONST(D_8014B2F8, s16)++;
                    }
                }
                if (temp_s1->unk_E2B8->unk_1 < 0xD) {
                    osSyncPrintf("M_OCARINA20 : ocarina_no=%x    status=%x\n", temp_s1->unk_E3F0, temp_s1->unk_E2B8->unk_1);
                    temp_s1->msgMode = 0x1C;
                    temp_s1->unk_E3EC = temp_s1->unk_E2B8->unk_1;
                    Item_Give(globalCtx, D_8015396C[temp_s1->unk_E2B8->unk_1] + 0x5A);
                    osSyncPrintf("\x1b[33m");
                    osSyncPrintf("z_message.c 取得メロディ＝%d\n", temp_s1->unk_E2B8->unk_1 + 0x5A);
                    osSyncPrintf("\x1b[m");
                    temp_s1->unk_E3E7 = 0x14;
                    Audio_PlaySoundGeneral(NA_SE_SY_TRE_BOX_APPEAR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                } else if (temp_s1->unk_E2B8->unk_1 == 0xFF) {
                    Audio_PlaySoundGeneral(NA_SE_SY_OCARINA_ERROR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    temp_s1->unk_E3E7 = 0xA;
                    temp_s1->msgMode = 0x1D;
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 31:
                func_801086B0(globalCtx, &sp140);
                if (func_80106BC8(globalCtx) != 0) {
                    func_8010BD58(globalCtx, temp_s1->unk_E3F0);
                }
                break;
            case 33:
                osSyncPrintf("案山子録音 初期化\n");
                func_800EE170(1);
                func_800ED858(1);
                temp_s1->unk_E2B8 = func_800EE3C8();
                NON_CONST(D_8014B2F8, s16) = 0;
                temp_s1->unk_E2B8->unk_2 = D_8014B2F8;
                NON_CONST(D_8014B2FC, s16) = 0;
                func_801069B0();
                temp_s1->msgMode = 0x22;
                func_801086B0(globalCtx, &sp140);
                break;
            case 34:
                temp_s1->unk_E2B8 = func_800EE3C8();
                osSyncPrintf("\nonpu_pt=%d, locate=%d", D_8014B2F8, temp_s1->unk_E2B8->unk_2);
                if ((temp_s1->unk_E2B8->unk_2 != 0) && (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1))) {
                    for (phi_v0_10 = D_8014B2FC - 8, phi_a3_4 = 0; phi_a3_4 < 8; phi_v0_10++, phi_a3_4++) {
                        NON_CONST(D_8015394C,u8*)[phi_v0_10] = NON_CONST(D_8015394C,u8*)[phi_v0_10 + 1];
                    }
                    NON_CONST(D_8014B2FC,s16)--;
                    osSyncPrintf("    入力ボタン【%d】=%d", D_8014B2FC, temp_s1->unk_E2B8->unk_0);
                    NON_CONST(D_8015394C,u8*)[D_8014B2FC] = temp_s1->unk_E2B8->unk_0;
                    temp_s1->unk_E410 = temp_s1->unk_E2B8->unk_0;
                    NON_CONST(D_8014B2FC, s16)++;
                    NON_CONST(D_8015394C,u8*)[D_8014B2FC] = 0xFF;
                    NON_CONST(D_8014B2F8, s16)++;
                    if (temp_s1->unk_E2B8->unk_2 == 8) {
                        NON_CONST(D_8014B2F8, s16) = 0;
                    }
                }
                if ((temp_s1->unk_E2B8->unk_1 == 0) || (~(globalCtx->state.input[0].press.button | ~0x4000) == 0)) {
                    if (D_8014B2FC != 0) {
                        osSyncPrintf("録音終了！！！！！！！！！  message->info->status=%d \n", temp_s1->unk_E2B8->unk_1);
                        gSaveContext.scarecrowCustomSongSet = 1;
                    }
                    Audio_PlaySoundGeneral(0x4827, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    osSyncPrintf("aaaaaaaaaaaaaa\n");
                    func_800EE170(0);
                    temp_s1->unk_E3E7 = 0xA;
                    globalCtx->msgCtx.unk_E3EE = 4;
                    func_80106CCC(globalCtx);
                    osSyncPrintf("録音終了！！！！！！！！！録音終了\n");
                    osSyncPrintf("\x1b[33m");
                    osSyncPrintf("\n====================================================================\n");
                    MemCopy(gSaveContext.scarecrowCustomSong, gScarecrowCustomSongPtr, sizeof(gSaveContext.scarecrowCustomSong));
                    for (phi_a3_5 = 0; phi_a3_5 < sizeof(gSaveContext.scarecrowCustomSong); phi_a3_5++) {
                        osSyncPrintf("%d, ", gSaveContext.scarecrowCustomSong[phi_a3_5]);
                    }
                    osSyncPrintf("\x1b[m");
                    osSyncPrintf("\n====================================================================\n");
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 35:
            case 40:
                temp_s1->unk_E2B8 = func_800EE3F8();
                if (temp_s1->unk_E2B8->unk_2 != 0 && temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                    for (phi_v0_11 = D_8014B2FC - 8, phi_a3_6 = 0; phi_a3_6 < 8; phi_v0_11++, phi_a3_6++) {
                        NON_CONST(D_8015394C,u8*)[phi_v0_11] = D_8015394C[phi_v0_11 + 1];
                    }
                    NON_CONST(D_8014B2FC,s16)--;
                    NON_CONST(D_8015394C,u8*)[D_8014B2FC] = temp_s1->unk_E2B8->unk_0;
                    NON_CONST(D_8014B2FC, s16)++;
                    NON_CONST(D_8015394C,u8*)[D_8014B2FC] = 0xFF;
                    NON_CONST(D_8014B2F8, s16)++;
                    if (temp_s1->unk_E2B8->unk_2 == 8) {
                        NON_CONST(D_8014B2FC, s16) = NON_CONST(D_8014B2F8, s16) = 0;
                    }
                }
                osSyncPrintf("status=%d (%d)\n", temp_s1->unk_E2B8->unk_1, 0);
                if (temp_s1->unk_E3E7 == 0) {
                    if (temp_s1->unk_E2B8->unk_1 == 0) {
                        osSyncPrintf("bbbbbbbbbbb\n");
                        func_800ED858(0);
                        globalCtx->msgCtx.unk_E3EE = 0xF;
                        func_80106CCC(globalCtx);
                    }
                } else {
                    temp_s1->unk_E3E7--;
                }
                break;
            case 36:
                func_800EE170(2);
                func_800ED858(1);
                temp_s1->msgMode = 0x25;
                func_801086B0(globalCtx, &sp140);
                break;
            case 37:
                temp_s1->unk_E2B8 = func_800EE3C8();
                if (temp_s1->unk_E2B8->unk_2 != 0) {
                    if (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                        NON_CONST(D_8015394C,u8*)[D_8014B2F8] = temp_s1->unk_E2B8->unk_0;
                        temp_s1->unk_E410 = temp_s1->unk_E2B8->unk_0;
                        NON_CONST(D_8014B2F8, s16)++;
                        NON_CONST(D_8015394C,u8*)[D_8014B2F8] = 0xFF;
                    }
                }
                if (temp_s1->unk_E2B8->unk_1 == 0) {
                    osSyncPrintf("８音録音ＯＫ！\n");
                    temp_s1->unk_E3E7 = 0x14;
                    gSaveContext.scarecrowSpawnSongSet = 1;
                    temp_s1->msgMode = 0x27;
                    Audio_PlaySoundGeneral(0x4807, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    osSyncPrintf("\x1b[33m");
                    osSyncPrintf("\n====================================================================\n");
                    MemCopy(gSaveContext.scarecrowSpawnSong, gScarecrowSpawnSongPtr, sizeof(gSaveContext.scarecrowSpawnSong));
                    for (phi_a3_7 = 0; phi_a3_7 < sizeof(gSaveContext.scarecrowSpawnSong); phi_a3_7++) {
                        osSyncPrintf("%d, ", gSaveContext.scarecrowSpawnSong[phi_a3_7]);
                    }
                    osSyncPrintf("\x1b[m");
                    osSyncPrintf("\n====================================================================\n");
                } else if ((temp_s1->unk_E2B8->unk_1 == 0xFF) || (~(globalCtx->state.input[0].press.button | ~0x4000) == 0)) {
                    osSyncPrintf("すでに存在する曲吹いた！！！ \n");
                    func_800EE170(0);
                    Audio_PlaySoundGeneral(NA_SE_SY_OCARINA_ERROR, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    func_80106CCC(globalCtx);
                    temp_s1->msgMode = 0x26;
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 38:
                osSyncPrintf("cccccccccccc\n");
                func_800ED858(0);
                func_8010B680(globalCtx, 0x40AD, NULL); // Bonooru doesn't remember your song
                globalCtx->msgCtx.unk_E3EE = 4;
                break;
            case 41:
                func_800ED858(1);
                func_800ED858(6);
                func_800EE57C(gSaveContext.ocarinaGameReward);
                temp_s1->unk_E2B8 = func_800EE3F8();
                NON_CONST(D_8014B2F8, s16) = 0;
                temp_s1->unk_E2B8->unk_2 = D_8014B2F8;
                func_801069B0();
                func_800ED93C(0xE, 1);
                temp_s1->msgMode = 0x2A;
                temp_s1->unk_E3E7 = 2;
                break;
            case 42:
            case 44:
                Audio_PlaySoundGeneral(0x4038, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                temp_s1->unk_E2B8 = func_800EE3F8();
                if (temp_s1->unk_E2B8->unk_2 != 0) {
                    if (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2 - 1] = temp_s1->unk_E2B8->unk_0;
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2] = 0xFF;
                        NON_CONST(D_8014B2F8, s16)++;
                    }
                }
                if (temp_s1->unk_E3E7 == 0) {
                    if (temp_s1->unk_E2B8->unk_1 == 0) {
                        if (temp_s1->msgMode == 0x2A) {
                            Audio_PlaySoundGeneral(NA_SE_SY_METRONOME, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                        } else {
                            Audio_PlaySoundGeneral(NA_SE_SY_METRONOME_2, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                        }
                        temp_s1->msgMode = temp_s1->msgMode + 1;
                    }
                } else {
                    temp_s1->unk_E3E7 = temp_s1->unk_E3E7 - 1;
                }
                break;
            case 43:
            case 45:
                temp_s1->unk_E2B8 = func_800EE3F8();
                if (temp_s1->unk_E2B8->unk_2 != 0 && temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                    NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2 - 1] = temp_s1->unk_E2B8->unk_0;
                    NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2] = 0xFF;
                    NON_CONST(D_8014B2F8, s16) = D_8014B2F8 + 1;
                }
                break;
            case 46:
                Audio_PlaySoundGeneral(0x4038, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                temp_s1->unk_E2B8 = func_800EE3D4();
                if (temp_s1->unk_E2B8->unk_2 != 0) {
                    if (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2 - 1] = temp_s1->unk_E2B8->unk_0;
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2] = 0xFF;
                        NON_CONST(D_8014B2F8, s16)++;
                    }
                }
                if (temp_s1->unk_E2B8->unk_1 == 0xFF) {
                    osSyncPrintf("輪唱失敗！！！！！！！！！\n");
                    func_800ED858(0);
                    Audio_PlaySoundGeneral(0x4827, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    temp_s1->unk_E3E7 = 0xA;
                    globalCtx->msgCtx.unk_E3EE = 3;
                } else if (temp_s1->unk_E2B8->unk_1 == 0xD) {
                    osSyncPrintf("輪唱成功！！！！！！！！！\n");
                    Audio_PlaySoundGeneral(0x4824, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                    temp_s1->msgMode = 0x2F;
                    temp_s1->unk_E3E7 = 0x1E;
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 47:
                temp_s1->unk_E2B8 = func_800EE3D4();
                if (temp_s1->unk_E2B8->unk_2 != 0) {
                    if (temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2 - 1] = temp_s1->unk_E2B8->unk_0;
                        NON_CONST(D_8015394C,u8*)[temp_s1->unk_E2B8->unk_2] = 0xFF;
                        NON_CONST(D_8014B2F8, s16)++;
                    }
                }
                temp_s1->unk_E3E7--;
                if (temp_s1->unk_E3E7 == 0) {
                    if (func_800EE5EC() != 1) {
                        Audio_PlaySoundGeneral(NA_SE_SY_METRONOME, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                        temp_s1->unk_E2B8 = func_800EE3D4();
                        NON_CONST(D_8014B2F8, s16) = 0;
                        temp_s1->unk_E2B8->unk_2 = D_8014B2F8;
                        func_801069B0();
                        temp_s1->msgMode = 0x30;
                    } else {
                        globalCtx->msgCtx.unk_E3EE = 0xF;
                    }
                }
                func_801086B0(globalCtx, &sp140);
                break;
            case 48:
                if (func_800F8FF4(NA_SE_SY_METRONOME) == 0) {
                    temp_s1->unk_E2B8 = func_800EE3F8();
                    NON_CONST(D_8014B2F8, s16) = 0;
                    temp_s1->unk_E2B8->unk_2 = D_8014B2F8;
                    func_801069B0();
                    func_800ED93C(0xE, 1);
                }
                break;
            case 49:
                func_800ED858(1);
                temp_s1->unk_E2B8 = func_800EE3D4();
                NON_CONST(D_8014B2F8, s16) = 0;
                temp_s1->unk_E2B8->unk_2 = D_8014B2F8;
                globalCtx->msgCtx.unk_E3EE = 1;
                func_801069B0();
                func_800ECC04(D_8014B31C + 0xC000);
                temp_s1->msgMode = 0x32;
                break;
            case 50:
                temp_s1->unk_E2B8 = func_800EE3D4();
                if (temp_s1->unk_E2B8->unk_2 == 0 || temp_s1->unk_E2B8->unk_2 == (D_8014B2F8 + 1)) {
                    temp_s1->unk_E410 = temp_s1->unk_E2B8->unk_0;
                    NON_CONST(D_8014B2F8, s16) = 0;
                    temp_s1->unk_E2B8->unk_2 = D_8014B2F8;
                    func_801069B0();
                    temp_s1->msgMode = 0x33;
                }
            case 51:
                break;
            case 53:
                func_801086B0(globalCtx, &sp140);
                if (temp_s1->unk_E3E4 != 0x10) {
                    if (temp_s1->unk_E3E4 != 0x20) {
                        if (temp_s1->unk_E3E4 != 0x40) {
                            if (temp_s1->unk_E3E4 == 0x50 || temp_s1->unk_E3E4 != 0x60) {
                                func_80107980(globalCtx, &sp140, XREG(64), XREG(65));
                            }
                        } else if ((temp_s1->unk_E2F8 >= 0x6D) && (temp_s1->unk_E2F8 < 0x73)) {
                            temp_s1->unk_E3E7++;
                            if (temp_s1->unk_E3E7 >= 0x1F) {
                                temp_s1->unk_E3E7 = 2;
                                temp_s1->msgMode = 0x36;
                            }
                        }
                    } else {
                        func_80106D40(globalCtx, 2);
                        func_80107980(globalCtx, &sp140, temp_s1->unk_E3D8, temp_s1->unk_E3DA);
                    }
                } else {
                    func_80106D40(globalCtx, 1);
                    func_80107980(globalCtx, &sp140, temp_s1->unk_E3D8, temp_s1->unk_E3DA);
                }
                break;
            case 54:
            case 55:
                break;
            case 32:
                temp_s1->msgMode = 6;
                break;
        }
        if ((temp_s1->msgMode >= 0xC) && (temp_s1->msgMode < 0x34) && (temp_s1->unk_E3F0 != 1) && (temp_s1->unk_E3F0 != 0x30)) {
            func_8009457C(&sp140);

            gDPSetCombineLERP(sp140++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, 
                                        PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

            if (temp_s1->msgMode == 0x1B) {
                for (phi_a3_8 = 0; phi_a3_8 < D_80131C00[temp_s1->unk_E3F0 - 0xF].len; phi_a3_8++) {
                    temp_a0_9 = D_80131C00[temp_s1->unk_E3F0 - 0xF].notesIdx[phi_a3_8];

                    gDPPipeSync(sp140++);
                    gDPSetPrimColor(sp140++, 0, 0, 150, 150, 150, 150);
                    gDPSetEnvColor(sp140++, 10, 10, 10, 0);
                    gDPSetTextureImage(sp140++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, D_80153C94[temp_a0_9]);
                    gDPSetTile(sp140++, G_IM_FMT_IA, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
                    gDPLoadSync(sp140++);
                    gDPLoadBlock(sp140++, G_TX_LOADTILE, 0, 0, 127, 1024);
                    gDPPipeSync(sp140++);
                    gDPSetTile(sp140++, G_IM_FMT_IA, G_IM_SIZ_8b, 2, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
                    gDPSetTileSize(sp140++, G_TX_RENDERTILE, 0, 0, 0x003C, 0x003C);

                    sp140 = sp140 + 8;
                    sp140->words.w0 = ((((&VREG(35))[temp_a0_9] + 0x10) * 4) & 0xFFF) | 0xE4000000 | ((((VREG(28) + 0x10) * 4) & 0xFFF) << 0xC);
                    sp140->words.w1 = (((&VREG(35))[temp_a0_9] * 4) & 0xFFF) | (((VREG(28) * 4) & 0xFFF) << 0xC);
                    sp140 = sp140 + 8;
                    sp140->words.w0 = 0xE1000000;
                    sp140->words.w1 = 0x00000000;
                    sp140 = sp140 + 8;
                    sp140->words.w0 = 0xF1000000;
                    sp140->words.w1 = 0x04000400;

                    VREG(28) += VREG(29);
                }
            }
            if ((temp_s1->msgMode != 0x21) && (temp_s1->msgMode != 0x29)) {

                if (D_8015394C[phi_a3_9] != 0xFF) {
                    phi_a3_9 = 0;
                    for (phi_a3_9 = 0; phi_a3_9 < 8; phi_a3_9++) {
                        if (D_80153958[phi_a3_9] != 0xFF) {
                            NON_CONST(D_80153958[phi_a3_9],s16) += VREG(50);
                            if (D_80153958[phi_a3_9] >= 0xFF) {
                                NON_CONST(D_80153958[phi_a3_9],s16) = 0xFF;
                            }
                        }

                        gDPPipeSync(sp140++);
                        if (D_8015394C[phi_a3_9] == 0) {
                            gDPSetPrimColor(sp140++, 0, 0, D_801759A8, D_801759AC, D_801759AA, D_80153958[phi_a3_9]);
                            gDPSetEnvColor(sp140++, D_801759AE, D_801759B2, D_801759B0, 0);
                        } else {
                            gDPSetPrimColor(sp140++, 0, 0, D_801759B4, D_801759B8, D_801759B6, D_80153958[phi_a3_9]);
                            gDPSetEnvColor(sp140++, D_801759BA, D_801759BE, D_801759BC, 0);
                        }
                        gDPSetTextureImage(sp140++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, D_80153C94[D_8015394C[phi_a3_9]]);
                        gDPSetTile(sp140++, G_IM_FMT_IA, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
                        gDPLoadSync(sp140++);
                        gDPLoadBlock(sp140++, G_TX_LOADTILE, 0, 0, 127, 1024);
                        gDPPipeSync(sp140++);
                        gDPSetTile(sp140++, G_IM_FMT_IA, G_IM_SIZ_8b, 2, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
                        gDPSetTileSize(sp140++, G_TX_RENDERTILE, 0, 0, 0x003C, 0x003C);

                        sp140 = sp140 + 8;
                        sp140->words.w0 = ((((&VREG(35))[D_8015394C[phi_a3_9]] + 0x10) * 4) & 0xFFF) | 0xE4000000 | ((((VREG(28) + 0x10) * 4) & 0xFFF) << 0xC);
                        sp140->words.w1 = (((&VREG(35))[D_8015394C[phi_a3_9]] * 4) & 0xFFF) | (((VREG(28) * 4) & 0xFFF) << 0xC);
                        sp140 = sp140 + 8;
                        sp140->words.w0 = 0xE1000000;
                        sp140->words.w1 = 0x00000000;
                        sp140 = sp140 + 8;
                        sp140->words.w0 = 0xF1000000;
                        sp140->words.w1 = 0x04000400;

                        VREG(28) += VREG(29);
                    }
                }
            }
        }
    }
    *p = sp140;
}
*/

// Message_DrawSetup ?
#ifdef NON_MATCHING
// Matches but it loads from data instead of rodata
void func_8010F2CC(s16* arg0, GraphicsContext* gfxCtx) {
    static s16 D_80153CF8 = 0;
    static s16 D_80153CFC = 0;
    s32 pad[2];

    OPEN_DISPS(gfxCtx, "../z_message_PAL.c", 3485);
    if (D_80153CF8 != *arg0) {
        D_80153CF8 = *arg0;
        D_80153CFC = 0x1E;
    }
    if (D_80153CFC != 0) {
        D_80153CFC--;
        gDPPipeSync(gfxCtx->polyOpa.p++);
        gDPSetCycleType(gfxCtx->polyOpa.p++, G_CYC_FILL);
        gDPSetRenderMode(gfxCtx->polyOpa.p++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(gfxCtx->polyOpa.p++, 0x00010001);
        gDPFillRectangle(gfxCtx->polyOpa.p++, 0, 110, 319, 150);
        gDPPipeSync(gfxCtx->polyOpa.p++);
        gDPPipeSync(gfxCtx->polyOpa.p++);
        gDPSetCycleType(gfxCtx->polyOpa.p++, G_CYC_FILL);
        gDPSetRenderMode(gfxCtx->polyOpa.p++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(gfxCtx->polyOpa.p++, 0xFFFFFFFF);
        gDPFillRectangle(gfxCtx->polyOpa.p++, 40, 120, 60, 140);
        gDPPipeSync(gfxCtx->polyOpa.p++);
    }
    CLOSE_DISPS(gfxCtx, "../z_message_PAL.c", 35130);
}
#else
void func_8010F2CC(s16* arg0, GraphicsContext* gfxCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_8010F2CC.s")
#endif

// Message_DrawDebug ?
void func_8010F494(GlobalContext* globalCtx, Gfx** p) {
    s32 pad;
    GfxPrint printer;
    s32 pad1;

    GfxPrint_Init(&printer);
    GfxPrint_Open(&printer, *p);
    GfxPrint_SetPos(&printer, 6, 26);
    GfxPrint_SetColor(&printer, 255, 60, 0, 255);
    GfxPrint_Printf(&printer, "%s", "MESSAGE");
    GfxPrint_SetPos(&printer, 14, 26);
    GfxPrint_Printf(&printer, "%s", "=");
    GfxPrint_SetPos(&printer, 16, 26);
    GfxPrint_Printf(&printer, "%x", globalCtx->msgCtx.unk_E2F8);
    *p = GfxPrint_Close(&printer);
    GfxPrint_Destroy(&printer);
}

// Message_Draw ?
void func_8010F58C(GlobalContext* globalCtx) {
    Gfx* plusOne;
    Gfx* polyOpaP;
    s16 sp4E;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_message_PAL.c", 3554);
    sp4E = gSaveContext.scarecrowCustomSongSet;
    func_8010F2CC(&sp4E, globalCtx->state.gfxCtx);
    if (BREG(0) != 0 && globalCtx->msgCtx.unk_E2F8 != 0) {
        plusOne = Graph_GfxPlusOne(polyOpaP = POLY_OPA_DISP);
        gSPDisplayList(OVERLAY_DISP++, plusOne);
        func_8010F494(globalCtx, &plusOne);
        gSPEndDisplayList(plusOne++);
        Graph_BranchDlist(polyOpaP, plusOne);
        POLY_OPA_DISP = plusOne;
    }
    if (1) {}
    plusOne = Graph_GfxPlusOne(polyOpaP = POLY_OPA_DISP);
    gSPDisplayList(OVERLAY_DISP++, plusOne);
    func_8010C39C(globalCtx, &plusOne);
    gSPEndDisplayList(plusOne++);
    Graph_BranchDlist(polyOpaP, plusOne);
    POLY_OPA_DISP = plusOne;
    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_message_PAL.c", 3582);
}

extern u16 D_80153D78;

extern s16 D_80153D00[];
extern s16 D_80153D0C[];
extern s16 D_80153D18[];
extern s16 D_80153D24[];
extern s16 D_80153D30[];

extern s32 D_80153D74;

// Message_Update ?
void func_8010F6F0(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_message_PAL/func_8010F6F0.s")
/*
void func_8010F6F0(GlobalContext *globalCtx) {
    Player *sp4C;
    s16 sp44;
    s16 sp42;
    s16 sp40;
    s16 sp3E;
    Input *temp_v0;
    InterfaceContext *temp_v0_6;
    MessageContext *temp_t6;
    const MessageTableEntry *temp_v0_2;
    s16 temp_a1;
    s16 temp_a1_2;
    s16 temp_v0_11;
    s16 temp_v0_13;
    s32 temp_v0_14;
    s32 temp_v0_15;
    s32 temp_v0_16;
    s32 temp_v0_17;
    s32 temp_v0_18;
    s32 temp_v0_19;
    s32 temp_v0_20;
    s32 temp_v0_21;
    s32 temp_v0_22;
    s32 temp_v0_5;
    s8 temp_t4;
    u16 temp_a0;
    u16 temp_v0_3;
    u16 temp_v0_4;
    u16 temp_v0_9;
    u16 temp_v1;
    u16 temp_v1_2;
    u32 temp_t2;
    u8 temp_v0_10;
    u8 temp_v0_12;
    u8 temp_v0_23;
    u8 temp_v1_3;
    u16 phi_v1;
    s16 phi_a1;
    const MessageTableEntry *phi_v0;
    u16 phi_a0;
    s32 phi_v1_2;
    s32 phi_v0_2;

    sp4C = PLAYER;
    if (BREG(0) != 0) {
        if (CHECK_BTN_ALL(globalCtx->state.input[0].press.button, BTN_DDOWN) && 
            CHECK_BTN_ALL(globalCtx->state.input[0].cur.button, BTN_L)) {
            osSyncPrintf("msgno=%d\n", D_80153D78);
            func_8010B680(globalCtx, YREG(79), NULL);
            D_80153D78 = (D_80153D78 + 1) % 10;
        }
        if (YREG(78) != 0) {
            while (YREG(79) != 0x8000) {
                phi_v0 = &D_801538F0[0];
                while (phi_v0->textId != 0xFFFD) {
                    if (phi_v0->textId == YREG(79)) {
                        // The message was found! !! !!
                        osSyncPrintf(" メッセージが,見つかった！！！ = %x\n", YREG(79));
                        func_8010B680(globalCtx, YREG(79), NULL);
                        YREG(79)++;
                        YREG(78) = 0;
                        return;
                    }
                    phi_v0++;
                }
                YREG(79)++;
            }
        }
    }
    temp_t6 = &globalCtx->msgCtx;
    if (temp_t6->unk_E300 != 0) {
        if (temp_t6->msgMode >= 0x36) {
            if (temp_t6->msgMode != 0x36) {
                if (temp_t6->msgMode != 0x37) {
                    temp_t6->unk_E410 = 0xFF;
                }
            } else {
                temp_t6->unk_E3E7--;
                if (temp_t6->unk_E3E7 == 0) {
                    if (temp_t6->unk_E2F8 >= 0xC2) {
                        if (temp_t6->unk_E2F8 < 0xC7) {
                            gSaveContext.healthAccumulator = 0x140;
                        } else {
                            if (temp_t6->unk_E2F8 >= 0xFA && temp_t6->unk_E2F8 < 0xFE) {
                                gSaveContext.healthAccumulator = 0x140;
                            }
                        }
                    } else {
                        if (temp_t6->unk_E2F8 >= 0xFA && temp_t6->unk_E2F8 < 0xFE) {
                            gSaveContext.healthAccumulator = 0x140;
                        }
                    }
                    if (temp_t6->unk_E2F8 == 0x301F || temp_t6->unk_E2F8 == 0xA || temp_t6->unk_E2F8 == 0xC || 
                        temp_t6->unk_E2F8 == 0xCF || temp_t6->unk_E2F8 == 0x21C || temp_t6->unk_E2F8 == 9 || 
                        temp_t6->unk_E2F8 == 0x4078 || temp_t6->unk_E2F8 == 0x2015 || temp_t6->unk_E2F8 == 0x3040) {
                        gSaveContext.unk_13EE = 0x32;
                    }
                    if (globalCtx->csCtx.state == 0) {
                        osSyncPrintf("\x1b[32m");
                        osSyncPrintf("day_time=%x  active_camera=%d  ", gSaveContext.cutsceneIndex, globalCtx->activeCamera);
                        if (temp_t6->unk_E2F8 != 0x2061 && 
                            temp_t6->unk_E2F8 != 0x2025 && 
                            temp_t6->unk_E2F8 != 0x208C && 
                            (temp_t6->unk_E2F8 < 0x088D || temp_t6->unk_E2F8 >= 0x893 || temp_t6->choiceIndex != 0) &&
                            temp_t6->unk_E2F8 != 0x3055) {
                            if (gSaveContext.cutsceneIndex < 0xFFF0) {
                                osSyncPrintf("=== day_time=%x ", gSaveContext.cutsceneIndex);
                                if (globalCtx->activeCamera == 0) {
                                    if ((gSaveContext.unk_13EE == 0) || (gSaveContext.unk_13EE == 1) || (gSaveContext.unk_13EE == 2)) {
                                        gSaveContext.unk_13EE = 0x32;
                                    }
                                    gSaveContext.unk_13EA = 0;
                                    Interface_ChangeAlpha(gSaveContext.unk_13EE);
                                }
                            }
                        }
                    }
                    osSyncPrintf("\x1b[m");
                    temp_t6->unk_E300 = 0;
                    temp_t6->msgMode = 0;
                    globalCtx->interfaceCtx.unk_1FA = globalCtx->interfaceCtx.unk_1FC = 0;
                    temp_t6->unk_E2F8 = temp_t6->unk_E3E7 = 0;
                    if (temp_t6->unk_E3E4 == 0x40) {
                        temp_t6->unk_E3E4 = 0;
                        globalCtx->msgCtx.unk_E3EE = 2;
                    } else {
                        temp_t6->unk_E3E4 = 0;
                    }
                    if ((gSaveContext.inventory.questItems & 0xF0000000) == 0x40000000) {
                        gSaveContext.inventory.questItems ^= 0x40000000;
                        gSaveContext.healthCapacity += 0x10;
                        gSaveContext.health += 0x10;
                    }
                    if (temp_t6->unk_E3F0 != 0x31) {
                        if (D_8014B310 == 6) {
                            sp4C->naviTextId = -0xE0;
                            sp4C->naviActor->flags |= 0x10000;
                        }
                        if (temp_t6->unk_E3F0 == 0x29 && (globalCtx->msgCtx.unk_E3EE == 1 || globalCtx->msgCtx.unk_E3EE == 0xB)) {
                            if (temp_t6->unk_E3F2 == 9) {
                                globalCtx->msgCtx.unk_E3EE = 1;
                            } else {
                                globalCtx->msgCtx.unk_E3EE = 4;
                            }
                        }
                    }
                    D_8014B310 = 0xFF;
                    osSyncPrintf("OCARINA_MODE=%d   chk_ocarina_no=%d\n", globalCtx->msgCtx.unk_E3EE, temp_t6->unk_E3F2);
                }
            }
        } else if (temp_t6->msgMode >= 0x35) {
            if (temp_t6->msgMode != 0x35) {
                temp_t6->unk_E410 = 0xFF;
            } else if (temp_t6->unk_E3E4 == 0x60) {
                temp_t6->unk_E3E7--;
                if (temp_t6->unk_E3E7 == 0) {
                    func_80106CCC(globalCtx);
                }
            } else if ((temp_t6->unk_E3E4 != 0x40) && (temp_t6->unk_E3E4 != 0x50) && (YREG(31) == 0)) {
                if ((temp_t6->unk_E3E4 == 0x10) && (globalCtx->msgCtx.unk_E3EE == 1)) {
                    if (func_80106BC8(globalCtx)) {
                        osSyncPrintf("OCARINA_MODE=%d -> ", globalCtx->msgCtx.unk_E3EE);
                        if (temp_t6->choiceIndex == 0) {
                            globalCtx->msgCtx.unk_E3EE = 2;
                        } else {
                            globalCtx->msgCtx.unk_E3EE = 4;
                        }
                        osSyncPrintf("InRaceSeq=%d(%d) OCARINA_MODE=%d  -->  ", gSaveContext.eventInf[0] & 0xF, 1);
                        func_80106CCC(globalCtx);
                        osSyncPrintf("OCARINA_MODE=%d\n", globalCtx->msgCtx.unk_E3EE);
                    }
                } else {
                    if (func_80106C88(globalCtx)) {
                        osSyncPrintf("select=%d\n", temp_t6->unk_E3E4);
                        if (temp_t6->unk_E3E4 != 0x30) {
                            Audio_PlaySoundGeneral(NA_SE_SY_DECIDE, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                            func_80106CCC(globalCtx);
                        } else {
                            Audio_PlaySoundGeneral(NA_SE_SY_MESSAGE_PASS, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
                            func_8010B720(globalCtx, D_8014B304);
                        }
                    }
                }
            }
        } else if (temp_t6->msgMode >= 9) {
            if (temp_t6->msgMode != 0x34) {
                temp_t6->unk_E410 = 0xFF;
            } else if (func_80106BC8(globalCtx)) {
                temp_t6->msgMode = 4;
                temp_t6->unk_E3D6 = 0;
                temp_t6->unk_E3CE++;
            }
        } else if (temp_t6->msgMode < 9) {
            switch (temp_t6->msgMode) {
                case 1:
                    D_8014B2F4++;
                    if (YREG(15) == 0x40) {
                        if ((D_8014B2F4 & 0xFF) >= 4) {
                            phi_v1_2 = 1;
                        } else {
                            phi_v1_2 = 0;
                        }
                    } else if ((YREG(15) != 0) || (globalCtx->sceneNum == 0x45)) {
                        phi_v1_2 = 1;
                    } else if ((D_8014B2F4 >= 4) || (phi_v1_2 = 0, (temp_t6->unk_E408 == 0))) {
                        phi_v1_2 = 1;
                    }
                    if (phi_v1_2 != 0) {
                        if (temp_t6->unk_E408 != 0) {
                            func_8002F374(globalCtx, PLAYER, &sp44, &sp40);
                            func_8002F374(globalCtx, temp_t6->unk_E408, &sp44, &sp3E);
                            if (sp40 >= sp3E) {
                                sp42 = ((sp40 - sp3E) / 2) + sp3E;
                            } else {
                                sp42 = ((sp3E - sp40) / 2) + sp40;
                            }
                            osSyncPrintf("dxpos=%d   dypos=%d  dypos1  dypos2=%d\n", sp44, sp42, sp40, sp3E);
                        } else {
                            VREG(0) = XREG(72);
                            VREG(1) = XREG(73);
                        }
                        if (temp_t6->unk_E2FE == 0) {
                            if ((YREG(15) != 0) || (globalCtx->sceneNum == 0x45)) {
                                if (sp42 < XREG(92)) {
                                    XREG(73) = D_80153D0C[temp_t6->unk_E2FD];
                                } else {
                                    XREG(73) = D_80153D18[temp_t6->unk_E2FD];
                                }
                            } else if ((globalCtx->sceneNum == 0x20) || (globalCtx->sceneNum == 0x21) || (globalCtx->sceneNum == 0x22)) {
                                if (sp42 < XREG(93)) {
                                    XREG(73) = D_80153D0C[temp_t6->unk_E2FD];
                                } else {
                                    XREG(73) = D_80153D18[temp_t6->unk_E2FD];
                                }
                            } else if (sp42 < XREG(94)) {
                                XREG(73) = D_80153D0C[temp_t6->unk_E2FD];
                            } else {
                                XREG(73) = D_80153D18[temp_t6->unk_E2FD];
                            }
                        } else if (temp_t6->unk_E2FE == 1) {
                            XREG(73) = D_80153D18[temp_t6->unk_E2FD];
                        } else if (temp_t6->unk_E2FE == 2) {
                            XREG(73) = D_80153D24[temp_t6->unk_E2FD];
                        } else {
                            XREG(73) = D_80153D0C[temp_t6->unk_E2FD];
                        }
                        XREG(72) = D_80153D00[temp_t6->unk_E2FD];
                        XREG(65) = XREG(73) + D_80153D30[temp_t6->unk_E2FD];
                        XREG(67) = XREG(73) + 0x14;
                        XREG(68) = XREG(73) + 0x20;
                        XREG(69) = XREG(73) + 0x2C;
                        osSyncPrintf("message->msg_disp_type=%x\n", temp_t6->unk_E2FC & 0xF0);
                        if (temp_t6->unk_E2FD != 4 && temp_t6->unk_E2FD != 5) {
                            func_80107244(globalCtx);
                            func_800F691C(0);
                            temp_t6->unk_E3E7 = 0;
                            temp_t6->msgMode = 2;
                        } else {
                            temp_t6->msgMode = 3;
                            VREG(0) = XREG(72);
                            VREG(1) = XREG(73);
                            YREG(22) = 0x100;
                            YREG(23) = 0x40;
                            YREG(16) = 0x200;
                            YREG(17) = 0x200;
                        }
                    }
                    break;
                case 2:
                    func_80107244(globalCtx);
                    break;
                case 3:
                    temp_t6->msgMode = 4;
                    if (YREG(31) == 0) {
                        Interface_SetDoAction(globalCtx, 0x10);
                    }
                    break;
                case 4:
                    func_80109B3C(globalCtx);
                    if (D_8014B2F0 != 0) {
                        Interface_ChangeAlpha(1);
                    }
                    if (D_80153D74 != 0) {
                        temp_t6->unk_E3D2 = temp_t6->unk_E3D4;
                        D_80153D74 = 0;
                    }
                    break;
                case 5:
                    temp_t6->unk_E3E7--;
                    if (temp_t6->unk_E3E7 == 0) {
                        func_80109B3C(globalCtx);
                    }
                    break;
                case 6:
                    if (temp_t6->unk_E2FD != 4 || YREG(31) == 0 || ~(globalCtx->state.input[0].press.button | ~0x4000) == 0 || temp_t6->unk_E3D6 == 0) {
                        D_8014B300 = 1;
                        temp_t6->unk_E3D2 = temp_t6->unk_E3D4;
                    }
                    break;
                case 7:
                    if (YREG(31) == 0 && func_80106BC8(globalCtx) != 0) {
                        temp_t6->msgMode = 6;
                        temp_t6->unk_E3D2++;
                    }
                    break;
                case 8:
                    temp_t6->unk_E3E7--;
                    if (temp_t6->unk_E3E7 == 0) {
                        temp_t6->msgMode = 4;
                    }
                    break;
            }
        } else {
            temp_t6->unk_E410 = 0xFF;
        }
    }
}
*/

void func_8011040C(void) {
    NON_CONST(D_801538F0, MessageTableEntry*) = &D_8014B320;
    NON_CONST(D_801538F4, u32*) = &D_8014F548;
    NON_CONST(D_801538F8, u32*) = &D_80151658;
    NON_CONST(D_801538FC, MessageTableEntry*) = &D_80153768;
}
