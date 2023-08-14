.rsp
#include "rsp.inc"
#include "rcp.h"
#include "gbi.h"

// TODO this is in gs2dex.h
#define G_OBJ_LOADTXTR 0x05

#define UOBJBG_IMAGEX(p)        ((p) + 0x00)
#define UOBJBG_IMAGEW(p)        ((p) + 0x02)
#define UOBJBG_FRAMEX(p)        ((p) + 0x04)
#define UOBJBG_FRAMEW(p)        ((p) + 0x06)
#define UOBJBG_IMAGEY(p)        ((p) + 0x08)
#define UOBJBG_IMAGEH(p)        ((p) + 0x0A)
#define UOBJBG_FRAMEY(p)        ((p) + 0x0C)
#define UOBJBG_FRAMEH(p)        ((p) + 0x0E)
#define UOBJBG_IMAGEPTR(p)      ((p) + 0x10)
#define UOBJBG_IMAGELOAD(p)     ((p) + 0x14)
#define UOBJBG_IMAGEFMT(p)      ((p) + 0x16)
#define UOBJBG_IMAGESIZ(p)      ((p) + 0x17)
#define UOBJBG_IMAGEPAL(p)      ((p) + 0x18)
#define UOBJBG_IMAGEFLIP(p)     ((p) + 0x1A)
#define UOBJBG_TMEMW(p)         ((p) + 0x1C)
#define UOBJBG_TMEMH(p)         ((p) + 0x1E)
#define UOBJBG_TMEMLOADSH(p)    ((p) + 0x20)
#define UOBJBG_TMEMLOADTH(p)    ((p) + 0x22)
#define UOBJBG_TMEMSIZEW(p)     ((p) + 0x24)
#define UOBJBG_TMEMSIZE(p)      ((p) + 0x26)

// data macros
.macro jumpTableEntry, addr
    .dh addr & 0xFFFF
.endmacro

overlay_load equ 0x0000
overlay_len  equ 0x0004
overlay_imem equ 0x0006
.macro OverlayEntry, loadStart, loadEnd, imemAddr
    .dw loadStart
    .dh (loadEnd - loadStart - 1) & 0xFFFF
    .dh (imemAddr) & 0xFFFF
.endmacro

// scalar macros
.macro li, reg, imm
    addi reg, $zero, imm
.endmacro

.macro la, reg, imm
    addiu reg, $zero, imm
.endmacro

.macro move, dst, src
    ori dst, src, 0
.endmacro

// vector macros
.macro vclr, dst
    vxor dst, dst, dst
.endmacro

ACC_LOWER equ 0
ACC_MIDDLE equ 1
ACC_UPPER equ 2
.macro vreadacc, dst, N
    vsar dst, dst, dst[N]
.endmacro

.create DATA_FILE, DMEM_START

data_0000:
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 

data_00C0: // scissor
    .dw G_SETSCISSOR << 24
data_00C4:
    .dw ((G_SC_NON_INTERLACE) << 24) | ((320 << 2) << 12) | ((240 << 2) << 0)

data_00C8: // othermode
    .dw (G_RDPSETOTHERMODE << 24) | 0x080CFF
data_00CC:
    .dw 0x00000000

data_00D0:
    .dh 0x0000
    .dh 0x0000
data_00D4:
    .dw 0x00000000

data_00D8:
    .dw 0x00000000

    .dh 0xFFFF

data_00DE: // dl stack depth?
    .dh 0x0048

data_00E0:
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

data_00F0:
    .dw 0x00000000

data_00F4:
    .dw 0x00000000

data_00F8: // segment table
    .dw 0x00000000, 0x00000000, 0x00000000, 0x00000000
    .dw 0x00000000, 0x00000000, 0x00000000, 0x00000000
    .dw 0x00000000, 0x00000000, 0x00000000, 0x00000000
    .dw 0x00000000, 0x00000000, 0x00000000, 0x00000000

data_0138:
    .ascii "RSP Gfx[Safe] S2DEX       fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo.", 0x0A
    .align 16

data_0180:
    .dh 0x0800, 0x0040, 0x0002, 0xFFFF, 0x4000, 0x0003, 0x0008, 0xFFF8

data_0190:
    .dh 0x0100, 0x8000, 0xFE00, 0x0000, 0x1000, 0x0400, 0x2000, 0x0200

data_01A0:
    .dh 0x3D10, 0x0001
    .dw (G_SETTILESIZE & 0x3F) << 24
    .dw (G_RDPPIPESYNC & 0x3F) << 24
    .db G_SETTILE & 0x3F
    .db G_TEXRECT & 0x3F
    .db G_RDPLOADSYNC & 0x3F
    .db G_RDPTILESYNC & 0x3F

data_01B0:
    .dh 0xFFFC, 0x0400, 0x0020, 0x0010, 0xFC00, 0x03FF, 0x001F, 0xFFE0

data_01C0:
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .dh 0x0001, 0x0000, 0x0000, 0x0000

data_01E8:
    .dw 0x00000000

data_01EC:
    .dw 0x00010000

data_01F0:
    .dh 0x0000, 0x0000, 0x0400, 0x0400

data_01F8:
    .dh 0x0000, 0x0000

data_01FC:
    .dh 0x0001, 0xFFFE, 0x0020, 0xFFFF

data_0204:
    .dh 0
data_0206:
    .dh 320 << 2
data_0208:
    .dh 0
data_020A:
    .dh 240 << 2

data_020C:
// 0x020C
    .dh next_cmd, func_000016F0, func_00001BB0

// 0x0212
    .dh         0x9BB0, 0x0000, 0x0000, 0x0010, 0x0020, 0x0020, 0x0040
    .dh 0x0030, 0x0060, 0x0000, 0xFFF4, 0x0010, 0x0014, 0x0020, 0x0034
    .dh 0x0030, 0x0054, 0x0001, 0xFFFE, 0xFFFE, 0xFFFE, 0x0001, 0x0000
    .dh 0x0000, 0x0000

data_0244: 
// 0x0244
    .dh 0x0248

data_0246:
// 0x0246
    .dh 0x0258

data_0248:
// 0x0248
    .dh 0xFFFC, 0x0000, 0x0000, 0x0001, 0xFFFF, 0x0003, 0xFFF0, 0x0000
    .dh 0x0001, 0xFFFF, 0x0008, 0x0200, 0xFFFF, 0x0001

data_0264:
// 0x0264
    .dw 0x0007C1F0

data_0268:
// 0x0268
    .dh 0x0000, 0x0800, 0x0800, 0x0400, 0x0800, 0x0800, 0x003F, 0x0800
    .dh 0x1000, 0x0080, 0x001F, 0x1000, 0x2000, 0x0100, 0x000F, 0x2000
    .dh 0x4000, 0x0200, 0x0007, 0x4000, 0x8000, 0x0400, 0x01FF, 0x0080
    .dh 0x00FF, 0x0100, 0x007F, 0x0200, 0x003F, 0x0400, 0x0400, 0x0400
    .dh 0x0200, 0x0400, 0x0400

data_02AE:
// 0x02AE
    .db 0x80

data_02AF:
// 0x02AF    
    .db 0x02

data_02B0:
// 0x02B0
    .dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .dh 0x0001, 0x0000, 0x0000, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001
    .dh 0x0000, 0x0001, 0x0001, 0x0000, 0x0000, 0x0001, 0x0000, 0x0001
    .dh 0x0000, 0x0001, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001, 0x0000
    .dh 0x0001, 0x0000, 0x0000, 0x0001, 0x0001, 0x0000, 0x0001, 0x0000
    .dh 0x0000, 0x0000

Overlay0Info:
    OverlayEntry orga(Overlay0Address), orga(Overlay0End), Overlay0Address
Overlay1Info:
    OverlayEntry orga(Overlay1Address), orga(Overlay1End), Overlay1Address

Overlay2Info:
    OverlayEntry orga(Overlay2Address), orga(Overlay2End), Overlay2Address
Overlay3Info:
    OverlayEntry orga(Overlay3Address), orga(Overlay3End), Overlay3Address

// 0x0324
jumpTableEntry G_OBJ_RECTANGLE_R_handler    // 0xDA G_OBJ_RECTANGLE_R
jumpTableEntry G_MOVEWORD_handler           // 0xDB G_MOVEWORD
jumpTableEntry G_OBJ_MOVEMEM_handler        // 0xDC G_OBJ_MOVEMEM
jumpTableEntry G_LOAD_UCODE_handler         // 0xDD G_LOAD_UCODE
jumpTableEntry G_DL_handler                 // 0xDE G_DL
jumpTableEntry G_ENDDL_handler              // 0xDF G_ENDDL
jumpTableEntry G_SPNOOP_handler             // 0xE0 G_SPNOOP
jumpTableEntry G_RDPHALF_1_handler          // 0xE1 G_RDPHALF_1
jumpTableEntry G_SETOTHERMODE_L_handler     // 0xE2 G_SETOTHERMODE_L
jumpTableEntry G_SETOTHERMODE_H_handler     // 0xE3 G_SETOTHERMODE_H
jumpTableEntry G_RDPHALF_0_handler          // 0xE4 G_RDPHALF_0
jumpTableEntry G_TEXRECTFLIP_handler        // 0xE5 G_TEXRECTFLIP
jumpTableEntry G_SYNC_handler               // 0xE6 G_RDPLOADSYNC
jumpTableEntry G_SYNC_handler               // 0xE7 G_RDPPIPESYNC
jumpTableEntry G_SYNC_handler               // 0xE8 G_RDPTILESYNC
jumpTableEntry G_SYNC_handler               // 0xE9 G_RDPFULLSYNC
jumpTableEntry G_RDP_handler                // 0xEA G_SETKEYGB
jumpTableEntry G_RDP_handler                // 0xEB G_SETKEYR
jumpTableEntry G_RDP_handler                // 0xEC G_SETCONVERT
jumpTableEntry G_SETSCISSOR_handler         // 0xED G_SETSCISSOR
jumpTableEntry G_RDP_handler                // 0xEE G_SETPRIMDEPTH
jumpTableEntry G_RDPSETOTHERMODE_handler    // 0xEF G_RDPSETOTHERMODE
jumpTableEntry G_RDP_handler                // 0xF0 G_LOADTLUT
jumpTableEntry G_RDPHALF_2_handler          // 0xF1 G_RDPHALF_2
jumpTableEntry G_RDP_handler                // 0xF2 G_SETTILESIZE
jumpTableEntry G_RDP_handler                // 0xF3 G_LOADBLOCK
jumpTableEntry G_RDP_handler                // 0xF4 G_LOADTILE
jumpTableEntry G_RDP_handler                // 0xF5 G_SETTILE
jumpTableEntry G_RDP_handler                // 0xF6 G_FILLRECT
jumpTableEntry G_RDP_handler                // 0xF7 G_SETFILLCOLOR
jumpTableEntry G_RDP_handler                // 0xF8 G_SETFOGCOLOR
jumpTableEntry G_RDP_handler                // 0xF9 G_SETBLENDCOLOR
jumpTableEntry G_RDP_handler                // 0xFA G_SETPRIMCOLOR
jumpTableEntry G_RDP_handler                // 0xFB G_SETENVCOLOR
jumpTableEntry G_RDP_handler                // 0xFC G_SETCOMBINE
jumpTableEntry G_SETxIMG_handler            // 0xFD G_SETTIMG
jumpTableEntry G_SETxIMG_handler            // 0xFE G_SETZIMG
jumpTableEntry G_SETxIMG_handler            // 0xFF G_SETCIMG
commandJumpTable: // 0x0370
jumpTableEntry G_NOOP_handler               // 0x00 G_NOOP
jumpTableEntry G_OBJ_RECTANGLE_handler      // 0x01 G_OBJ_RECTANGLE
jumpTableEntry G_OBJ_SPRITE_handler         // 0x02 G_OBJ_SPRITE
jumpTableEntry G_CULLDL_handler             // 0x03 G_CULLDL
jumpTableEntry G_SELECT_DL_handler          // 0x04 G_SELECT_DL
jumpTableEntry G_OBJ_LOADTXTR_handler       // 0x05 G_OBJ_LOADTXTR
jumpTableEntry G_OBJ_LDTX_SPRITE_handler    // 0x06 G_OBJ_LDTX_SPRITE
jumpTableEntry G_OBJ_LDTX_RECT_handler      // 0x07 G_OBJ_LDTX_RECT
jumpTableEntry G_OBJ_LDTX_RECT_R_handler    // 0x08 G_OBJ_LDTX_RECT_R
jumpTableEntry G_BG_1CYC_handler            // 0x09 G_BG_1CYC
jumpTableEntry G_BG_COPY_handler            // 0x0A G_BG_COPY
jumpTableEntry G_OBJ_RENDERMODE_handler     // 0x0B G_OBJ_RENDERMODE

data_0388: // movemem table
    .dh 0x01E0 // ObjMatrix, len 0x18
    .dh 0x01F0 // ?
    .dh 0x00F8 // ObjSubMatrix, len 8
    .dh 0x02B0 // ?

data_0390:
    .skip 0x18

data_03A8:
    .skip 8

bg: // uObjBg (0x28 bytes)
data_03B0: // imageX
    .skip 2
data_03B2: // imageW
    .skip 2
data_03B4: // frameX
    .skip 2
data_03B6: // frameW
    .skip 2
data_03B8: // imageY
    .skip 2
data_03BA: // imageH
    .skip 2
data_03BC: // frameY
    .skip 2
data_03BE: // frameH
    .skip 2
data_03C0: // imagePtr
    .skip 4
data_03C4: // imageLoad
    .skip 2
data_03C6: // imageFmt
    .skip 1
data_03C7: // imageSiz
    .skip 1
data_03C8: // imagePal
    .skip 2
data_03CA: // imageFlip
    .skip 2
data_03CC: // tmemW
    .skip 2
data_03CE: // tmemH
    .skip 2
data_03D0: // tmemLoadSH
    .skip 2
data_03D2: // tmemLoadTH
    .skip 2
data_03D4: // tmemSizeW
    .skip 2
data_03D6: // tmemSize
    .skip 2

// another uObjBg ?
data_03D8:
// 0x03DA
// 0x03DC
// 0x03DE
// 0x03E0
// 0x03E1
// 0x03E2
// 0x03E4
// 0x03E8
// 0x03EC
// 0x03F0
// 0x03F4
// 0x03F6
// 0x03F8
// 0x03FC

// 0x0400
// 0x0402
// 0x0404
// 0x0408
// 0x040C
// 0x040E

.org 0x4C0
data_04C0:
// 0x04C0

.org 0x7D8
data_07D8:

.org 0xBF8
data_0BF8:
    .skip 4
data_0BFC:
    .skip 4
// 0x0BF8
// 0x0BFC

// 0x0FC4
// 0x0FD0
// 0x0FE0
// 0x0FE8
// 0x0FEC
// 0x0FF0
// 0x0FF8

.close

.create CODE_FILE, RSPBOOT_ENTRYPOINT

// Global registers
vzero equ v0

cmd_w0 equ $25
cmd_w1 equ $24

entry:
/* 00001080 000000 4A00002C */  vclr    $vzero
/* 00001084 000004 C81F2018 */  lqv     $v31[0], (data_0180)($zero)
/* 00001088 000008 C81E2019 */  lqv     $v30[0], (data_0190)($zero)
/* 0000108C 00000C C81D201A */  lqv     $v29[0], (data_01A0)($zero)
/* 00001090 000010 C81C201B */  lqv     $v28[0], (data_01B0)($zero)
/* 00001094 000014 201E01C0 */  li      $30, data_01C0
/* 00001098 000018 201704C0 */  li      $23, data_04C0
/* 0000109C 00001C 201607D8 */  li      $22, data_07D8
/* 000010A0 000020 4B7F0051 */  vsub    $v1, $vzero, $v31[3]
/* 000010A4 000024 8C0B00F0 */  lw      $11, data_00F0
/* 000010A8 000028 8C0C0FC4 */  lw      $12, OSTask_addr + OS_TASK_OFF_FLAGS
/* 000010AC 00002C 20012800 */  li      $1, SP_CLR_SIG1 | SP_CLR_SIG2
/* 000010B0 000030 11600006 */  beqz    $11, .L000010CC
/* 000010B4 000034 40812000 */   mtc0   $1, SP_STATUS
/* 000010B8 000038 318C0001 */  andi    $12, $12, OS_TASK_YIELDED
/* 000010BC 00003C 1180001E */  beqz    $12, .L00001138
/* 000010C0 000040 AC000FC4 */   sw     $zero, OSTask_addr + OS_TASK_OFF_FLAGS
/* 000010C4 000044 0800045F */  j       .L0000117C
/* 000010C8 000048 8C1A0BF8 */   lw     $26, data_0BF8
.L000010CC:
/* 000010CC 00004C 400B5800 */  mfc0    $11, DPC_STATUS
/* 000010D0 000050 316B0001 */  andi    $11, $11, DPC_STATUS_XBUS_DMEM_DMA
/* 000010D4 000054 1560000B */  bnez    $11, .L00001104
/* 000010D8 000058 40024800 */   mfc0   $2, DPC_END
/* 000010DC 00005C 8C030FE8 */  lw      $3, OSTask_addr + OS_TASK_OFF_OUTBUFF
/* 000010E0 000060 00625822 */  sub     $11, $3, $2
/* 000010E4 000064 1D600007 */  bgtz    $11, .L00001104
/* 000010E8 000068 40015000 */   mfc0   $1, DPC_CURRENT
/* 000010EC 00006C 8C040FEC */  lw      $4, OSTask_addr + OS_TASK_OFF_OUTBUFF_SZ
/* 000010F0 000070 10200004 */  beqz    $1, .L00001104
/* 000010F4 000074 00245822 */   sub    $11, $1, $4
/* 000010F8 000078 05610002 */  bgez    $11, .L00001104
/* 000010FC 00007C 00000000 */   nop    
/* 00001100 000080 14220008 */  bne     $1, $2, .L00001124
.L00001104:
/* 00001104 000084 400B5800 */   mfc0   $11, DPC_STATUS
/* 00001108 000088 316B0400 */  andi    $11, $11, DPC_STATUS_START_VALID
/* 0000110C 00008C 1560FFFD */  bnez    $11, .L00001104
/* 00001110 000090 200B0001 */   li     $11, DPC_CLR_XBUS_DMEM_DMA
/* 00001114 000094 408B5800 */  mtc0    $11, DPC_STATUS
/* 00001118 000098 8C020FEC */  lw      $2, OSTask_addr + OS_TASK_OFF_OUTBUFF_SZ
/* 0000111C 00009C 40824000 */  mtc0    $2, DPC_START
/* 00001120 0000A0 40824800 */  mtc0    $2, DPC_END
.L00001124:
/* 00001124 0000A4 AC0200F0 */  sw      $2, data_00F0
/* 00001128 0000A8 8C0B00F4 */  lw      $11, data_00F4
/* 0000112C 0000AC 15600002 */  bnez    $11, .L00001138
/* 00001130 0000B0 8C0B0FE0 */   lw     $11, OSTask_addr + OS_TASK_OFF_STACK
/* 00001134 0000B4 AC0B00F4 */  sw      $11, data_00F4
.L00001138:
/* 00001138 0000B8 8C010FD0 */  lw      $1, OSTask_addr + OS_TASK_OFF_UCODE     // Relocate overlay DRAM addresses
/* 0000113C 0000BC 8C020304 */  lw      $2, Overlay0Info
/* 00001140 0000C0 8C03030C */  lw      $3, Overlay1Info
/* 00001144 0000C4 8C040314 */  lw      $4, Overlay2Info
/* 00001148 0000C8 8C05031C */  lw      $5, Overlay3Info
/* 0000114C 0000CC 00411020 */  add     $2, $2, $1
/* 00001150 0000D0 00611820 */  add     $3, $3, $1
/* 00001154 0000D4 AC020304 */  sw      $2, Overlay0Info
/* 00001158 0000D8 AC03030C */  sw      $3, Overlay1Info
/* 0000115C 0000DC 00812020 */  add     $4, $4, $1
/* 00001160 0000E0 00A12820 */  add     $5, $5, $1
/* 00001164 0000E4 AC040314 */  sw      $4, Overlay2Info
/* 00001168 0000E8 AC05031C */  sw      $5, Overlay3Info
/* 0000116C 0000EC 8C1900C0 */  lw      $25, data_00C0
/* 00001170 0000F0 0C000477 */  jal     func_000011DC
/* 00001174 0000F4 8C1800C4 */   lw     $24, data_00C4
/* 00001178 0000F8 8C1A0FF0 */  lw      $26, OSTask_addr + OS_TASK_OFF_DATA
.L0000117C:
/* 0000117C 0000FC 200B030C */  li      $11, Overlay1Info
/* 00001180 000100 0C0007ED */  jal     load_overlay_and_enter
/* 00001184 000104 37EC0000 */   move   $12, $ra
.L00001188:
/* 00001188 000108 201300A7 */  li      $19, 0xa8-1
/* 0000118C 00010C 37580000 */  move    $24, $26
/* 00001190 000110 0C0007F6 */  jal     dma_read_write
/* 00001194 000114 24140418 */   la     $20, 0x418
/* 00001198 000118 275A00A8 */  addiu   $26, $26, 0xa8
/* 0000119C 00011C 201BFF58 */  li      $27, -0xa8
dma_wait_and_next_cmd:
/* 000011A0 000120 0C0007F2 */  jal     dma_wait
G_CULLDL_handler:
G_SPNOOP_handler:
next_cmd:
/* 000011A4 000124 40012000 */   mfc0   $1, SP_STATUS
/* 000011A8 000128 8F7904C0 */  lw      cmd_w0, (data_04C0)($27)
/* 000011AC 00012C 1360FFF6 */  beqz    $27, .L00001188
/* 000011B0 000130 30210080 */   andi   $1, $1, SP_STATUS_YIELD
/* 000011B4 000134 00196603 */  sra     $12, cmd_w0, 0x18
/* 000011B8 000138 000C5840 */  sll     $11, $12, 1
/* 000011BC 00013C 956B0370 */  lhu     $11, (commandJumpTable)($11)
/* 000011C0 000140 1420037A */  bnez    $1, load_overlay_0_and_enter
/* 000011C4 000144 8F7804C4 */   lw     cmd_w1, (data_04C0 + 4)($27)
/* 000011C8 000148 01600008 */  jr      $11
/* 000011CC 00014C 277B0008 */   addiu  $27, $27, 8

G_SETSCISSOR_handler:
/* 000011D0 000150 AC1900C0 */  sw      cmd_w0, data_00C0
/* 000011D4 000154 AC1800C4 */  sw      cmd_w1, data_00C4
/* 000011D8 000158 201F1140 */  li      $ra, G_RDP_handler
func_000011DC:
/* 000011DC 00015C 332B0FFF */  andi    $11, cmd_w0, 0xfff
/* 000011E0 000160 A40B0208 */  sh      $11, data_0208      // uly
/* 000011E4 000164 00195B02 */  srl     $11, cmd_w0, 12
/* 000011E8 000168 316B0FFF */  andi    $11, $11, 0xfff
/* 000011EC 00016C A40B0204 */  sh      $11, data_0204      // ulx
/* 000011F0 000170 330B0FFF */  andi    $11, cmd_w1, 0xfff
/* 000011F4 000174 A40B020A */  sh      $11, data_020A      // lry
/* 000011F8 000178 00185B02 */  srl     $11, cmd_w1, 12
/* 000011FC 00017C 316B0FFF */  andi    $11, $11, 0xfff
/* 00001200 000180 03E00008 */  jr      $ra
/* 00001204 000184 A40B0206 */   sh     $11, data_0206      // lrx

/**
 * Renders background texture in COPY mode
 */
G_BG_COPY_handler:
/* 00001208 000188 0C000455 */  jal     segmented_to_physical   // Convert bg segmented address to physical address
/* 0000120C 00018C 201403B0 */   li     $20, bg
/* 00001210 000190 0C0007F6 */  jal     dma_read_write          // Read bg struct into DMEM
/* 00001214 000194 20130027 */   li     $19, 0x28-1 // sizeof(uObjBg)-1
/* 00001218 000198 CBC21211 */  llv     $v2[4], 0x44($30)
/* 0000121C 00019C 0C0007F2 */  jal     dma_wait
/* 00001220 0001A0 CBC21612 */   llv    $v2[12], 0x48($30)
/* 00001224 0001A4 CA882000 */  lqv     $v8[0], 0x0($20)
/* 00001228 0001A8 900103CB */  lbu     $1, UOBJBG_IMAGEFLIP(bg) + 1
/* 0000122C 0001AC 4BDE42C5 */  vmudm   $v11, $v8, $v30[6]
/* 00001230 0001B0 4A024251 */  vsub    $v9, $v8, $v2
/* 00001234 0001B4 4AC80A8E */  vmadn   $v10, $v1, $v8[2h]
/* 00001238 0001B8 4AA80190 */  vadd    $v6, $vzero, $v8[1h]
/* 0000123C 0001BC 4B1C5AE8 */  vand    $v11, $v11, $v28[0]
/* 00001240 0001C0 4AC90260 */  vlt     $v9, $vzero, $v9[2h]
/* 00001244 0001C4 4AEA02A3 */  vge     $v10, $vzero, $v10[3h]
/* 00001248 0001C8 4A0940D1 */  vsub    $v3, $v8, $v9
/* 0000124C 0001CC 4A095AD1 */  vsub    $v11, $v11, $v9
/* 00001250 0001D0 4B7F314E */  vmadn   $v5, $v6, $v31[3]
/* 00001254 0001D4 4A0941D0 */  vadd    $v7, $v8, $v9
/* 00001258 0001D8 4B7F51CE */  vmadn   $v7, $v10, $v31[3]
/* 0000125C 0001DC 4A0706E3 */  vge     $v27, $vzero, $v7
/* 00001260 0001E0 484B0800 */  cfc2    $11, $vcc
/* 00001264 0001E4 14200002 */  bnez    $1, .L00001270      // (u8)bg->imageFlip != 0
/* 00001268 0001E8 4A0A5910 */   vadd   $v4, $v11, $v10
/* 0000126C 0001EC 4B0B4133 */  vmov    $v4[0], $v11[0]
.L00001270:
/* 00001270 0001F0 316B0088 */  andi    $11, $11, 0x88
/* 00001274 0001F4 4B7D2EE3 */  vge     $v27, $v5, $v29[3]
/* 00001278 0001F8 1560FFCA */  bnez    $11, next_cmd
/* 0000127C 0001FC 4A0B2967 */   vmrg   $v5, $v5, $v11
/* 00001280 000200 940203CC */  lhu     $2, UOBJBG_TMEMW(bg)
/* 00001284 000204 800503C4 */  lb      $5, UOBJBG_IMAGELOAD(bg)
/* 00001288 000208 3C033510 */  lui     $3, 0x3510
/* 0000128C 00020C 00021240 */  sll     $2, $2, 9
/* 00001290 000210 00452824 */  and     $5, $2, $5
/* 00001294 000214 00651825 */  or      $3, $3, $5
/* 00001298 000218 AEE30000 */  sw      $3, ($23)
/* 0000129C 00021C EAFD1401 */  slv     $v29[8], 0x4($23)
/* 000012A0 000220 EAFD1202 */  slv     $v29[4], 0x8($23)
/* 000012A4 000224 AEE0000C */  sw      $zero, 0xc($23)
/* 000012A8 000228 900503C6 */  lbu     $5, UOBJBG_IMAGEFMT(bg)
/* 000012AC 00022C 900603C7 */  lbu     $6, UOBJBG_IMAGESIZ(bg)
/* 000012B0 000230 900703C9 */  lbu     $7, UOBJBG_IMAGEPAL(bg) + 1
/* 000012B4 000234 8C040264 */  lw      $4, data_0264
/* 000012B8 000238 00051880 */  sll     $3, $5, 2           //  (fmt) << 2
/* 000012BC 00023C 00661825 */  or      $3, $3, $6          // ((fmt) << 2) | (siz)
/* 000012C0 000240 00031CC0 */  sll     $3, $3, 19          // ((fmt) << 21) | ((siz) << 19)
/* 000012C4 000244 00621825 */  or      $3, $3, $2          // ((fmt) << 21) | ((siz) << 19) | ((tmem) << 9)
/* 000012C8 000248 00075D00 */  sll     $11, $7, 20
/* 000012CC 00024C 008B2025 */  or      $4, $4, $11
/* 000012D0 000250 AEE30010 */  sw      $3, 0x10($23)
/* 000012D4 000254 EAFD0610 */  sbv     $v29[12], 0x10($23) // G_SETTILE
/* 000012D8 000258 AEE40014 */  sw      $4, 0x14($23)
/* 000012DC 00025C 22F70018 */  addi    $23, $23, 0x18
/* 000012E0 000260 000630C0 */  sll     $6, $6, 3
/* 000012E4 000264 20C60274 */  addi    $6, $6, 0x274
/* 000012E8 000268 C8C21800 */  ldv     $v2[0], 0x0($6)
/* 000012EC 00026C CA860912 */  lsv     $v6[2], 0x24($20)
/* 000012F0 000270 4B9F2945 */  vmudm   $v5, $v5, $v31[4]
/* 000012F4 000274 4B041228 */  vand    $v8, $v2, $v4[0]
/* 000012F8 000278 4B670A8E */  vmadn   $v10, $v1, $v7[3]
/* 000012FC 00027C 4B041244 */  vmudl   $v9, $v2, $v4[0]
/* 00001300 000280 4B8532CE */  vmadn   $v11, $v6, $v5[4]
/* 00001304 000284 10200002 */  beqz    $1, .L00001310
/* 00001308 000288 4B7D030F */   vmadh  $v12, $vzero, $v29[3]
/* 0000130C 00028C 4A0A0A11 */  vsub    $v8, $v1, $v10
.L00001310:
/* 00001310 000290 4B0A1284 */  vmudl   $v10, $v2, $v10[0]
/* 00001314 000294 4BDF4205 */  vmudm   $v8, $v8, $v31[6]
/* 00001318 000298 4B7DFA0E */  vmadn   $v8, $v31, $v29[3]
/* 0000131C 00029C 4B004A33 */  vmov    $v8[1], $vzero[0]
/* 00001320 0002A0 EBCB0901 */  ssv     $v11[2], 0x2($30)
/* 00001324 0002A4 EBCC0900 */  ssv     $v12[2], 0x0($30)
/* 00001328 0002A8 8FC20000 */  lw      $2, ($30)
/* 0000132C 0002AC 8C1803C0 */  lw      $24, UOBJBG_IMAGEPTR(bg)
/* 00001330 0002B0 0C000455 */  jal     segmented_to_physical
/* 00001334 0002B4 00021042 */   srl    $2, $2, 1
/* 00001338 0002B8 000210C0 */  sll     $2, $2, 3
/* 0000133C 0002BC 00581020 */  add     $2, $2, $24
/* 00001340 0002C0 37030000 */  move    $3, $24
/* 00001344 0002C4 4B671890 */  vadd    $v2, $v3, $v7[3]
/* 00001348 0002C8 4B7F088E */  vmadn   $v2, $v1, $v31[3]
/* 0000134C 0002CC 940503BA */  lhu     $5, UOBJBG_IMAGEH(bg)
/* 00001350 0002D0 48062C00 */  mfc2    $6, $v5[8]
/* 00001354 0002D4 48074900 */  mfc2    $7, $v9[2]
/* 00001358 0002D8 48081A00 */  mfc2    $8, $v3[4]
/* 0000135C 0002DC 48091E00 */  mfc2    $9, $v3[12]
/* 00001360 0002E0 480A1200 */  mfc2    $10, $v2[4]
/* 00001364 0002E4 48013F00 */  mfc2    $1, $v7[14]
/* 00001368 0002E8 00084300 */  sll     $8, $8, 0xc
/* 0000136C 0002EC 30A5FFFC */  andi    $5, $5, 0xfffc
/* 00001370 0002F0 00063080 */  sll     $6, $6, 2
/* 00001374 0002F4 00A62022 */  sub     $4, $5, $6
/* 00001378 0002F8 1880FF8A */  blez    $4, next_cmd
/* 0000137C 0002FC 000A5300 */   sll    $10, $10, 0xc
/* 00001380 000300 18E00002 */  blez    $7, .L0000138C
/* 00001384 000304 3C0CE400 */   lui    $12, 0xe400
/* 00001388 000308 2084FFFC */  addi    $4, $4, -4
.L0000138C:
/* 0000138C 00030C 00815822 */  sub     $11, $4, $1
/* 00001390 000310 19600002 */  blez    $11, .L0000139C
/* 00001394 000314 014C5025 */   or     $10, $10, $12
/* 00001398 000318 34240000 */  move    $4, $1
.L0000139C:
/* 0000139C 00031C 800F03C5 */  lb      $15, UOBJBG_IMAGELOAD(bg) + 1
/* 000013A0 000320 940D03D4 */  lhu     $13, UOBJBG_TMEMSIZEW(bg)
/* 000013A4 000324 940603D2 */  lhu     $6, UOBJBG_TMEMLOADTH(bg)
/* 000013A8 000328 05E10004 */  bgez    $15, .L000013BC
/* 000013AC 00032C 940503D0 */   lhu    $5, UOBJBG_TMEMLOADSH(bg)
/* 000013B0 000330 48055200 */  mfc2    $5, $v10[4]
/* 000013B4 000334 00052880 */  sll     $5, $5, 2
/* 000013B8 000338 20A5FFFF */  addi    $5, $5, -1
.L000013BC:
/* 000013BC 00033C 000D7040 */  sll     $14, $13, 1
/* 000013C0 000340 21CEFFFF */  addi    $14, $14, -1        // (width) - 1
/* 000013C4 000344 3C0B3D10 */  lui     $11, 0x3d10         // G_SETTIMG, G_IM_FMT_RGBA, G_IM_SIZ_16b ?
/* 000013C8 000348 01CB7025 */  or      $14, $14, $11
/* 000013CC 00034C 000F7E00 */  sll     $15, $15, 0x18
/* 000013D0 000350 34A57000 */  ori     $5, $5, 0x7000
/* 000013D4 000354 00052B00 */  sll     $5, $5, 0xc
/* 000013D8 000358 1880002E */  blez    $4, .L00001494
/* 000013DC 00035C 00240822 */   sub    $1, $1, $4
.L000013E0:
/* 000013E0 000360 941D03D6 */  lhu     $29, UOBJBG_TMEMSIZE(bg)
/* 000013E4 000364 05E10004 */  bgez    $15, .L000013F8
/* 000013E8 000368 941503CE */   lhu    $21, UOBJBG_TMEMH(bg)
/* 000013EC 00036C 940B03D0 */  lhu     $11, UOBJBG_TMEMLOADSH(bg)
/* 000013F0 000370 001DEC00 */  sll     $29, $29, 0x10
/* 000013F4 000374 03ABE825 */  or      $29, $29, $11
.L000013F8:
/* 000013F8 000378 00A68025 */  or      $16, $5, $6
.L000013FC:
/* 000013FC 00037C 00952022 */  sub     $4, $4, $21
/* 00001400 000380 04810010 */  bgez    $4, .L00001444
/* 00001404 000384 48841100 */   mtc2   $4, $v2[2]
/* 00001408 000388 4B223086 */  vmudn   $v2, $v6, $v2[1]
/* 0000140C 00038C 4B7D00CF */  vmadh   $v3, $vzero, $v29[3]
/* 00001410 000390 EBC20901 */  ssv     $v2[2], 0x2($30)
/* 00001414 000394 EBC30900 */  ssv     $v3[2], 0x0($30)
/* 00001418 000398 8FCB0000 */  lw      $11, ($30)
/* 0000141C 00039C 02A4A820 */  add     $21, $21, $4
/* 00001420 0003A0 05E10004 */  bgez    $15, .L00001434
/* 00001424 0003A4 03ABE820 */   add    $29, $29, $11
/* 00001428 0003A8 22ABFFFF */  addi    $11, $21, -1
/* 0000142C 0003AC 08000511 */  j       .L00001444
/* 00001430 0003B0 00AB8025 */   or     $16, $5, $11
.L00001434:
/* 00001434 0003B4 23ABFFFE */  addi    $11, $29, -2
/* 00001438 0003B8 356BE000 */  ori     $11, $11, 0xe000
/* 0000143C 0003BC 000B5AC0 */  sll     $11, $11, 0xb
/* 00001440 0003C0 01668025 */  or      $16, $11, $6
.L00001444:
/* 00001444 0003C4 0135E020 */  add     $28, $9, $21
/* 00001448 0003C8 239CFFFF */  addi    $28, $28, -1
/* 0000144C 0003CC EAFD0700 */  sbv     $v29[14], 0x0($23)
/* 00001450 0003D0 AEEE0008 */  sw      $14, 8($23)             // SETTIMG
/* 00001454 0003D4 AEE2000C */  sw      $2, 0xc($23)            // timg ptr
/* 00001458 0003D8 AEEF0010 */  sw      $15, 0x10($23)          // LOADBLOCK or LOADTILE
/* 0000145C 0003DC AEF00014 */  sw      $16, 0x14($23)
/* 00001460 0003E0 EAFD0418 */  sbv     $v29[8], 0x18($23)      // PIPESYNC
/* 00001464 0003E4 015C5825 */  or      $11, $10, $28
/* 00001468 0003E8 AEEB0020 */  sw      $11, 0x20($23)          // 
/* 0000146C 0003EC 01095825 */  or      $11, $8, $9
/* 00001470 0003F0 AEEB0024 */  sw      $11, 0x24($23)
/* 00001474 0003F4 EAE8100A */  slv     $v8[0], 0x28($23)       // 
/* 00001478 0003F8 EAFE140B */  slv     $v30[8], 0x2C($23)
/* 0000147C 0003FC 0C000591 */  jal     func_00001644
/* 00001480 000400 22F70030 */   addi   $23, $23, 0x30
/* 00001484 000404 23890001 */  addi    $9, $28, 1
/* 00001488 000408 1C80FFDC */  bgtz    $4, .L000013FC
/* 0000148C 00040C 005D1020 */   add    $2, $2, $29
/* 00001490 000410 1820FF44 */  blez    $1, next_cmd
.L00001494:
/* 00001494 000414 EAFD0700 */   sbv    $v29[14], 0x0($23)
/* 00001498 000418 18E00028 */  blez    $7, .L0000153C
/* 0000149C 00041C AEEE0008 */   sw     $14, 8($23)
/* 000014A0 000420 AEE2000C */  sw      $2, 0xc($23)
/* 000014A4 000424 3C0B3510 */  lui     $11, 0x3510
/* 000014A8 000428 AEEB0010 */  sw      $11, 0x10($23)
/* 000014AC 00042C 3C0C0600 */  lui     $12, 0x600
/* 000014B0 000430 AEEC0014 */  sw      $12, 0x14($23)
/* 000014B4 000434 3C1D3300 */  lui     $29, 0x3300
/* 000014B8 000438 AEFD0018 */  sw      $29, 0x18($23)
/* 000014BC 00043C 00073842 */  srl     $7, $7, 1
/* 000014C0 000440 000DA842 */  srl     $21, $13, 1
/* 000014C4 000444 02A7A822 */  sub     $21, $21, $7
/* 000014C8 000448 00155B80 */  sll     $11, $21, 0xe
/* 000014CC 00044C 216BF000 */  addi    $11, $11, -0x1000
/* 000014D0 000450 016C5825 */  or      $11, $11, $12
/* 000014D4 000454 AEEB001C */  sw      $11, 0x1c($23)
/* 000014D8 000458 EAFD0420 */  sbv     $v29[8], 0x20($23)
/* 000014DC 00045C AEEE0028 */  sw      $14, 0x28($23)
/* 000014E0 000460 AEE3002C */  sw      $3, 0x2c($23)
/* 000014E4 000464 3C0B3510 */  lui     $11, 0x3510
/* 000014E8 000468 AEEB0030 */  sw      $11, 0x30($23)
/* 000014EC 00046C A6F50032 */  sh      $21, 0x32($23)
/* 000014F0 000470 3C0C0600 */  lui     $12, 0x600
/* 000014F4 000474 AEEC0034 */  sw      $12, 0x34($23)
/* 000014F8 000478 AEFD0038 */  sw      $29, 0x38($23)
/* 000014FC 00047C 00075B80 */  sll     $11, $7, 0xe
/* 00001500 000480 216BF000 */  addi    $11, $11, -0x1000
/* 00001504 000484 016C5825 */  or      $11, $11, $12
/* 00001508 000488 AEEB003C */  sw      $11, 0x3c($23)
/* 0000150C 00048C 22F70058 */  addi    $23, $23, 0x58
/* 00001510 000490 EAFD0468 */  sbv     $v29[8], -0x18($23)
/* 00001514 000494 01495825 */  or      $11, $10, $9
/* 00001518 000498 AEEBFFF0 */  sw      $11, -0x10($23)
/* 0000151C 00049C 01095825 */  or      $11, $8, $9
/* 00001520 0004A0 AEEBFFF4 */  sw      $11, -0xc($23)
/* 00001524 0004A4 EAE8107E */  slv     $v8[0], -0x8($23)
/* 00001528 0004A8 0C000591 */  jal     func_00001644
/* 0000152C 0004AC EAFE147F */   slv    $v30[8], -0x4($23)
/* 00001530 0004B0 21290004 */  addi    $9, $9, 4
/* 00001534 0004B4 2021FFFC */  addi    $1, $1, -4
/* 00001538 0004B8 1820FF1A */  blez    $1, next_cmd
.L0000153C:
/* 0000153C 0004BC 000758C0 */   sll    $11, $7, 3
/* 00001540 0004C0 006B1020 */  add     $2, $3, $11
/* 00001544 0004C4 34240000 */  move    $4, $1
/* 00001548 0004C8 080004F8 */  j       .L000013E0
/* 0000154C 0004CC 20010000 */   li     $1, 0

G_OBJ_LOADTXTR_handler:
G_OBJ_LDTX_SPRITE_handler:
G_OBJ_LDTX_RECT_handler:
G_OBJ_LDTX_RECT_R_handler:
/* 00001550 0004D0 0C000455 */  jal     segmented_to_physical
/* 00001554 0004D4 201403A8 */   li     $20, data_03A8
/* 00001558 0004D8 0C0007F6 */  jal     dma_read_write
/* 0000155C 0004DC 333300FF */   andi   $19, cmd_w0, 0xff
/* 00001560 0004E0 940B0244 */  lhu     $11, data_0244
/* 00001564 0004E4 CBC41807 */  ldv     $v4[0], 0x38($30)
/* 00001568 0004E8 800102AE */  lb      $1, data_02AE
/* 0000156C 0004EC C9632000 */  lqv     $v3[0], 0x0($11)
/* 00001570 0004F0 0019CDC2 */  srl     $25, cmd_w0, 0x17
/* 00001574 0004F4 0C0007F2 */  jal     dma_wait
/* 00001578 0004F8 87390202 */   lh     $25, (data_020C - 2 * G_OBJ_LOADTXTR)($25)
/* 0000157C 0004FC 9283000F */  lbu     $3, 0xf($20)
/* 00001580 000500 8E870014 */  lw      $7, 0x14($20)
/* 00001584 000504 8E850010 */  lw      $5, 0x10($20)
/* 00001588 000508 8C6402B0 */  lw      $4, 0x2b0($3)
/* 0000158C 00050C 00E04027 */  not     $8, $7
/* 00001590 000510 00A73024 */  and     $6, $5, $7
/* 00001594 000514 00874824 */  and     $9, $4, $7
/* 00001598 000518 11250027 */  beq     $9, $5, .L00001638
/* 0000159C 00051C 00884824 */   and    $9, $4, $8
/* 000015A0 000520 01264825 */  or      $9, $9, $6
/* 000015A4 000524 AC6902B0 */  sw      $9, 0x2b0($3)
/* 000015A8 000528 200BFF81 */  li      $11, -0x7f
/* 000015AC 00052C 15610003 */  bne     $11, $1, .L000015BC
/* 000015B0 000530 A00B02AE */   sb     $11, data_02AE
/* 000015B4 000534 EAFD0780 */  sbv     $v29[15], 0x0($23)
/* 000015B8 000538 22F70008 */  addi    $23, $23, 8
.L000015BC:
/* 000015BC 00053C 9684000A */  lhu     $4, 0xa($20)
/* 000015C0 000540 EAFD0800 */  ssv     $v29[0], 0x0($23)
/* 000015C4 000544 8E980004 */  lw      $24, 4($20)
/* 000015C8 000548 A6E40002 */  sh      $4, 2($23)
/* 000015CC 00054C 0C000455 */  jal     segmented_to_physical
/* 000015D0 000550 96830008 */   lhu    $3, 8($20)
/* 000015D4 000554 AEF80004 */  sw      $24, 4($23)
/* 000015D8 000558 82880001 */  lb      $8, 1($20)
/* 000015DC 00055C 92870002 */  lbu     $7, 2($20)
/* 000015E0 000560 92850003 */  lbu     $5, 3($20)
/* 000015E4 000564 20860001 */  addi    $6, $4, 1
/* 000015E8 000568 00C83024 */  and     $6, $6, $8
/* 000015EC 00056C 000631C0 */  sll     $6, $6, 7
/* 000015F0 000570 00073C00 */  sll     $7, $7, 0x10
/* 000015F4 000574 00661825 */  or      $3, $3, $6
/* 000015F8 000578 00671825 */  or      $3, $3, $7
/* 000015FC 00057C AEE30008 */  sw      $3, 8($23)
/* 00001600 000580 EAFD0608 */  sbv     $v29[12], 0x8($23)
/* 00001604 000584 04200003 */  bltz    $1, .L00001614
/* 00001608 000588 EAFD1403 */   slv    $v29[8], 0xC($23)
/* 0000160C 00058C EAFD0710 */  sbv     $v29[14], 0x10($23)
/* 00001610 000590 22F70008 */  addi    $23, $23, 8
.L00001614:
/* 00001614 000594 9681000C */  lhu     $1, 0xc($20)
/* 00001618 000598 00052E00 */  sll     $5, $5, 0x18
/* 0000161C 00059C AEE50010 */  sw      $5, 0x10($23)
/* 00001620 0005A0 00042380 */  sll     $4, $4, 0xe
/* 00001624 0005A4 00812025 */  or      $4, $4, $1
/* 00001628 0005A8 AEE40014 */  sw      $4, 0x14($23)
/* 0000162C 0005AC EAFD0414 */  sbv     $v29[8], 0x14($23)
/* 00001630 0005B0 0C000591 */  jal     func_00001644
/* 00001634 0005B4 22F70018 */   addi   $23, $23, 0x18
.L00001638:
/* 00001638 0005B8 03200008 */  jr      $25
/* 0000163C 0005BC 201403C0 */   li     $20, data_03C0

func_00001640:
/* 00001640 0005C0 201F11A4 */  li      $ra, next_cmd
func_00001644:
/* 00001644 0005C4 200B00E7 */  li      $11, G_RDPPIPESYNC
/* 00001648 0005C8 A2EB0000 */  sb      $11, ($23)
/* 0000164C 0005CC 22F70008 */  addi    $23, $23, 8
/* 00001650 0005D0 02F65822 */  sub     $11, $23, $22
/* 00001654 0005D4 1960025F */  blez    $11, return_routine
.L00001658:
/* 00001658 0005D8 400C3000 */   mfc0   $12, SP_DMA_BUSY
/* 0000165C 0005DC 8C1800F0 */  lw      $24, data_00F0
/* 00001660 0005E0 25730318 */  addiu   $19, $11, 0x318
/* 00001664 0005E4 1580FFFC */  bnez    $12, .L00001658
/* 00001668 0005E8 8C0C0FEC */   lw     $12, OSTask_addr + OS_TASK_OFF_OUTBUFF_SZ
/* 0000166C 0005EC 40984800 */  mtc0    $24, DPC_END
/* 00001670 0005F0 03135820 */  add     $11, $24, $19
/* 00001674 0005F4 018B6022 */  sub     $12, $12, $11
/* 00001678 0005F8 05810008 */  bgez    $12, .L0000169C
.L0000167C:
/* 0000167C 0005FC 400B5800 */   mfc0   $11, DPC_STATUS
/* 00001680 000600 316B0400 */  andi    $11, $11, 0x400
/* 00001684 000604 1560FFFD */  bnez    $11, .L0000167C
/* 00001688 000608 8C180FE8 */   lw     $24, OSTask_addr + OS_TASK_OFF_OUTBUFF
.L0000168C:
/* 0000168C 00060C 400B5000 */  mfc0    $11, DPC_CURRENT
/* 00001690 000610 1178FFFE */  beq     $11, $24, .L0000168C
/* 00001694 000614 00000000 */   nop    
/* 00001698 000618 40984000 */  mtc0    $24, DPC_START
.L0000169C:
/* 0000169C 00061C 400B5000 */  mfc0    $11, DPC_CURRENT
/* 000016A0 000620 01785822 */  sub     $11, $11, $24
/* 000016A4 000624 19600002 */  blez    $11, .L000016B0
/* 000016A8 000628 01735822 */   sub    $11, $11, $19
/* 000016AC 00062C 1960FFFB */  blez    $11, .L0000169C
.L000016B0:
/* 000016B0 000630 03135820 */   add    $11, $24, $19
/* 000016B4 000634 AC0B00F0 */  sw      $11, data_00F0
/* 000016B8 000638 2273FFFF */  addi    $19, $19, -1
/* 000016BC 00063C 22D4DCE8 */  addi    $20, $22, -0x2318
/* 000016C0 000640 3AD60C00 */  xori    $22, $22, 0xc00
/* 000016C4 000644 080007F6 */  j       dma_read_write
/* 000016C8 000648 22D7FCE8 */   addi   $23, $22, -0x318

.align 8

Overlay23LoadAddress:

.headersize Overlay23LoadAddress - orga()
Overlay2Address:

G_OBJ_SPRITE_handler:
/* 000016D0 000650 0C000455 */  jal     segmented_to_physical
/* 000016D4 000654 20130017 */   li     $19, 0x18-1
/* 000016D8 000658 0C0007F6 */  jal     dma_read_write
/* 000016DC 00065C 201403C0 */   li     $20, data_03C0
/* 000016E0 000660 940B0244 */  lhu     $11, data_0244
/* 000016E4 000664 CBC41807 */  ldv     $v4[0], 0x38($30)
/* 000016E8 000668 0C0007F2 */  jal     dma_wait
/* 000016EC 00066C C9631800 */   ldv    $v3[0], 0x0($11)
/* 000016F0 000670 CA822000 */  lqv     $v2[0], 0x0($20)
/* 000016F4 000674 900201E8 */  lbu     $2, data_01E8
/* 000016F8 000678 800301EC */  lb      $3, data_01EC
/* 000016FC 00067C 00431026 */  xor     $2, $2, $3
/* 00001700 000680 30420180 */  andi    $2, $2, 0x180
/* 00001704 000684 000210C2 */  srl     $2, $2, 3
/* 00001708 000688 204102C0 */  addi    $1, $2, 0x2c0
/* 0000170C 00068C 4B2411D1 */  vsub    $v7, $v2, $v4[1]
/* 00001710 000690 4B432190 */  vadd    $v6, $v4, $v3[2]
/* 00001714 000694 4B641150 */  vadd    $v5, $v2, $v4[3]
/* 00001718 000698 4B224230 */  vrcp    $v8[0], $v2[1]
/* 0000171C 00069C 4B1B4132 */  vrcph   $v4[0], $v27[0]
/* 00001720 0006A0 4BA26230 */  vrcp    $v8[4], $v2[5]
/* 00001724 0006A4 4B1B6132 */  vrcph   $v4[4], $v27[0]
/* 00001728 0006A8 4A044214 */  vaddc   $v8, $v8, $v4
/* 0000172C 0006AC 4B7D2110 */  vadd    $v4, $v4, $v29[3]
/* 00001730 0006B0 4A883EC4 */  vmudl   $v27, $v7, $v8[0h]
/* 00001734 0006B4 4A84388D */  vmadm   $v2, $v7, $v4[0h]
/* 00001738 0006B8 4B7DDA4E */  vmadn   $v9, $v27, $v29[3]
/* 0000173C 0006BC 4B1E16C5 */  vmudm   $v27, $v2, $v30[0]
/* 00001740 0006C0 4B1E488C */  vmadl   $v2, $v9, $v30[0]
/* 00001744 0006C4 C8292000 */  lqv     $v9[0], 0x0($1)
/* 00001748 0006C8 928B0017 */  lbu     $11, 0x17($20)
/* 0000174C 0006CC 48CB0800 */  ctc2    $11, $vcc
/* 00001750 0006D0 4A000AA7 */  vmrg    $v10, $v1, $vzero
/* 00001754 0006D4 4A8A4A2C */  vxor    $v8, $v9, $v10[0h]
/* 00001758 0006D8 4B060EC7 */  vmudh   $v27, $v1, $v6[0]
/* 0000175C 0006DC 4AC7410F */  vmadh   $v4, $v8, $v7[2h]
/* 00001760 0006E0 4A850EC7 */  vmudh   $v27, $v1, $v5[0h]
/* 00001764 0006E4 4AC2498F */  vmadh   $v6, $v9, $v2[2h]
/* 00001768 0006E8 4B7D01CF */  vmadh   $v7, $vzero, $v29[3]
/* 0000176C 0006EC EBC62000 */  sqv     $v6[0], 0x0($30)
/* 00001770 0006F0 CBC61C00 */  ldv     $v6[8], 0x0($30)
/* 00001774 0006F4 CBC71801 */  ldv     $v7[0], 0x8($30)
/* 00001778 0006F8 CBC22002 */  lqv     $v2[0], 0x20($30)
/* 0000177C 0006FC CBC50818 */  lsv     $v5[0], 0x30($30)
/* 00001780 000700 CBC50C19 */  lsv     $v5[8], 0x32($30)
/* 00001784 000704 4B632950 */  vadd    $v5, $v5, $v3[3]
/* 00001788 000708 4B032968 */  vand    $v5, $v5, $v3[0]
/* 0000178C 00070C 4A850EC7 */  vmudh   $v27, $v1, $v5[0h]
/* 00001790 000710 4AA236CD */  vmadm   $v27, $v6, $v2[1h]
/* 00001794 000714 4A8236CF */  vmadh   $v27, $v6, $v2[0h]
/* 00001798 000718 4AE23ECD */  vmadm   $v27, $v7, $v2[3h]
/* 0000179C 00071C 4AC23A0F */  vmadh   $v8, $v7, $v2[2h]
/* 000017A0 000720 CBC21011 */  llv     $v2[0], 0x44($30)
/* 000017A4 000724 CBC21412 */  llv     $v2[8], 0x48($30)
/* 000017A8 000728 4A8246E3 */  vge     $v27, $v8, $v2[0h]
/* 000017AC 00072C 484B0800 */  cfc2    $11, $vcc
/* 000017B0 000730 316C000F */  andi    $12, $11, 0xf
/* 000017B4 000734 1180FE7B */  beqz    $12, next_cmd
/* 000017B8 000738 316C00F0 */   andi   $12, $11, 0xf0
/* 000017BC 00073C 1180FE79 */  beqz    $12, next_cmd
/* 000017C0 000740 4AA246E0 */   vlt    $v27, $v8, $v2[1h]
/* 000017C4 000744 484B0800 */  cfc2    $11, $vcc
/* 000017C8 000748 316C000F */  andi    $12, $11, 0xf
/* 000017CC 00074C 1180FE75 */  beqz    $12, next_cmd
/* 000017D0 000750 316C00F0 */   andi   $12, $11, 0xf0
/* 000017D4 000754 1180FE73 */  beqz    $12, next_cmd
/* 000017D8 000758 4B7D4090 */   vadd   $v2, $v8, $v29[3]
/* 000017DC 00075C EBC81C00 */  sdv     $v8[8], 0x0($30)
/* 000017E0 000760 4A48416C */  vxor    $v5, $v8, $v8[0q]
/* 000017E4 000764 CBC21800 */  ldv     $v2[0], 0x0($30)
/* 000017E8 000768 4A4421AC */  vxor    $v6, $v4, $v4[0q]
/* 000017EC 00076C 4A6210A0 */  vlt     $v2, $v2, $v2[1q]
/* 000017F0 000770 4A652967 */  vmrg    $v5, $v5, $v5[1q]
/* 000017F4 000774 4A6631A7 */  vmrg    $v6, $v6, $v6[1q]
/* 000017F8 000778 4A45422C */  vxor    $v8, $v8, $v5[0q]
/* 000017FC 00077C 4A46212C */  vxor    $v4, $v4, $v6[0q]
/* 00001800 000780 900102AF */  lbu     $1, data_02AF
/* 00001804 000784 38210002 */  xori    $1, $1, 2
/* 00001808 000788 A00102AF */  sb      $1, data_02AF
/* 0000180C 00078C 800C02AE */  lb      $12, data_02AE
/* 00001810 000790 EAFD0780 */  sbv     $v29[15], 0x0($23)
/* 00001814 000794 142C0002 */  bne     $1, $12, .L00001820
/* 00001818 000798 A00102AE */   sb     $1, data_02AE
/* 0000181C 00079C 26F70008 */  addiu   $23, $23, 8
.L00001820:
/* 00001820 0007A0 94070246 */  lhu     $7, data_0246
/* 00001824 0007A4 92820014 */  lbu     $2, 0x14($20)
/* 00001828 0007A8 92830015 */  lbu     $3, 0x15($20)
/* 0000182C 0007AC 96840010 */  lhu     $4, 0x10($20)
/* 00001830 0007B0 96850012 */  lhu     $5, 0x12($20)
/* 00001834 0007B4 92860016 */  lbu     $6, 0x16($20)
/* 00001838 0007B8 8CE70004 */  lw      $7, 4($7)
/* 0000183C 0007BC 00021540 */  sll     $2, $2, 0x15
/* 00001840 0007C0 00031CC0 */  sll     $3, $3, 0x13
/* 00001844 0007C4 00431025 */  or      $2, $2, $3
/* 00001848 0007C8 00042240 */  sll     $4, $4, 9
/* 0000184C 0007CC 00441025 */  or      $2, $2, $4
/* 00001850 0007D0 00451025 */  or      $2, $2, $5
/* 00001854 0007D4 AEE20000 */  sw      $2, ($23)
/* 00001858 0007D8 EAFD0600 */  sbv     $v29[12], 0x0($23)
/* 0000185C 0007DC 00063500 */  sll     $6, $6, 0x14
/* 00001860 0007E0 00C73025 */  or      $6, $6, $7
/* 00001864 0007E4 AEE60004 */  sw      $6, 4($23)
/* 00001868 0007E8 A2E10004 */  sb      $1, 4($23)
/* 0000186C 0007EC 96840004 */  lhu     $4, 4($20)
/* 00001870 0007F0 EAFD1202 */  slv     $v29[4], 0x8($23)
/* 00001874 0007F4 9685000C */  lhu     $5, 0xc($20)
/* 00001878 0007F8 30847FF8 */  andi    $4, $4, 0x7ff8
/* 0000187C 0007FC 00042240 */  sll     $4, $4, 9
/* 00001880 000800 000528C3 */  sra     $5, $5, 3
/* 00001884 000804 00851825 */  or      $3, $4, $5
/* 00001888 000808 2063BFFC */  addi    $3, $3, -0x4004
/* 0000188C 00080C AEE3000C */  sw      $3, 0xc($23)
/* 00001890 000810 A2E1000C */  sb      $1, 0xc($23)
/* 00001894 000814 EAFD0410 */  sbv     $v29[8], 0x10($23)
/* 00001898 000818 26F70018 */  addiu   $23, $23, 0x18
/* 0000189C 00081C 4B3E01EB */  vnor    $v7, $vzero, $v30[1]
/* 000018A0 000820 200B0020 */  li      $11, 0x20
/* 000018A4 000824 48CB0800 */  ctc2    $11, $vcc
/* 000018A8 000828 200C00E7 */  li      $12, G_RDPPIPESYNC
/* 000018AC 00082C 4AC838A7 */  vmrg    $v2, $v7, $v8[2h]
/* 000018B0 000830 4A8838E7 */  vmrg    $v3, $v7, $v8[0h]
/* 000018B4 000834 4AA83967 */  vmrg    $v5, $v7, $v8[1h]
/* 000018B8 000838 4AE839A7 */  vmrg    $v6, $v7, $v8[3h]
/* 000018BC 00083C 48CC0800 */  ctc2    $12, $vcc
/* 000018C0 000840 4AC410A7 */  vmrg    $v2, $v2, $v4[2h]
/* 000018C4 000844 4A8418E7 */  vmrg    $v3, $v3, $v4[0h]
/* 000018C8 000848 4AA42967 */  vmrg    $v5, $v5, $v4[1h]
/* 000018CC 00084C 4AE431A7 */  vmrg    $v6, $v6, $v4[3h]
.L000018D0:
/* 000018D0 000850 4A022911 */  vsub    $v4, $v5, $v2
/* 000018D4 000854 4A0219D1 */  vsub    $v7, $v3, $v2
/* 000018D8 000858 4A031211 */  vsub    $v8, $v2, $v3
/* 000018DC 00085C 4A032A51 */  vsub    $v9, $v5, $v3
/* 000018E0 000860 4BC726C7 */  vmudh   $v27, $v4, $v7[6]
/* 000018E4 000864 4BC446CF */  vmadh   $v27, $v8, $v4[6]
/* 000018E8 000868 4B0A529D */  vreadacc $v10, ACC_LOWER
/* 000018EC 00086C 4B2B5ADD */  vreadacc $v11, ACC_MIDDLE
/* 000018F0 000870 4BC44370 */  vrcp    $v13[0], $v4[6]
/* 000018F4 000874 4B7D4332 */  vrcph   $v12[0], $v29[3]
/* 000018F8 000878 4A0A56D3 */  vabs    $v27, $v10, $v10
/* 000018FC 00087C 4BC74B70 */  vrcp    $v13[1], $v7[6]
/* 00001900 000880 4B7D4B32 */  vrcph   $v12[1], $v29[3]
/* 00001904 000884 4BDFDEE0 */  vlt     $v27, $v27, $v31[6]
/* 00001908 000888 4B1EF3A7 */  vmrg    $v14, $v30, $v30[0]
/* 0000190C 00088C 4BC95370 */  vrcp    $v13[2], $v9[6]
/* 00001910 000890 4B7D5332 */  vrcph   $v12[2], $v29[3]
/* 00001914 000894 4B2E5EC4 */  vmudl   $v27, $v11, $v14[1]
/* 00001918 000898 4B2E528D */  vmadm   $v10, $v10, $v14[1]
/* 0000191C 00089C 4B7DFACE */  vmadn   $v11, $v31, $v29[3]
/* 00001920 0008A0 4B444273 */  vmov    $v9[0], $v4[2]
/* 00001924 0008A4 4B474A73 */  vmov    $v9[1], $v7[2]
/* 00001928 0008A8 4A0C6B54 */  vaddc   $v13, $v13, $v12
/* 0000192C 0008AC 4B7D6310 */  vadd    $v12, $v12, $v29[3]
/* 00001930 0008B0 4BC44473 */  vmov    $v17[0], $v4[6]
/* 00001934 0008B4 4BC74C73 */  vmov    $v17[1], $v7[6]
/* 00001938 0008B8 4BC95473 */  vmov    $v17[2], $v9[6]
/* 0000193C 0008BC 4B5F0490 */  vadd    $v18, $vzero, $v31[2]
/* 00001940 0008C0 4B5F6B46 */  vmudn   $v13, $v13, $v31[2]
/* 00001944 0008C4 4B5F630F */  vmadh   $v12, $v12, $v31[2]
/* 00001948 0008C8 4A116EC4 */  vmudl   $v27, $v13, $v17
/* 0000194C 0008CC 4A11640D */  vmadm   $v16, $v12, $v17
/* 00001950 0008D0 4B7DFBCE */  vmadn   $v15, $v31, $v29[3]
/* 00001954 0008D4 4A0F03D5 */  vsubc   $v15, $vzero, $v15
/* 00001958 0008D8 4A109411 */  vsub    $v16, $v18, $v16
/* 0000195C 0008DC 4A0F6EC4 */  vmudl   $v27, $v13, $v15
/* 00001960 0008E0 4A0F66CD */  vmadm   $v27, $v12, $v15
/* 00001964 0008E4 4A106B4E */  vmadn   $v13, $v13, $v16
/* 00001968 0008E8 4A10630F */  vmadh   $v12, $v12, $v16
/* 0000196C 0008EC 48025180 */  mfc2    $2, $v10[3]
/* 00001970 0008F0 30420080 */  andi    $2, $2, 0x80
/* 00001974 0008F4 00411025 */  or      $2, $2, $1
/* 00001978 0008F8 34420A00 */  ori     $2, $2, 0xa00
/* 0000197C 0008FC 4B4A46F2 */  vrcph   $v27[0], $v10[2]
/* 00001980 000900 4B4B5431 */  vrcpl   $v16[2], $v11[2]
/* 00001984 000904 4B7D53F2 */  vrcph   $v15[2], $v29[3]
/* 00001988 000908 4A096EC6 */  vmudn   $v27, $v13, $v9
/* 0000198C 00090C 4A0966CF */  vmadh   $v27, $v12, $v9
/* 00001990 000910 4B2A529D */  vreadacc $v10, ACC_MIDDLE
/* 00001994 000914 4B0B5ADD */  vreadacc $v11, ACC_LOWER
/* 00001998 000918 4BC726C7 */  vmudh   $v27, $v4, $v7[6]
/* 0000199C 00091C 4BC446CF */  vmadh   $v27, $v8, $v4[6]
/* 000019A0 000920 4B2C631D */  vreadacc $v12, ACC_MIDDLE
/* 000019A4 000924 4B0D6B5D */  vreadacc $v13, ACC_LOWER
/* 000019A8 000928 4B443EC7 */  vmudh   $v27, $v7, $v4[2]
/* 000019AC 00092C 4B4826CF */  vmadh   $v27, $v4, $v8[2]
/* 000019B0 000930 4B318C5D */  vreadacc $v17, ACC_MIDDLE
/* 000019B4 000934 4B12949D */  vreadacc $v18, ACC_LOWER
/* 000019B8 000938 4B5066C4 */  vmudl   $v27, $v12, $v16[2]
/* 000019BC 00093C 4B506ECD */  vmadm   $v27, $v13, $v16[2]
/* 000019C0 000940 4B4F630E */  vmadn   $v12, $v12, $v15[2]
/* 000019C4 000944 4B4F6B4F */  vmadh   $v13, $v13, $v15[2]
/* 000019C8 000948 4B508EC4 */  vmudl   $v27, $v17, $v16[2]
/* 000019CC 00094C 4B5096CD */  vmadm   $v27, $v18, $v16[2]
/* 000019D0 000950 4B4F8C4E */  vmadn   $v17, $v17, $v15[2]
/* 000019D4 000954 4B4F948F */  vmadh   $v18, $v18, $v15[2]
/* 000019D8 000958 48031A00 */  mfc2    $3, $v3[4]
/* 000019DC 00095C 4B9F03D0 */  vadd    $v15, $vzero, $v31[4]
/* 000019E0 000960 4B9F56C4 */  vmudl   $v27, $v10, $v31[4]
/* 000019E4 000964 4B9F5A4D */  vmadm   $v9, $v11, $v31[4]
/* 000019E8 000968 00031B80 */  sll     $3, $3, 0xe
/* 000019EC 00096C 4B7D020E */  vmadn   $v8, $vzero, $v29[3]
/* 000019F0 000970 4BBF1128 */  vand    $v4, $v2, $v31[5]
/* 000019F4 000974 4A040111 */  vsub    $v4, $vzero, $v4
/* 000019F8 000978 4B7D5EE0 */  vlt     $v27, $v11, $v29[3]
/* 000019FC 00097C 4B3D0427 */  vmrg    $v16, $vzero, $v29[1]
/* 00001A00 000980 4B427EC6 */  vmudn   $v27, $v15, $v2[2]
/* 00001A04 000984 4BC4420E */  vmadn   $v8, $v8, $v4[6]
/* 00001A08 000988 4BC44A4F */  vmadh   $v9, $v9, $v4[6]
/* 00001A0C 00098C 4BC47906 */  vmudn   $v4, $v15, $v4[6]
/* 00001A10 000990 4B7D01CF */  vmadh   $v7, $vzero, $v29[3]
/* 00001A14 000994 4B1044D4 */  vaddc   $v19, $v8, $v16[0]
/* 00001A18 000998 4B7D4D10 */  vadd    $v20, $v9, $v29[3]
/* 00001A1C 00099C 4B2E66C4 */  vmudl   $v27, $v12, $v14[1]
/* 00001A20 0009A0 4B2E6B4D */  vmadm   $v13, $v13, $v14[1]
/* 00001A24 0009A4 4B7DFB0E */  vmadn   $v12, $v31, $v29[3]
/* 00001A28 0009A8 4B2E8EC4 */  vmudl   $v27, $v17, $v14[1]
/* 00001A2C 0009AC 4B2E948D */  vmadm   $v18, $v18, $v14[1]
/* 00001A30 0009B0 4B7DFC4E */  vmadn   $v17, $v31, $v29[3]
/* 00001A34 0009B4 4BDF6306 */  vmudn   $v12, $v12, $v31[6]
/* 00001A38 0009B8 4BDF6B4F */  vmadh   $v13, $v13, $v31[6]
/* 00001A3C 0009BC 4BDF8C46 */  vmudn   $v17, $v17, $v31[6]
/* 00001A40 0009C0 4BDF948F */  vmadh   $v18, $v18, $v31[6]
/* 00001A44 0009C4 4B0A66C4 */  vmudl   $v27, $v12, $v10[0]
/* 00001A48 0009C8 4B0A6ECD */  vmadm   $v27, $v13, $v10[0]
/* 00001A4C 0009CC 4B0B66CE */  vmadn   $v27, $v12, $v11[0]
/* 00001A50 0009D0 4B0B6ECF */  vmadh   $v27, $v13, $v11[0]
/* 00001A54 0009D4 4B3D8B8E */  vmadn   $v14, $v17, $v29[1]
/* 00001A58 0009D8 4B3D93CF */  vmadh   $v15, $v18, $v29[1]
/* 00001A5C 0009DC 4B3D16C7 */  vmudh   $v27, $v2, $v29[1]
/* 00001A60 0009E0 4BC476CC */  vmadl   $v27, $v14, $v4[6]
/* 00001A64 0009E4 4BC47ECD */  vmadm   $v27, $v15, $v4[6]
/* 00001A68 0009E8 4BC7740E */  vmadn   $v16, $v14, $v7[6]
/* 00001A6C 0009EC 4BC77D4F */  vmadh   $v21, $v15, $v7[6]
/* 00001A70 0009F0 A6E20000 */  sh      $2, ($23)
/* 00001A74 0009F4 EAE50E01 */  ssv     $v5[12], 0x2($23)
/* 00001A78 0009F8 EAE30E02 */  ssv     $v3[12], 0x4($23)
/* 00001A7C 0009FC EAE20E03 */  ssv     $v2[12], 0x6($23)
/* 00001A80 000A00 AEE30008 */  sw      $3, 8($23)
/* 00001A84 000A04 EAEB0A06 */  ssv     $v11[4], 0xC($23)
/* 00001A88 000A08 EAEA0A07 */  ssv     $v10[4], 0xE($23)
/* 00001A8C 000A0C EAE90808 */  ssv     $v9[0], 0x10($23)
/* 00001A90 000A10 EAE80809 */  ssv     $v8[0], 0x12($23)
/* 00001A94 000A14 EAEB080A */  ssv     $v11[0], 0x14($23)
/* 00001A98 000A18 EAEA080B */  ssv     $v10[0], 0x16($23)
/* 00001A9C 000A1C EAF4090C */  ssv     $v20[2], 0x18($23)
/* 00001AA0 000A20 EAF3090D */  ssv     $v19[2], 0x1A($23)
/* 00001AA4 000A24 EAEB090E */  ssv     $v11[2], 0x1C($23)
/* 00001AA8 000A28 EAEA090F */  ssv     $v10[2], 0x1E($23)
/* 00001AAC 000A2C EAF51B04 */  sdv     $v21[6], 0x20($23)
/* 00001AB0 000A30 EAED1B05 */  sdv     $v13[6], 0x28($23)
/* 00001AB4 000A34 EAF01B06 */  sdv     $v16[6], 0x30($23)
/* 00001AB8 000A38 EAEC1B07 */  sdv     $v12[6], 0x38($23)
/* 00001ABC 000A3C EAEF1B08 */  sdv     $v15[6], 0x40($23)
/* 00001AC0 000A40 EAF21B09 */  sdv     $v18[6], 0x48($23)
/* 00001AC4 000A44 EAEE1B0A */  sdv     $v14[6], 0x50($23)
/* 00001AC8 000A48 EAF11B0B */  sdv     $v17[6], 0x58($23)
/* 00001ACC 000A4C 1320FEDC */  beqz    $25, func_00001640
/* 00001AD0 000A50 26F70060 */   addiu  $23, $23, 0x60
/* 00001AD4 000A54 4B7D1890 */  vadd    $v2, $v3, $v29[3]
/* 00001AD8 000A58 20190000 */  li      $25, 0
/* 00001ADC 000A5C 4B7D28D0 */  vadd    $v3, $v5, $v29[3]
/* 00001AE0 000A60 08000634 */  j       .L000018D0
/* 00001AE4 000A64 4B7D3150 */   vadd   $v5, $v6, $v29[3]

.orga 0xB04

G_BG_1CYC_handler:
/* 00001B84 000B04 200C1B80 */  li      $12, func_00001B80
/* 00001B88 000B08 080007ED */  j       load_overlay_and_enter
/* 00001B8C 000B0C 200B031C */   li     $11, Overlay3Info

G_OBJ_RECTANGLE_handler:
/* 00001B90 000B10 0C000455 */  jal     segmented_to_physical
/* 00001B94 000B14 20130017 */   li     $19, 0x18-1
/* 00001B98 000B18 0C0007F6 */  jal     dma_read_write
/* 00001B9C 000B1C 201403C0 */   li     $20, data_03C0
/* 00001BA0 000B20 940B0244 */  lhu     $11, data_0244
/* 00001BA4 000B24 CBC41807 */  ldv     $v4[0], 0x38($30)
/* 00001BA8 000B28 0C0007F2 */  jal     dma_wait
/* 00001BAC 000B2C C9631800 */   ldv    $v3[0], 0x0($11)
/* 00001BB0 000B30 07200077 */  bltz    $25, .L00001D90
/* 00001BB4 000B34 CA822000 */   lqv    $v2[0], 0x0($20)
/* 00001BB8 000B38 4B441150 */  vadd    $v5, $v2, $v4[2]
/* 00001BBC 000B3C 4B032968 */  vand    $v5, $v5, $v3[0]
/* 00001BC0 000B40 4B7D01A8 */  vand    $v6, $vzero, $v29[3]
.L00001BC4:
/* 00001BC4 000B44 4B241211 */  vsub    $v8, $v2, $v4[1]
/* 00001BC8 000B48 4B0401D0 */  vadd    $v7, $vzero, $v4[0]
/* 00001BCC 000B4C 4B224270 */  vrcp    $v9[0], $v2[1]
/* 00001BD0 000B50 4B1B4132 */  vrcph   $v4[0], $v27[0]
/* 00001BD4 000B54 4BA26270 */  vrcp    $v9[4], $v2[5]
/* 00001BD8 000B58 4B1B6132 */  vrcph   $v4[4], $v27[0]
/* 00001BDC 000B5C 4A044A54 */  vaddc   $v9, $v9, $v4
/* 00001BE0 000B60 4B7D2110 */  vadd    $v4, $v4, $v29[3]
/* 00001BE4 000B64 4B1E42C6 */  vmudn   $v11, $v8, $v30[0]
/* 00001BE8 000B68 4B7D028F */  vmadh   $v10, $vzero, $v29[3]
/* 00001BEC 000B6C 4A895EC4 */  vmudl   $v27, $v11, $v9[0h]
/* 00001BF0 000B70 4A8956CD */  vmadm   $v27, $v10, $v9[0h]
/* 00001BF4 000B74 4A845ACE */  vmadn   $v11, $v11, $v4[0h]
/* 00001BF8 000B78 4A84528F */  vmadh   $v10, $v10, $v4[0h]
/* 00001BFC 000B7C 940B0246 */  lhu     $11, data_0246
/* 00001C00 000B80 C9641800 */  ldv     $v4[0], 0x0($11)
/* 00001C04 000B84 4AC83A50 */  vadd    $v9, $v7, $v8[2h]
/* 00001C08 000B88 4B7F0A4E */  vmadn   $v9, $v1, $v31[3]
/* 00001C0C 000B8C 4B044A46 */  vmudn   $v9, $v9, $v4[0]
/* 00001C10 000B90 4B241306 */  vmudn   $v12, $v2, $v4[1]
/* 00001C14 000B94 928B0017 */  lbu     $11, 0x17($20)
/* 00001C18 000B98 316B0011 */  andi    $11, $11, 0x11
/* 00001C1C 000B9C 256B0077 */  addiu   $11, $11, 0x77
/* 00001C20 000BA0 48CB0800 */  ctc2    $11, $vcc
/* 00001C24 000BA4 4A0939E7 */  vmrg    $v7, $v7, $v9
/* 00001C28 000BA8 4A0C1367 */  vmrg    $v13, $v2, $v12
/* 00001C2C 000BAC 4B4339D0 */  vadd    $v7, $v7, $v3[2]
/* 00001C30 000BB0 4ACB36D4 */  vaddc   $v27, $v6, $v11[2h]
/* 00001C34 000BB4 4ACA2890 */  vadd    $v2, $v5, $v10[2h]
/* 00001C38 000BB8 CBC81011 */  llv     $v8[0], 0x44($30)
/* 00001C3C 000BBC CBC81412 */  llv     $v8[8], 0x48($30)
/* 00001C40 000BC0 4AA810A0 */  vlt     $v2, $v2, $v8[1h]
/* 00001C44 000BC4 4A882A51 */  vsub    $v9, $v5, $v8[0h]
/* 00001C48 000BC8 4A882963 */  vge     $v5, $v5, $v8[0h]
/* 00001C4C 000BCC 4A090267 */  vmrg    $v9, $vzero, $v9
/* 00001C50 000BD0 4A022EE3 */  vge     $v27, $v5, $v2
/* 00001C54 000BD4 48410800 */  cfc2    $1, $vcc
/* 00001C58 000BD8 30210011 */  andi    $1, $1, 0x11
/* 00001C5C 000BDC 1420FD51 */  bnez    $1, next_cmd
/* 00001C60 000BE0 4AAD4EC7 */   vmudh  $v27, $v9, $v13[1h]
/* 00001C64 000BE4 4B2B5ADD */  vreadacc $v11, ACC_MIDDLE
/* 00001C68 000BE8 4B0A529D */  vreadacc $v10, ACC_LOWER
/* 00001C6C 000BEC 4B5E5EC4 */  vmudl   $v27, $v11, $v30[2]
/* 00001C70 000BF0 4B5E56CD */  vmadm   $v27, $v10, $v30[2]
/* 00001C74 000BF4 4B7F5ACE */  vmadn   $v11, $v11, $v31[3]
/* 00001C78 000BF8 4AC709CE */  vmadn   $v7, $v1, $v7[2h]
/* 00001C7C 000BFC 4B232A28 */  vand    $v8, $v5, $v3[1]
/* 00001C80 000C00 4AFE4207 */  vmudh   $v8, $v8, $v30[3h]
/* 00001C84 000C04 4A886A05 */  vmudm   $v8, $v13, $v8[0h]
/* 00001C88 000C08 4AA839D1 */  vsub    $v7, $v7, $v8[1h]
/* 00001C8C 000C0C 48012800 */  mfc2    $1, $v5[0]
/* 00001C90 000C10 48022C00 */  mfc2    $2, $v5[8]
/* 00001C94 000C14 48031000 */  mfc2    $3, $v2[0]
/* 00001C98 000C18 48041400 */  mfc2    $4, $v2[8]
/* 00001C9C 000C1C 30420FFF */  andi    $2, $2, 0xfff
/* 00001CA0 000C20 00010B00 */  sll     $1, $1, 0xc
/* 00001CA4 000C24 00220825 */  or      $1, $1, $2
/* 00001CA8 000C28 30840FFF */  andi    $4, $4, 0xfff
/* 00001CAC 000C2C 00031B00 */  sll     $3, $3, 0xc
/* 00001CB0 000C30 00641825 */  or      $3, $3, $4
/* 00001CB4 000C34 48056900 */  mfc2    $5, $v13[2]
/* 00001CB8 000C38 900200C9 */  lbu     $2, data_00C8 + 1
/* 00001CBC 000C3C 30420020 */  andi    $2, $2, 0x20
/* 00001CC0 000C40 10400003 */  beqz    $2, .L00001CD0
/* 00001CC4 000C44 00000000 */   nop    
/* 00001CC8 000C48 00052880 */  sll     $5, $5, 2
/* 00001CCC 000C4C 2063BFFC */  addi    $3, $3, -0x4004
.L00001CD0:
/* 00001CD0 000C50 900202AF */  lbu     $2, data_02AF
/* 00001CD4 000C54 38420002 */  xori    $2, $2, 2
/* 00001CD8 000C58 A00202AF */  sb      $2, data_02AF
/* 00001CDC 000C5C 800C02AE */  lb      $12, data_02AE
/* 00001CE0 000C60 EAFD0780 */  sbv     $v29[15], 0x0($23)
/* 00001CE4 000C64 144C0002 */  bne     $2, $12, .L00001CF0
/* 00001CE8 000C68 A00202AE */   sb     $2, data_02AE
/* 00001CEC 000C6C 26F70008 */  addiu   $23, $23, 8
.L00001CF0:
/* 00001CF0 000C70 EBC41200 */  slv     $v4[4], 0x0($30)
/* 00001CF4 000C74 8FCA0000 */  lw      $10, ($30)
/* 00001CF8 000C78 92840014 */  lbu     $4, 0x14($20)
/* 00001CFC 000C7C 92860015 */  lbu     $6, 0x15($20)
/* 00001D00 000C80 96870010 */  lhu     $7, 0x10($20)
/* 00001D04 000C84 96880012 */  lhu     $8, 0x12($20)
/* 00001D08 000C88 92890016 */  lbu     $9, 0x16($20)
/* 00001D0C 000C8C 00042540 */  sll     $4, $4, 0x15
/* 00001D10 000C90 000634C0 */  sll     $6, $6, 0x13
/* 00001D14 000C94 00862025 */  or      $4, $4, $6
/* 00001D18 000C98 00073A40 */  sll     $7, $7, 9
/* 00001D1C 000C9C 00872025 */  or      $4, $4, $7
/* 00001D20 000CA0 00882025 */  or      $4, $4, $8
/* 00001D24 000CA4 AEE40000 */  sw      $4, ($23)
/* 00001D28 000CA8 EAFD0600 */  sbv     $v29[12], 0x0($23)
/* 00001D2C 000CAC 00094D00 */  sll     $9, $9, 0x14
/* 00001D30 000CB0 012A4825 */  or      $9, $9, $10
/* 00001D34 000CB4 AEE90004 */  sw      $9, 4($23)
/* 00001D38 000CB8 A2E20004 */  sb      $2, 4($23)
/* 00001D3C 000CBC 96870004 */  lhu     $7, 4($20)
/* 00001D40 000CC0 EAFD1202 */  slv     $v29[4], 0x8($23)
/* 00001D44 000CC4 9688000C */  lhu     $8, 0xc($20)
/* 00001D48 000CC8 30E77FF8 */  andi    $7, $7, 0x7ff8
/* 00001D4C 000CCC 00073A40 */  sll     $7, $7, 9
/* 00001D50 000CD0 000840C3 */  sra     $8, $8, 3
/* 00001D54 000CD4 00E83025 */  or      $6, $7, $8
/* 00001D58 000CD8 20C6BFFC */  addi    $6, $6, -0x4004
/* 00001D5C 000CDC AEE6000C */  sw      $6, 0xc($23)
/* 00001D60 000CE0 A2E2000C */  sb      $2, 0xc($23)
/* 00001D64 000CE4 AEE30010 */  sw      $3, 0x10($23)
/* 00001D68 000CE8 EAFD0690 */  sbv     $v29[13], 0x10($23)
/* 00001D6C 000CEC AEE10014 */  sw      $1, 0x14($23)
/* 00001D70 000CF0 A2E20014 */  sb      $2, 0x14($23)
/* 00001D74 000CF4 EAE7080C */  ssv     $v7[0], 0x18($23)
/* 00001D78 000CF8 EAE70C0D */  ssv     $v7[8], 0x1A($23)
/* 00001D7C 000CFC A6E5001C */  sh      $5, 0x1c($23)
/* 00001D80 000D00 EAED0D0F */  ssv     $v13[10], 0x1E($23)
/* 00001D84 000D04 08000590 */  j       func_00001640
/* 00001D88 000D08 26F70020 */   addiu  $23, $23, 0x20
/* 00001D8C 000D0C CA822000 */  lqv     $v2[0], 0x0($20)
.L00001D90:
/* 00001D90 000D10 CBC7091A */  lsv     $v7[2], 0x34($30)
/* 00001D94 000D14 CBC70D1B */  lsv     $v7[10], 0x36($30)
/* 00001D98 000D18 CBC80818 */  lsv     $v8[0], 0x30($30)
/* 00001D9C 000D1C CBC80C19 */  lsv     $v8[8], 0x32($30)
/* 00001DA0 000D20 4B1F1245 */  vmudm   $v9, $v2, $v31[0]
/* 00001DA4 000D24 4B7DFA8E */  vmadn   $v10, $v31, $v29[3]
/* 00001DA8 000D28 4B274330 */  vrcp    $v12[0], $v7[1]
/* 00001DAC 000D2C 4B1B42F2 */  vrcph   $v11[0], $v27[0]
/* 00001DB0 000D30 4BA76330 */  vrcp    $v12[4], $v7[5]
/* 00001DB4 000D34 4B1B62F2 */  vrcph   $v11[4], $v27[0]
/* 00001DB8 000D38 4A0B0EC6 */  vmudn   $v27, $v1, $v11
/* 00001DBC 000D3C 4B3D630E */  vmadn   $v12, $v12, $v29[1]
/* 00001DC0 000D40 4B3D5ACF */  vmadh   $v11, $v11, $v29[1]
/* 00001DC4 000D44 4B3F39C5 */  vmudm   $v7, $v7, $v31[1]
/* 00001DC8 000D48 4B7DFB4E */  vmadn   $v13, $v31, $v29[3]
/* 00001DCC 000D4C 4A026EC6 */  vmudn   $v27, $v13, $v2
/* 00001DD0 000D50 4A0239CF */  vmadh   $v7, $v7, $v2
/* 00001DD4 000D54 4A8C56C4 */  vmudl   $v27, $v10, $v12[0h]
/* 00001DD8 000D58 4A8C4ECD */  vmadm   $v27, $v9, $v12[0h]
/* 00001DDC 000D5C 4A8B518E */  vmadn   $v6, $v10, $v11[0h]
/* 00001DE0 000D60 4A8B494F */  vmadh   $v5, $v9, $v11[0h]
/* 00001DE4 000D64 4B444210 */  vadd    $v8, $v8, $v4[2]
/* 00001DE8 000D68 4B034228 */  vand    $v8, $v8, $v3[0]
/* 00001DEC 000D6C 4B3D294E */  vmadn   $v5, $v5, $v29[1]
/* 00001DF0 000D70 4B2748B3 */  vmov    $v2[1], $v7[1]
/* 00001DF4 000D74 080006F1 */  j       .L00001BC4
/* 00001DF8 000D78 4BA768B3 */   vmov   $v2[5], $v7[5]

.align 8
Overlay2End:

.orga 0xF2C

G_LOAD_UCODE_handler:
load_overlay_0_and_enter:
/* 00001FAC 000F2C 200C1000 */  li      $12, 0x1000
/* 00001FB0 000F30 200B0304 */  li      $11, Overlay0Info
load_overlay_and_enter:
/* 00001FB4 000F34 8D780000 */  lw      $24, 0($11)
/* 00001FB8 000F38 95730004 */  lhu     $19, 4($11)
/* 00001FBC 000F3C 0C0007F6 */  jal     dma_read_write
/* 00001FC0 000F40 95740006 */   lhu    $20, 6($11)
/* 00001FC4 000F44 359F0000 */  move    $ra, $12

dma_wait:
/* 00001FC8 000F48 400B3000 */  mfc0    $11, SP_DMA_BUSY
@@dma_busy:
/* 00001FCC 000F4C 1560FFFF */  bnez    $11, @@dma_busy
/* 00001FD0 000F50 400B3000 */   mfc0   $11, SP_DMA_BUSY
return_routine:
/* 00001FD4 000F54 03E00008 */  jr      $ra

// $20 = SP addr
// $24 = DRAM addr
// $19 = len
dma_read_write:
/* 00001FD8 000F58 400B2800 */   mfc0   $11, SP_DMA_FULL
@@dma_full:
/* 00001FDC 000F5C 1560FFFF */  bnez    $11, @@dma_full
/* 00001FE0 000F60 400B2800 */   mfc0   $11, SP_DMA_FULL
/* 00001FE4 000F64 40940000 */  mtc0    $20, SP_MEM_ADDR
/* 00001FE8 000F68 06800003 */  bltz    $20, dma_write
/* 00001FEC 000F6C 40980800 */   mtc0   $24, SP_DRAM_ADDR
/* 00001FF0 000F70 03E00008 */  jr      $ra
/* 00001FF4 000F74 40931000 */   mtc0   $19, SP_RD_LEN
dma_write:
/* 00001FF8 000F78 03E00008 */  jr      $ra
/* 00001FFC 000F7C 40931800 */   mtc0   $19, SP_WR_LEN

.headersize 0x00001000 - orga()
Overlay0Address:

/* 00001000 000F80 02F65822 */  sub     $11, $23, $22
/* 00001004 000F84 256C0317 */  addiu   $12, $11, 0x317
/* 00001008 000F88 05910193 */  bgezal  $12, .L00001658
/* 0000100C 000F8C 00000000 */   nop    
/* 00001010 000F90 0C0007F2 */  jal     dma_wait
/* 00001014 000F94 8C1800F0 */   lw     $24, data_00F0
/* 00001018 000F98 0420001A */  bltz    $1, .L00001084
/* 0000101C 000F9C 40984800 */   mtc0   $24, DPC_END
/* 00001020 000FA0 1420000F */  bnez    $1, .L00001060
/* 00001024 000FA4 035BD020 */   add    $26, $26, $27
/* 00001028 000FA8 8F7804BC */  lw      $24, 0x4bc($27)
/* 0000102C 000FAC AC1A0FF0 */  sw      $26, OSTask_addr + OS_TASK_OFF_DATA
/* 00001030 000FB0 AC180FD0 */  sw      $24, OSTask_addr + OS_TASK_OFF_UCODE
/* 00001034 000FB4 24141080 */  la      $20, 0x1080     // TODO IMEM label
/* 00001038 000FB8 0C0007F6 */  jal     dma_read_write
/* 0000103C 000FBC 20130F47 */   li     $19, 0xf48-1
/* 00001040 000FC0 8C1800D8 */  lw      $24, data_00D8
/* 00001044 000FC4 24140180 */  la      $20, 0x180
/* 00001048 000FC8 33330FFF */  andi    $19, $25, 0xfff
/* 0000104C 000FCC 0314C020 */  add     $24, $24, $20
/* 00001050 000FD0 0C0007F6 */  jal     dma_read_write
/* 00001054 000FD4 02749822 */   sub    $19, $19, $20
/* 00001058 000FD8 080007F2 */  j       dma_wait
/* 0000105C 000FDC 201F1084 */   li     $ra, .L00001084
.L00001060:
/* 00001060 000FE0 8C0B0FD0 */  lw      $11, OSTask_addr + OS_TASK_OFF_UCODE
/* 00001064 000FE4 AC1A0BF8 */  sw      $26, data_0BF8
/* 00001068 000FE8 AC0B0BFC */  sw      $11, data_0BFC
/* 0000106C 000FEC 200C5000 */  li      $12, SP_SET_SIG1 | SP_SET_SIG2
/* 00001070 000FF0 8C180FF8 */  lw      $24, OSTask_addr + OS_TASK_OFF_YIELD
/* 00001074 000FF4 20148000 */  li      $20, -0x8000
/* 00001078 000FF8 20130BFF */  li      $19, 0xC00-1
/* 0000107C 000FFC 080007F6 */  j       dma_read_write
/* 00001080 001000 201F1088 */   li     $ra, .L00001088
.L00001084:
/* 00001084 001004 200C4000 */  li      $12, SP_SET_SIG2
.L00001088:
/* 00001088 001008 408C2000 */  mtc0    $12, SP_STATUS
/* 0000108C 00100C 0000000D */  break   
/* 00001090 001010 00000000 */  nop     

.align 8
Overlay0End:

.headersize 0x00001000 - orga()
Overlay1Address:

/* 00001000 001018 0C000455 */  jal     segmented_to_physical
/* 00001004 00101C 877404B9 */   lh     $20, 0x4b9($27)
/* 00001008 001020 33330FF8 */  andi    $19, $25, 0xff8
/* 0000100C 001024 0014A083 */  sra     $20, $20, 2
/* 00001010 001028 080007F6 */  j       dma_read_write
/* 00001014 00102C 201F11A0 */   li     $ra, dma_wait_and_next_cmd

G_TEXRECTFLIP_handler:
G_RDPHALF_0_handler:
/* 00001018 001030 AD79F0B8 */  sw      cmd_w0, -0xf48($11)
G_RDPHALF_1_handler:
/* 0000101C 001034 08000469 */  j       next_cmd
/* 00001020 001038 AD78F0BC */   sw     cmd_w1, -0xf44($11)

G_SELECT_DL_handler:
/* 00001024 00103C 900100D1 */  lbu     $1, data_00D0 + 1
/* 00001028 001040 8C0300D4 */  lw      $3, data_00D4
/* 0000102C 001044 03002827 */  not     $5, cmd_w1
/* 00001030 001048 8C2202B0 */  lw      $2, 0x2b0($1)
/* 00001034 00104C A41900D0 */  sh      $25, data_00D0
/* 00001038 001050 00782024 */  and     $4, $3, cmd_w1
/* 0000103C 001054 00583024 */  and     $6, $2, cmd_w1
/* 00001040 001058 10C30058 */  beq     $6, $3, next_cmd
/* 00001044 00105C 00453024 */   and    $6, $2, $5
/* 00001048 001060 00C43025 */  or      $6, $6, $4
/* 0000104C 001064 AC2602B0 */  sw      $6, 0x2b0($1)
/* 00001050 001068 8C1800D0 */  lw      $24, data_00D0
G_DL_handler:
/* 00001054 00106C 900100DE */  lbu     $1, data_00DE
/* 00001058 001070 001913C0 */  sll     $2, cmd_w0, 0xf
/* 0000105C 001074 0C000455 */  jal     segmented_to_physical
/* 00001060 001078 035B1820 */   add    $3, $26, $27
/* 00001064 00107C 04400048 */  bltz    $2, .L00001188
/* 00001068 001080 371A0000 */   move   $26, cmd_w1
/* 0000106C 001084 AC230138 */  sw      $3, 0x138($1)
/* 00001070 001088 20210004 */  addi    $1, $1, 4
func_00001074:
/* 00001074 00108C 08000462 */  j       .L00001188
/* 00001078 001090 A00100DE */   sb     $1, data_00DE

G_ENDDL_handler:
/* 0000107C 001094 900100DE */  lbu     $1, data_00DE
/* 00001080 001098 102003CA */  beqz    $1, load_overlay_0_and_enter
/* 00001084 00109C 2021FFFC */   addi   $1, $1, -4
/* 00001088 0010A0 0800041D */  j       func_00001074
/* 0000108C 0010A4 8C3A0138 */   lw     $26, 0x138($1)

#define G_MOVEWORD      0xDB

G_MOVEWORD_handler:
/* 00001090 0010A8 00190C02 */  srl     $1, cmd_w0, 0x10                                        // $1 = ((G_MOVEWORD << 8) | (index))
/* 00001094 0010AC 94222886 */  lhu     $2, (data_0388 - 2 + 0x10000 - (G_MOVEWORD << 8))($1)   // 0x2886 + (G_MOVEWORD << 8) = 0x10386 ={0..11}= 0x386
/* 00001098 0010B0 00591020 */  add     $2, $2, cmd_w0
/* 0000109C 0010B4 08000469 */  j       next_cmd
/* 000010A0 0010B8 AC580000 */   sw     cmd_w1, ($2)

G_OBJ_MOVEMEM_handler:
/* 000010A4 0010BC 0C000455 */  jal     segmented_to_physical       // convert cmd_w1 to physical addr
/* 000010A8 0010C0 97340388 */   lhu    $20, (data_0388)(cmd_w0)    // load DMEM address from movemem table
/* 000010AC 0010C4 937304B9 */  lbu     $19, 0x4b9($27)             // load DMA length
/* 000010B0 0010C8 080007F6 */  j       dma_read_write              // do DMA

G_SETOTHERMODE_H_handler:
/* 000010B4 0010CC 201F11A0 */   li     $ra, dma_wait_and_next_cmd
G_SETOTHERMODE_L_handler:
/* 000010B8 0010D0 8D63F014 */  lw      $3, -0xfec($11)
/* 000010BC 0010D4 3C028000 */  lui     $2, 0x8000
/* 000010C0 0010D8 03221007 */  srav    $2, $2, cmd_w0
/* 000010C4 0010DC 00190A02 */  srl     $1, cmd_w0, 8
/* 000010C8 0010E0 00221006 */  srlv    $2, $2, $1
/* 000010CC 0010E4 00401027 */  not     $2, $2
/* 000010D0 0010E8 00621824 */  and     $3, $3, $2
/* 000010D4 0010EC 00781825 */  or      $3, $3, cmd_w1
/* 000010D8 0010F0 AD63F014 */  sw      $3, -0xfec($11)
/* 000010DC 0010F4 8C1900C8 */  lw      cmd_w0, data_00C8
/* 000010E0 0010F8 08000450 */  j       G_RDP_handler
/* 000010E4 0010FC 8C1800CC */   lw     cmd_w1, data_00CC

G_OBJ_RENDERMODE_handler:
/* 000010E8 001100 A0180268 */  sb      cmd_w1, data_0268
/* 000010EC 001104 33010008 */  andi    $1, cmd_w1, 8
/* 000010F0 001108 24210248 */  addiu   $1, $1, 0x248
/* 000010F4 00110C A4010244 */  sh      $1, data_0244
/* 000010F8 001110 33210008 */  andi    $1, cmd_w0, 8
/* 000010FC 001114 24210258 */  addiu   $1, $1, 0x258
/* 00001100 001118 A4010246 */  sh      $1, data_0246
/* 00001104 00111C 33010018 */  andi    $1, cmd_w1, 0x18
/* 00001108 001120 00010842 */  srl     $1, $1, 1
/* 0000110C 001124 8C230234 */  lw      $3, 0x234($1)
/* 00001110 001128 33010070 */  andi    $1, cmd_w1, 0x70
/* 00001114 00112C 00010882 */  srl     $1, $1, 2
/* 00001118 001130 8C220214 */  lw      $2, 0x214($1)
/* 0000111C 001134 AC0301FC */  sw      $3, data_01FC
/* 00001120 001138 08000469 */  j       next_cmd
/* 00001124 00113C AC0201F8 */   sw     $2, data_01F8

G_RDPHALF_2_handler:
/* 00001128 001140 C81B1818 */  ldv     $v27[0], (data_00C0)($zero)
/* 0000112C 001144 C81B1C1A */  ldv     $v27[8], (data_00D0)($zero)
/* 00001130 001148 8C1900D8 */  lw      $25, data_00D8
/* 00001134 00114C 22F70010 */  addi    $23, $23, 0x10
/* 00001138 001150 EAFB187E */  sdv     $v27[0], -0x10($23)
/* 0000113C 001154 EAFB1C7F */  sdv     $v27[8], -0x8($23)
G_RDP_handler:
/* 00001140 001158 AEF80004 */  sw      cmd_w1, 4($23)
G_NOOP_handler:
G_SYNC_handler:
/* 00001144 00115C AEF90000 */  sw      cmd_w0, ($23)
/* 00001148 001160 08000590 */  j       func_00001640
/* 0000114C 001164 22F70008 */   addi   $23, $23, 8

G_SETxIMG_handler:
/* 00001150 001168 201F1140 */  li      $ra, G_RDP_handler
segmented_to_physical: // Converts cmd_w1 / $24 to a physical DRAM address
/* 00001154 00116C 00185D82 */  srl     $11, cmd_w1, 0x16
/* 00001158 001170 316B003C */  andi    $11, $11, 0x3c
/* 0000115C 001174 8D6B00F8 */  lw      $11, (data_00F8)($11)
/* 00001160 001178 0018C200 */  sll     cmd_w1, cmd_w1, 8
/* 00001164 00117C 0018C202 */  srl     cmd_w1, cmd_w1, 8
/* 00001168 001180 03E00008 */  jr      $ra
/* 0000116C 001184 030BC020 */   add    cmd_w1, cmd_w1, $11

G_RDPSETOTHERMODE_handler:
/* 00001170 001188 AC1900C8 */  sw      cmd_w0, data_00C8
/* 00001174 00118C 08000450 */  j       G_RDP_handler
/* 00001178 001190 AC1800CC */   sw     cmd_w1, data_00CC

.align 8
Overlay1End:

.headersize Overlay23LoadAddress - orga()
Overlay3Address:

/* 000016D0 001198 0C000455 */  jal     segmented_to_physical
/* 000016D4 00119C 20130017 */   li     $19, 0x18-1
/* 000016D8 0011A0 0C0007F6 */  jal     dma_read_write
/* 000016DC 0011A4 201403C0 */   li     $20, data_03C0
/* 000016E0 0011A8 200C16DC */  li      $12, 0x16dc     // TODO IMEM label
/* 000016E4 0011AC 080007ED */  j       load_overlay_and_enter
/* 000016E8 0011B0 200B0314 */   li     $11, Overlay2Info

/* 000016EC 0011B4 00000000 */  nop     

func_000016F0:
/* 000016F0 0011B8 200C1638 */  li      $12, .L00001638
/* 000016F4 0011BC 080007ED */  j       load_overlay_and_enter
/* 000016F8 0011C0 200B0314 */   li     $11, Overlay2Info

func_000016FC:
/* 000016FC 0011C4 0C0007F6 */  jal     dma_read_write
/* 00001700 0011C8 20130027 */   li     $19, 0x28-1
/* 00001704 0011CC 200103D8 */  li      $1, data_03D8
/* 00001708 0011D0 CBC21211 */  llv     $v2[4], 0x44($30)
/* 0000170C 0011D4 0C0007F2 */  jal     dma_wait
/* 00001710 0011D8 CBC21612 */   llv    $v2[12], 0x48($30)
/* 00001714 0011DC CA832000 */  lqv     $v3[0], 0x0($20)
/* 00001718 0011E0 CA84080E */  lsv     $v4[0], 0x1C($20)
/* 0000171C 0011E4 CA840C0F */  lsv     $v4[8], 0x1E($20)
/* 00001720 0011E8 4B3F2145 */  vmudm   $v5, $v4, $v31[1]
/* 00001724 0011EC 4B7DF98E */  vmadn   $v6, $v31, $v29[3]
/* 00001728 0011F0 4B044230 */  vrcp    $v8[0], $v4[0]
/* 0000172C 0011F4 4B7D41F2 */  vrcph   $v7[0], $v29[3]
/* 00001730 0011F8 4B846230 */  vrcp    $v8[4], $v4[4]
/* 00001734 0011FC 4B7D61F2 */  vrcph   $v7[4], $v29[3]
/* 00001738 001200 4B3D0EC6 */  vmudn   $v27, $v1, $v29[1]
/* 0000173C 001204 4B1F46CC */  vmadl   $v27, $v8, $v31[0]
/* 00001740 001208 4B1F39CD */  vmadm   $v7, $v7, $v31[0]
/* 00001744 00120C 4B7DFA0E */  vmadn   $v8, $v31, $v29[3]
/* 00001748 001210 4B9F1EC6 */  vmudn   $v27, $v3, $v31[4]
/* 0000174C 001214 4B7DFA8F */  vmadh   $v10, $v31, $v29[3]
/* 00001750 001218 900203CB */  lbu     $2, 0x3cb($zero)
/* 00001754 00121C 4B7F0EC6 */  vmudn   $v27, $v1, $v31[3]
/* 00001758 001220 4AA346CC */  vmadl   $v27, $v8, $v3[1h]
/* 0000175C 001224 30420001 */  andi    $2, $2, 1
/* 00001760 001228 4AA33B0D */  vmadm   $v12, $v7, $v3[1h]
/* 00001764 00122C 4B7DFACE */  vmadn   $v11, $v31, $v29[3]
/* 00001768 001230 4B1C5AE8 */  vand    $v11, $v11, $v28[0]
/* 0000176C 001234 4A8B1AD5 */  vsubc   $v11, $v3, $v11[0h]
/* 00001770 001238 4A8C0311 */  vsub    $v12, $vzero, $v12[0h]
/* 00001774 00123C 4B7D6323 */  vge     $v12, $v12, $v29[3]
/* 00001778 001240 4B7D5AE7 */  vmrg    $v11, $v11, $v29[3]
/* 0000177C 001244 10400003 */  beqz    $2, .L0000178C
/* 00001780 001248 4AEB1B51 */   vsub   $v13, $v3, $v11[3h]
/* 00001784 00124C 4B6B1ED0 */  vadd    $v27, $v3, $v11[3]
/* 00001788 001250 4B5B50F3 */  vmov    $v3[2], $v27[2]
.L0000178C:
/* 0000178C 001254 4A031251 */  vsub    $v9, $v2, $v3
/* 00001790 001258 4AE21AD1 */  vsub    $v11, $v3, $v2[3h]
/* 00001794 00125C 4AED0ACE */  vmadn   $v11, $v1, $v13[3h]
/* 00001798 001260 4B7D4A63 */  vge     $v9, $v9, $v29[3]
/* 0000179C 001264 4B7D5AE3 */  vge     $v11, $v11, $v29[3]
/* 000017A0 001268 4AC96B51 */  vsub    $v13, $v13, $v9[2h]
/* 000017A4 00126C 4ACB6B51 */  vsub    $v13, $v13, $v11[2h]
/* 000017A8 001270 4AC91890 */  vadd    $v2, $v3, $v9[2h]
/* 000017AC 001274 4B3D6EE3 */  vge     $v27, $v13, $v29[1]
/* 000017B0 001278 48CB0800 */  ctc2    $11, $vcc
/* 000017B4 00127C 316B0088 */  andi    $11, $11, 0x88
/* 000017B8 001280 1560FE7A */  bnez    $11, next_cmd
/* 000017BC 001284 4AED1310 */   vadd   $v12, $v2, $v13[3h]
/* 000017C0 001288 4BED78B3 */  vmov    $v2[7], $v13[7]
/* 000017C4 00128C 4B9F16C5 */  vmudm   $v27, $v2, $v31[4]
/* 000017C8 001290 E8220A00 */  ssv     $v2[4], 0x0($1)
/* 000017CC 001294 E82C0A01 */  ssv     $v12[4], 0x2($1)
/* 000017D0 001298 10400002 */  beqz    $2, .L000017DC
/* 000017D4 00129C E83B1601 */   slv    $v27[12], 0x4($1)
/* 000017D8 0012A0 4B4B5273 */  vmov    $v9[2], $v11[2]
.L000017DC:
/* 000017DC 0012A4 4BDF1886 */  vmudn   $v2, $v3, $v31[6]
/* 000017E0 0012A8 4AC92306 */  vmudn   $v12, $v4, $v9[2h]
/* 000017E4 0012AC 4B7DFACF */  vmadh   $v11, $v31, $v29[3]
/* 000017E8 0012B0 4BFE6306 */  vmudn   $v12, $v12, $v30[7]
/* 000017EC 0012B4 4BFE5ACF */  vmadh   $v11, $v11, $v30[7]
/* 000017F0 0012B8 4A830ACF */  vmadh   $v11, $v1, $v3[0h]
/* 000017F4 0012BC 48035800 */  mfc2    $3, $v11[0]
/* 000017F8 0012C0 48051100 */  mfc2    $5, $v2[2]
/* 000017FC 0012C4 48045C00 */  mfc2    $4, $v11[8]
/* 00001800 0012C8 8C0703D0 */  lw      $7, 0x3d0($zero)
/* 00001804 0012CC 48061500 */  mfc2    $6, $v2[10]
.L00001808:
/* 00001808 0012D0 00655822 */  sub     $11, $3, $5
/* 0000180C 0012D4 05600005 */  bltz    $11, .L00001824
/* 00001810 0012D8 48835800 */   mtc2   $3, $v11[0]
/* 00001814 0012DC 00651822 */  sub     $3, $3, $5
/* 00001818 0012E0 20840020 */  addi    $4, $4, 0x20
/* 0000181C 0012E4 08000602 */  j       .L00001808
/* 00001820 0012E8 20E70020 */   addi   $7, $7, 0x20
.L00001824:
/* 00001824 0012EC 00865822 */  sub     $11, $4, $6
/* 00001828 0012F0 05600004 */  bltz    $11, .L0000183C
/* 0000182C 0012F4 48845C00 */   mtc2   $4, $v11[8]
/* 00001830 0012F8 00862022 */  sub     $4, $4, $6
/* 00001834 0012FC 08000609 */  j       .L00001824
/* 00001838 001300 00E63822 */   sub    $7, $7, $6
.L0000183C:
/* 0000183C 001304 AC0703F0 */  sw      $7, 0x3f0($zero)
/* 00001840 001308 00875822 */  sub     $11, $4, $7
/* 00001844 00130C 000B5940 */  sll     $11, $11, 5
/* 00001848 001310 AC0B03E4 */  sw      $11, 0x3e4($zero)
/* 0000184C 001314 4AED2306 */  vmudn   $v12, $v4, $v13[3h]
/* 00001850 001318 4B7DFA4F */  vmadh   $v9, $v31, $v29[3]
/* 00001854 00131C 4BFE6306 */  vmudn   $v12, $v12, $v30[7]
/* 00001858 001320 4BFE4A4F */  vmadh   $v9, $v9, $v30[7]
/* 0000185C 001324 900B0268 */  lbu     $11, data_0268
/* 00001860 001328 316B0008 */  andi    $11, $11, 8
/* 00001864 00132C 000B30C2 */  srl     $6, $11, 3
/* 00001868 001330 48044800 */  mfc2    $4, $v9[0]
/* 0000186C 001334 A00603E0 */  sb      $6, 0x3e0($zero)
/* 00001870 001338 00065940 */  sll     $11, $6, 5
/* 00001874 00133C 2084000B */  addi    $4, $4, 0xb
/* 00001878 001340 00646020 */  add     $12, $3, $4
/* 0000187C 001344 0185602A */  slt     $12, $12, $5
/* 00001880 001348 398C0001 */  xori    $12, $12, 1
/* 00001884 00134C A00C03E1 */  sb      $12, 0x3e1($zero)
/* 00001888 001350 900B03C6 */  lbu     $11, 0x3c6($zero)
/* 0000188C 001354 900C03C7 */  lbu     $12, 0x3c7($zero)
/* 00001890 001358 000B5840 */  sll     $11, $11, 1
/* 00001894 00135C 216B02A4 */  addi    $11, $11, 0x2a4
/* 00001898 001360 000C6080 */  sll     $12, $12, 2
/* 0000189C 001364 218C0294 */  addi    $12, $12, 0x294
/* 000018A0 001368 C96C0A00 */  lsv     $v12[4], 0x0($11)
/* 000018A4 00136C C98C1000 */  llv     $v12[0], 0x0($12)
/* 000018A8 001370 48866800 */  mtc2    $6, $v13[0]
/* 000018AC 001374 4B632386 */  vmudn   $v14, $v4, $v3[3]
/* 000018B0 001378 4B7DFBCF */  vmadh   $v15, $v31, $v29[3]
/* 000018B4 00137C 4B5C6EC6 */  vmudn   $v27, $v13, $v28[2]
/* 000018B8 001380 4BFE76CC */  vmadl   $v27, $v14, $v30[7]
/* 000018BC 001384 4BFE7BCD */  vmadm   $v15, $v15, $v30[7]
/* 000018C0 001388 4B7DFB8E */  vmadn   $v14, $v31, $v29[3]
/* 000018C4 00138C 4B2276D5 */  vsubc   $v27, $v14, $v2[1]
/* 000018C8 001390 4B7D7EE0 */  vlt     $v27, $v15, $v29[3]
/* 000018CC 001394 4B2276E7 */  vmrg    $v27, $v14, $v2[1]
/* 000018D0 001398 4B0C08CE */  vmadn   $v3, $v1, $v12[0]
/* 000018D4 00139C 4B3D0EC7 */  vmudh   $v27, $v1, $v29[1]
/* 000018D8 0013A0 4B2C18CD */  vmadm   $v3, $v3, $v12[1]
/* 000018DC 0013A4 E8230805 */  ssv     $v3[0], 0xA($1)
/* 000018E0 0013A8 4B0343F0 */  vrcp    $v15[0], $v3[0]
/* 000018E4 0013AC 4B7D43B2 */  vrcph   $v14[0], $v29[3]
/* 000018E8 0013B0 4B7F6EC7 */  vmudh   $v27, $v13, $v31[3]
/* 000018EC 0013B4 4B4C7ECC */  vmadl   $v27, $v15, $v12[2]
/* 000018F0 0013B8 4B4C740D */  vmadm   $v16, $v14, $v12[2]
/* 000018F4 0013BC 4B3C8346 */  vmudn   $v13, $v16, $v28[1]
/* 000018F8 0013C0 4B7DF8CF */  vmadh   $v3, $v31, $v29[3]
/* 000018FC 0013C4 4B886EC4 */  vmudl   $v27, $v13, $v8[4]
/* 00001900 0013C8 4B881ECD */  vmadm   $v27, $v3, $v8[4]
/* 00001904 0013CC 4B876B4E */  vmadn   $v13, $v13, $v7[4]
/* 00001908 0013D0 4B8718CF */  vmadh   $v3, $v3, $v7[4]
/* 0000190C 0013D4 E8230808 */  ssv     $v3[0], 0x10($1)
/* 00001910 0013D8 E82D0809 */  ssv     $v13[0], 0x12($1)
/* 00001914 0013DC C82E0807 */  lsv     $v14[0], 0xE($1)
/* 00001918 0013E0 C82F0806 */  lsv     $v15[0], 0xC($1)
/* 0000191C 0013E4 4B8876C4 */  vmudl   $v27, $v14, $v8[4]
/* 00001920 0013E8 4B887ECD */  vmadm   $v27, $v15, $v8[4]
/* 00001924 0013EC 4B87738E */  vmadn   $v14, $v14, $v7[4]
/* 00001928 0013F0 4B877BCF */  vmadh   $v15, $v15, $v7[4]
/* 0000192C 0013F4 4B9C73A8 */  vand    $v14, $v14, $v28[4]
/* 00001930 0013F8 4B0346F2 */  vrcph   $v27[0], $v3[0]
/* 00001934 0013FC 4B0D41F1 */  vrcpl   $v7[0], $v13[0]
/* 00001938 001400 4B7D4232 */  vrcph   $v8[0], $v29[3]
/* 0000193C 001404 4A0776C4 */  vmudl   $v27, $v14, $v7
/* 00001940 001408 4A077ECD */  vmadm   $v27, $v15, $v7
/* 00001944 00140C 4A08744E */  vmadn   $v17, $v14, $v8
/* 00001948 001410 4A087C8F */  vmadh   $v18, $v15, $v8
/* 0000194C 001414 4B5F8C46 */  vmudn   $v17, $v17, $v31[2]
/* 00001950 001418 4B5F948F */  vmadh   $v18, $v18, $v31[2]
/* 00001954 00141C 4B3D0C4F */  vmadh   $v17, $v1, $v29[1]
/* 00001958 001420 4A1169C6 */  vmudn   $v7, $v13, $v17
/* 0000195C 001424 4A111A0F */  vmadh   $v8, $v3, $v17
/* 00001960 001428 4A0776D5 */  vsubc   $v27, $v14, $v7
/* 00001964 00142C 4A087EE3 */  vge     $v27, $v15, $v8
/* 00001968 001430 4A128CA7 */  vmrg    $v18, $v17, $v18
/* 0000196C 001434 4B1269C6 */  vmudn   $v7, $v13, $v18[0]
/* 00001970 001438 4B121A0F */  vmadh   $v8, $v3, $v18[0]
/* 00001974 00143C 4B9C3C68 */  vand    $v17, $v7, $v28[4]
/* 00001978 001440 4A1174D5 */  vsubc   $v19, $v14, $v17
/* 0000197C 001444 4A087D11 */  vsub    $v20, $v15, $v8
/* 00001980 001448 4B3F9EC4 */  vmudl   $v27, $v19, $v31[1]
/* 00001984 00144C 4B3FA3CD */  vmadm   $v15, $v20, $v31[1]
/* 00001988 001450 4B7DFB8E */  vmadn   $v14, $v31, $v29[3]
/* 0000198C 001454 4B847386 */  vmudn   $v14, $v14, $v4[4]
/* 00001990 001458 4B847BCF */  vmadh   $v15, $v15, $v4[4]
/* 00001994 00145C 4B3D1EC7 */  vmudh   $v27, $v3, $v29[1]
/* 00001998 001460 4BBC3EE8 */  vand    $v27, $v7, $v28[5]
/* 0000199C 001464 4B3D6ECE */  vmadn   $v27, $v13, $v29[1]
/* 000019A0 001468 4B7F99CE */  vmadn   $v7, $v19, $v31[3]
/* 000019A4 00146C 4B7FA20F */  vmadh   $v8, $v20, $v31[3]
/* 000019A8 001470 E828080A */  ssv     $v8[0], 0x14($1)
/* 000019AC 001474 E827080B */  ssv     $v7[0], 0x16($1)
/* 000019B0 001478 4B1F70C5 */  vmudm   $v3, $v14, $v31[0]
/* 000019B4 00147C 4BDC18E8 */  vand    $v3, $v3, $v28[6]
/* 000019B8 001480 4B3F7384 */  vmudl   $v14, $v14, $v31[1]
/* 000019BC 001484 4B3F7BCD */  vmadm   $v15, $v15, $v31[1]
/* 000019C0 001488 4B7DFB8E */  vmadn   $v14, $v31, $v29[3]
/* 000019C4 00148C 4B0E81D1 */  vsub    $v7, $v16, $v14[0]
/* 000019C8 001490 4B1049F3 */  vmov    $v7[1], $v16[0]
/* 000019CC 001494 080006EF */  j       func_00001BBC
/* 000019D0 001498 E827100A */   slv    $v7[0], 0x28($1)

func_000019D4:
/* 000019D4 00149C 8C0B03FC */  lw      $11, 0x3fc($zero)
/* 000019D8 0014A0 EAFD0780 */  sbv     $v29[15], 0x0($23)
/* 000019DC 0014A4 22F70010 */  addi    $23, $23, 0x10
/* 000019E0 0014A8 01745825 */  or      $11, $11, $20
/* 000019E4 0014AC AEEBFFF8 */  sw      $11, -8($23)
/* 000019E8 0014B0 EAFD147F */  slv     $v29[8], -0x4($23)
func_000019EC:
/* 000019EC 0014B4 8C0B03F8 */  lw      $11, 0x3f8($zero)
/* 000019F0 0014B8 AEF10004 */  sw      $17, 4($23)
/* 000019F4 0014BC 22F70018 */  addi    $23, $23, 0x18
/* 000019F8 0014C0 AEEBFFE8 */  sw      $11, -0x18($23)
/* 000019FC 0014C4 EAFD0770 */  sbv     $v29[14], -0x10($23)
/* 00001A00 0014C8 3C0BF400 */  lui     $11, 0xf400
/* 00001A04 0014CC AEEBFFF8 */  sw      $11, -8($23)
/* 00001A08 0014D0 226B06FF */  addi    $11, $19, 0x6ff
/* 00001A0C 0014D4 A6EBFFFC */  sh      $11, -4($23)
/* 00001A10 0014D8 00125880 */  sll     $11, $18, 2
/* 00001A14 0014DC 216BFFFF */  addi    $11, $11, -1
/* 00001A18 0014E0 03E00008 */  jr      $ra
/* 00001A1C 0014E4 A6EBFFFE */   sh     $11, -2($23)

func_00001A20:
/* 00001A20 0014E8 900B03E0 */  lbu     $11, 0x3e0($zero)
/* 00001A24 0014EC 900C03E1 */  lbu     $12, 0x3e1($zero)
/* 00001A28 0014F0 008BC820 */  add     $25, $4, $11
/* 00001A2C 0014F4 006CC022 */  sub     $24, $3, $12
/* 00001A30 0014F8 03195822 */  sub     $11, $24, $25
/* 00001A34 0014FC 05600007 */  bltz    $11, .L00001A54
/* 00001A38 001500 941303E2 */   lhu    $19, 0x3e2($zero)
/* 00001A3C 001504 37320000 */  move    $18, $25
/* 00001A40 001508 0C00067B */  jal     func_000019EC
/* 00001A44 00150C 34B10000 */   move   $17, $5
/* 00001A48 001510 00641822 */  sub     $3, $3, $4
/* 00001A4C 001514 08000768 */  j       func_00001DA0
/* 00001A50 001518 00A62820 */   add    $5, $5, $6
.L00001A54:
/* 00001A54 00151C 4893C000 */  mtc2    $19, $v24[0]
/* 00001A58 001520 03239022 */  sub     $18, $25, $3
/* 00001A5C 001524 1A40000E */  blez    $18, .L00001A98
/* 00001A60 001528 20740000 */   addi   $20, $3, 0
/* 00001A64 00152C 8C1103C0 */  lw      $17, 0x3c0($zero)
/* 00001A68 001530 940B03F4 */  lhu     $11, 0x3f4($zero)
/* 00001A6C 001534 329F0001 */  andi    $ra, $20, 1
/* 00001A70 001538 13E00005 */  beqz    $ra, .L00001A88
/* 00001A74 00153C 022B8820 */   add    $17, $17, $11
/* 00001A78 001540 940B03F6 */  lhu     $11, 0x3f6($zero)
/* 00001A7C 001544 2294FFFF */  addi    $20, $20, -1
/* 00001A80 001548 22520001 */  addi    $18, $18, 1
/* 00001A84 00154C 022B8822 */  sub     $17, $17, $11
.L00001A88:
/* 00001A88 001550 4894D800 */  mtc2    $20, $v27[0]
/* 00001A8C 001554 4B18DEC6 */  vmudn   $v27, $v27, $v24[0]
/* 00001A90 001558 0C000675 */  jal     func_000019D4
/* 00001A94 00155C 4814D800 */   mfc2   $20, $v27[0]
.L00001A98:
/* 00001A98 001560 1180001D */  beqz    $12, .L00001B10
/* 00001A9C 001564 C838090F */   lsv    $v24[2], 0x1E($1)
/* 00001AA0 001568 4898D100 */  mtc2    $24, $v26[2]
/* 00001AA4 00156C 4B38D6C6 */  vmudn   $v27, $v26, $v24[1]
/* 00001AA8 001570 4B7DFE4F */  vmadh   $v25, $v31, $v29[3]
/* 00001AAC 001574 EBDB0901 */  ssv     $v27[2], 0x2($30)
/* 00001AB0 001578 EBD90900 */  ssv     $v25[2], 0x0($30)
/* 00001AB4 00157C 8FCB0000 */  lw      $11, ($30)
/* 00001AB8 001580 8C1103C0 */  lw      $17, 0x3c0($zero)
/* 00001ABC 001584 33120001 */  andi    $18, $24, 1
/* 00001AC0 001588 12400005 */  beqz    $18, .L00001AD8
/* 00001AC4 00158C 00AB6020 */   add    $12, $5, $11
/* 00001AC8 001590 940B03F6 */  lhu     $11, 0x3f6($zero)
/* 00001ACC 001594 4B3DD691 */  vsub    $v26, $v26, $v29[1]
/* 00001AD0 001598 018B6022 */  sub     $12, $12, $11
/* 00001AD4 00159C 022B8822 */  sub     $17, $17, $11
.L00001AD8:
/* 00001AD8 0015A0 C8380A0E */  lsv     $v24[4], 0x1C($1)
/* 00001ADC 0015A4 22520001 */  addi    $18, $18, 1
/* 00001AE0 0015A8 4B58C6D1 */  vsub    $v27, $v24, $v24[2]
/* 00001AE4 0015AC 4BDEDEC5 */  vmudm   $v27, $v27, $v30[6]
/* 00001AE8 0015B0 4B3BC651 */  vsub    $v25, $v24, $v27[1]
/* 00001AEC 0015B4 4813C800 */  mfc2    $19, $v25[0]
/* 00001AF0 0015B8 4B18D646 */  vmudn   $v25, $v26, $v24[0]
/* 00001AF4 0015BC 4B3DDE8E */  vmadn   $v26, $v27, $v29[1]
/* 00001AF8 0015C0 0C000675 */  jal     func_000019D4
/* 00001AFC 0015C4 4814D100 */   mfc2   $20, $v26[2]
/* 00001B00 0015C8 35910000 */  move    $17, $12
/* 00001B04 0015CC 4814C900 */  mfc2    $20, $v25[2]
/* 00001B08 0015D0 0C000675 */  jal     func_000019D4
/* 00001B0C 0015D4 4813D900 */   mfc2   $19, $v27[2]
.L00001B10:
/* 00001B10 0015D8 1F000006 */  bgtz    $24, .L00001B2C
/* 00001B14 0015DC 00641822 */   sub    $3, $3, $4
/* 00001B18 0015E0 8C0B03FC */  lw      $11, 0x3fc($zero)
/* 00001B1C 0015E4 22F70008 */  addi    $23, $23, 8
/* 00001B20 0015E8 AEEBFFF8 */  sw      $11, -8($23)
/* 00001B24 0015EC 080006D0 */  j       .L00001B40
/* 00001B28 0015F0 EAFD147F */   slv    $v29[8], -0x4($23)
.L00001B2C:
/* 00001B2C 0015F4 34B10000 */  move    $17, $5
/* 00001B30 0015F8 941303E2 */  lhu     $19, 0x3e2($zero)
/* 00001B34 0015FC 37120000 */  move    $18, $24
/* 00001B38 001600 0C000675 */  jal     func_000019D4
/* 00001B3C 001604 20140000 */   li     $20, 0
.L00001B40:
/* 00001B40 001608 1C600097 */  bgtz    $3, func_00001DA0
/* 00001B44 00160C 00A62820 */   add    $5, $5, $6
/* 00001B48 001610 00035822 */  neg     $11, $3
/* 00001B4C 001614 488BD800 */  mtc2    $11, $v27[0]
/* 00001B50 001618 940B03F4 */  lhu     $11, 0x3f4($zero)
/* 00001B54 00161C 8C0503C0 */  lw      $5, 0x3c0($zero)
/* 00001B58 001620 4B38DEC6 */  vmudn   $v27, $v27, $v24[1]
/* 00001B5C 001624 4B7DFE4F */  vmadh   $v25, $v31, $v29[3]
/* 00001B60 001628 00AB2820 */  add     $5, $5, $11
/* 00001B64 00162C EBDB0801 */  ssv     $v27[0], 0x2($30)
/* 00001B68 001630 940C040C */  lhu     $12, 0x40c($zero)
/* 00001B6C 001634 EBD90800 */  ssv     $v25[0], 0x0($30)
/* 00001B70 001638 8FCB0000 */  lw      $11, ($30)
/* 00001B74 00163C 006C1820 */  add     $3, $3, $12
/* 00001B78 001640 08000768 */  j       func_00001DA0
/* 00001B7C 001644 00AB2820 */   add    $5, $5, $11

func_00001B80:
/* 00001B80 001648 8F7804BC */  lw      $24, 0x4bc($27)
/* 00001B84 00164C 201F16FC */  li      $ra, func_000016FC
/* 00001B88 001650 08000455 */  j       segmented_to_physical
/* 00001B8C 001654 201403B0 */   li     $20, data_03B0

G_OBJ_RECTANGLE_R_handler:
/* 00001B90 001658 0C000455 */  jal     segmented_to_physical
/* 00001B94 00165C 20130017 */   li     $19, 0x18-1
/* 00001B98 001660 0C0007F6 */  jal     dma_read_write
/* 00001B9C 001664 201403C0 */   li     $20, data_03C0
/* 00001BA0 001668 200C1B9C */  li      $12, 0x1b9c     // TODO IMEM label
/* 00001BA4 00166C 080007ED */  j       load_overlay_and_enter
/* 00001BA8 001670 200B0314 */   li     $11, Overlay2Info

/* 00001BAC 001674 00000000 */  nop     

func_00001BB0:
/* 00001BB0 001678 200C1638 */  li      $12, .L00001638
/* 00001BB4 00167C 080007ED */  j       load_overlay_and_enter
/* 00001BB8 001680 200B0314 */   li     $11, Overlay2Info

func_00001BBC:
/* 00001BBC 001684 C82D080D */  lsv     $v13[0], 0x1A($1)
/* 00001BC0 001688 C831080C */  lsv     $v17[0], 0x18($1)
/* 00001BC4 00168C 4A1286C6 */  vmudn   $v27, $v16, $v18
/* 00001BC8 001690 4B3D76CE */  vmadn   $v27, $v14, $v29[1]
/* 00001BCC 001694 4B3D7ECF */  vmadh   $v27, $v15, $v29[1]
/* 00001BD0 001698 4B1F8ECD */  vmadm   $v27, $v17, $v31[0]
/* 00001BD4 00169C 4B1F6A0C */  vmadl   $v8, $v13, $v31[0]
/* 00001BD8 0016A0 4BAA4351 */  vsub    $v13, $v8, $v10[5]
/* 00001BDC 0016A4 4BAA4390 */  vadd    $v14, $v8, $v10[5]
/* 00001BE0 0016A8 4B7D46E3 */  vge     $v27, $v8, $v29[3]
/* 00001BE4 0016AC 4A0E4227 */  vmrg    $v8, $v8, $v14
/* 00001BE8 0016B0 4B7D6EE3 */  vge     $v27, $v13, $v29[3]
/* 00001BEC 0016B4 4A086A27 */  vmrg    $v8, $v13, $v8
/* 00001BF0 0016B8 4B224AF3 */  vmov    $v11[1], $v2[1]
/* 00001BF4 0016BC 4B2C5A85 */  vmudm   $v10, $v11, $v12[1]
/* 00001BF8 0016C0 4BDF5286 */  vmudn   $v10, $v10, $v31[6]
/* 00001BFC 0016C4 E82A1007 */  slv     $v10[0], 0x1C($1)
/* 00001C00 0016C8 E828081B */  ssv     $v8[0], 0x36($1)
/* 00001C04 0016CC 8C1803C0 */  lw      $24, 0x3c0($zero)
/* 00001C08 0016D0 4B3D56C6 */  vmudn   $v27, $v10, $v29[1]
/* 00001C0C 0016D4 0C000455 */  jal     segmented_to_physical
/* 00001C10 0016D8 4B2A408E */   vmadn  $v2, $v8, $v10[1]
/* 00001C14 0016DC AC1803C0 */  sw      $24, 0x3c0($zero)
/* 00001C18 0016E0 4B7DFB4F */  vmadh   $v13, $v31, $v29[3]
/* 00001C1C 0016E4 EBC20801 */  ssv     $v2[0], 0x2($30)
/* 00001C20 0016E8 EBCD0800 */  ssv     $v13[0], 0x0($30)
/* 00001C24 0016EC 8FC50000 */  lw      $5, ($30)
/* 00001C28 0016F0 00B82820 */  add     $5, $5, $24
/* 00001C2C 0016F4 4B2A3A05 */  vmudm   $v8, $v7, $v10[1]
/* 00001C30 0016F8 4B7DF88E */  vmadn   $v2, $v31, $v29[3]
/* 00001C34 0016FC E8280816 */  ssv     $v8[0], 0x2C($1)
/* 00001C38 001700 E8280918 */  ssv     $v8[2], 0x30($1)
/* 00001C3C 001704 E8220817 */  ssv     $v2[0], 0x2E($1)
/* 00001C40 001708 E8220919 */  ssv     $v2[2], 0x32($1)
/* 00001C44 00170C 10400003 */  beqz    $2, .L00001C54
/* 00001C48 001710 4B0C58A8 */   vand   $v2, $v11, $v12[0]
/* 00001C4C 001714 4B7F16C6 */  vmudn   $v27, $v2, $v31[3]
/* 00001C50 001718 4B7F488E */  vmadn   $v2, $v9, $v31[3]
.L00001C54:
/* 00001C54 00171C 940B03F6 */  lhu     $11, 0x3f6($zero)
/* 00001C58 001720 940C03E2 */  lhu     $12, 0x3e2($zero)
/* 00001C5C 001724 3C02FD10 */  lui     $2, 0xfd10
/* 00001C60 001728 000B5842 */  srl     $11, $11, 1
/* 00001C64 00172C 216BFFFF */  addi    $11, $11, -1
/* 00001C68 001730 004B1025 */  or      $2, $2, $11
/* 00001C6C 001734 AC0203F8 */  sw      $2, 0x3f8($zero)
/* 00001C70 001738 3C07F510 */  lui     $7, 0xf510
/* 00001C74 00173C 000C6240 */  sll     $12, $12, 9
/* 00001C78 001740 00EC3825 */  or      $7, $7, $12
/* 00001C7C 001744 AC0703FC */  sw      $7, 0x3fc($zero)
/* 00001C80 001748 AEE70000 */  sw      $7, ($23)
/* 00001C84 00174C EAFD1401 */  slv     $v29[8], 0x4($23)
/* 00001C88 001750 AEE70008 */  sw      $7, 8($23)
/* 00001C8C 001754 900B03C6 */  lbu     $11, 0x3c6($zero)
/* 00001C90 001758 900C03C7 */  lbu     $12, 0x3c7($zero)
/* 00001C94 00175C 000B5940 */  sll     $11, $11, 5
/* 00001C98 001760 000C60C0 */  sll     $12, $12, 3
/* 00001C9C 001764 016C5825 */  or      $11, $11, $12
/* 00001CA0 001768 A2EB0009 */  sb      $11, 9($23)
/* 00001CA4 00176C 900B03C9 */  lbu     $11, 0x3c9($zero)
/* 00001CA8 001770 8C0C0264 */  lw      $12, data_0264
/* 00001CAC 001774 000B5D00 */  sll     $11, $11, 0x14
/* 00001CB0 001778 016C5825 */  or      $11, $11, $12
/* 00001CB4 00177C AEEB000C */  sw      $11, 0xc($23)
/* 00001CB8 001780 EAFD1204 */  slv     $v29[4], 0x10($23)
/* 00001CBC 001784 AEE00014 */  sw      $zero, 0x14($23)
/* 00001CC0 001788 22F70018 */  addi    $23, $23, 0x18
/* 00001CC4 00178C 940303BA */  lhu     $3, 0x3ba($zero)
/* 00001CC8 001790 940B040E */  lhu     $11, 0x40e($zero)
/* 00001CCC 001794 84040400 */  lh      $4, 0x400($zero)
/* 00001CD0 001798 8C0203EC */  lw      $2, 0x3ec($zero)
/* 00001CD4 00179C 8C0F03E8 */  lw      $15, 0x3e8($zero)
/* 00001CD8 0017A0 940E03DE */  lhu     $14, 0x3de($zero)
/* 00001CDC 0017A4 00031882 */  srl     $3, $3, 2
/* 00001CE0 0017A8 A403040C */  sh      $3, 0x40c($zero)
/* 00001CE4 0017AC 006B1822 */  sub     $3, $3, $11
/* 00001CE8 0017B0 8C060404 */  lw      $6, 0x404($zero)
/* 00001CEC 0017B4 940703D8 */  lhu     $7, 0x3d8($zero)
/* 00001CF0 0017B8 940803DA */  lhu     $8, 0x3da($zero)
/* 00001CF4 0017BC 940903DC */  lhu     $9, 0x3dc($zero)
/* 00001CF8 0017C0 00073B00 */  sll     $7, $7, 0xc
/* 00001CFC 0017C4 00084300 */  sll     $8, $8, 0xc
.L00001D00:
/* 00001D00 0017C8 00026A82 */  srl     $13, $2, 0xa
/* 00001D04 0017CC 1DA00013 */  bgtz    $13, .L00001D54
/* 00001D08 0017D0 00000000 */   nop    
/* 00001D0C 0017D4 00641822 */  sub     $3, $3, $4
/* 00001D10 0017D8 1C600033 */  bgtz    $3, .L00001DE0
/* 00001D14 0017DC 00A62820 */   add    $5, $5, $6
/* 00001D18 0017E0 C827080F */  lsv     $v7[0], 0x1E($1)
/* 00001D1C 0017E4 00035822 */  neg     $11, $3
/* 00001D20 0017E8 488B3900 */  mtc2    $11, $v7[2]
/* 00001D24 0017EC 8C0503C0 */  lw      $5, 0x3c0($zero)
/* 00001D28 0017F0 4B2739C6 */  vmudn   $v7, $v7, $v7[1]
/* 00001D2C 0017F4 4B7DFA0F */  vmadh   $v8, $v31, $v29[3]
/* 00001D30 0017F8 EBC70801 */  ssv     $v7[0], 0x2($30)
/* 00001D34 0017FC EBC80800 */  ssv     $v8[0], 0x0($30)
/* 00001D38 001800 8FCB0000 */  lw      $11, ($30)
/* 00001D3C 001804 940C03F4 */  lhu     $12, 0x3f4($zero)
/* 00001D40 001808 941F040C */  lhu     $ra, 0x40c($zero)
/* 00001D44 00180C 00AB2820 */  add     $5, $5, $11
/* 00001D48 001810 00AC2820 */  add     $5, $5, $12
/* 00001D4C 001814 08000778 */  j       .L00001DE0
/* 00001D50 001818 007F1820 */   add    $3, $3, $ra
.L00001D54:
/* 00001D54 00181C 01CD7022 */  sub     $14, $14, $13
/* 00001D58 001820 05C1000F */  bgez    $14, .L00001D98
/* 00001D5C 001824 304203FF */   andi   $2, $2, 0x3ff
/* 00001D60 001828 488ED800 */  mtc2    $14, $v27[0]
/* 00001D64 00182C 4B1B21C6 */  vmudn   $v7, $v4, $v27[0]
/* 00001D68 001830 4B7DFA0F */  vmadh   $v8, $v31, $v29[3]
/* 00001D6C 001834 940C0402 */  lhu     $12, 0x402($zero)
/* 00001D70 001838 EBC80C00 */  ssv     $v8[8], 0x0($30)
/* 00001D74 00183C EBC70C01 */  ssv     $v7[8], 0x2($30)
/* 00001D78 001840 8FCB0000 */  lw      $11, ($30)
/* 00001D7C 001844 20840001 */  addi    $4, $4, 1
/* 00001D80 001848 000B5A83 */  sra     $11, $11, 0xA
/* 00001D84 00184C 008B2020 */  add     $4, $4, $11
/* 00001D88 001850 008C5822 */  sub     $11, $4, $12
/* 00001D8C 001854 05600002 */  bltz    $11, .L00001D98
/* 00001D90 001858 01AE6820 */   add    $13, $13, $14
/* 00001D94 00185C 21840000 */  addi    $4, $12, 0
.L00001D98:
/* 00001D98 001860 08000688 */  j       func_00001A20
/* 00001D9C 001864 012D5020 */   add    $10, $9, $13

func_00001DA0:
/* 00001DA0 001868 EAFD0400 */  sbv     $v29[8], 0x0($23)
/* 00001DA4 00186C 000A5880 */  sll     $11, $10, 2
/* 00001DA8 001870 01685825 */  or      $11, $11, $8
/* 00001DAC 001874 AEEB0008 */  sw      $11, 8($23)
/* 00001DB0 001878 EAFD0688 */  sbv     $v29[13], 0x8($23)
/* 00001DB4 00187C 00095880 */  sll     $11, $9, 2
/* 00001DB8 001880 01675825 */  or      $11, $11, $7
/* 00001DBC 001884 AEEB000C */  sw      $11, 0xC($23)
/* 00001DC0 001888 EAE20808 */  ssv     $v2[0], 0x10($23)
/* 00001DC4 00188C EAE30809 */  ssv     $v3[0], 0x12($23)
/* 00001DC8 001890 EAE4080A */  ssv     $v4[0], 0x14($23)
/* 00001DCC 001894 EAE40C0B */  ssv     $v4[8], 0x16($23)
/* 00001DD0 001898 0C000591 */  jal     func_00001644
/* 00001DD4 00189C 26F70018 */   addiu  $23, $23, 0x18
/* 00001DD8 0018A0 19C0FCF2 */  blez    $14, next_cmd
/* 00001DDC 0018A4 21490000 */   addi   $9, $10, 0
.L00001DE0:
/* 00001DE0 0018A8 004F1020 */  add     $2, $2, $15
/* 00001DE4 0018AC 84040402 */  lh      $4, 0x402($zero)
/* 00001DE8 0018B0 8C060408 */  lw      $6, 0x408($zero)
/* 00001DEC 0018B4 08000740 */  j       .L00001D00
/* 00001DF0 0018B8 4A0318EC */   vclr   $v3

.align 8
Overlay3End:

.close
