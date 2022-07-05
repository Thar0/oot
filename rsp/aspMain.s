.rsp
#include "rsp.inc"
#include "rsp.h"
#include "rdp.h"

// data macros
.macro jumpTableEntry, addr
    .dh addr & 0xFFFF
.endmacro

// scalar macros
.macro li, reg, imm
    addi reg, $zero, imm
.endmacro

.macro move, dst, src
    addi dst, src, 0
.endmacro

// vector macros
.macro vclr, dst
    vxor dst, dst, dst
.endmacro

.create DATA_FILE, DMEM_START

// 0x0000
data_0000:
.dh 0x0000, 0x0001, 0x0002, 0xFFFF, 0x0020, 0x0800, 0x7FFF, 0x4000

// 0x0010
dispatchTable:
    jumpTableEntry cmd_UNK0
    jumpTableEntry cmd_ADPCM
    jumpTableEntry cmd_CLEARBUFF
    jumpTableEntry cmd_UNK3
    jumpTableEntry cmd_ADDMIXER
    jumpTableEntry cmd_RESAMPLE
    jumpTableEntry cmd_RESAMPLE_ZOH
    jumpTableEntry cmd_FILTER
    jumpTableEntry cmd_SETBUFF
    jumpTableEntry cmd_DUPLICATE
    jumpTableEntry cmd_DMEMMOVE
    jumpTableEntry cmd_LOADADPCM
    jumpTableEntry cmd_MIXER
    jumpTableEntry cmd_INTERLEAVE
    jumpTableEntry cmd_HILOGAIN
    jumpTableEntry cmd_SETLOOP
    jumpTableEntry cmd_UNK16
    jumpTableEntry cmd_INTERL
    jumpTableEntry cmd_ENVSETUP1
    jumpTableEntry cmd_ENVMIXER
    jumpTableEntry cmd_LOADBUFF
    jumpTableEntry cmd_SAVEBUFF
    jumpTableEntry cmd_ENVSETUP2
    jumpTableEntry cmd_S8DEC
    //! Audio driver code claims there should be a command at index 19,
    //! but there is no entry for it?

// 0x0040
data_0040:
.dh 0xF000, 0x0F00, 0x00F0, 0x000F
.dh 0x0001, 0x0010, 0x0100, 0x1000
.dh 0xC000, 0x3000, 0x0C00, 0x0300
.dh 0x0001, 0x0004, 0x0010, 0x0040

// 0x0060
// related to resampling
data_0060:
.dh 0x0002, 0x0004, 0x0006, 0x0008, 0x000A, 0x000C, 0x000E, 0x0010
.dh 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001
.dh 0x0000, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0100, 0x0200
.dh 0x0001, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000, 0x0000, 0x0000
.dh 0x0000, 0x0001, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000, 0x0000
.dh 0x0000, 0x0000, 0x0001, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000
.dh 0x0000, 0x0000, 0x0000, 0x0001, 0x0000, 0x0000, 0x0000, 0x0001
.dh 0x2000, 0x4000, 0x6000, 0x8000, 0xA000, 0xC000, 0xE000, 0xFFFF

// 0x00E0
// unused?
data_00E0:
.dh 0x0C39, 0x66AD, 0x0D46, 0xFFDF, 0x0B39, 0x6696, 0x0E5F, 0xFFD8
.dh 0x0A44, 0x6669, 0x0F83, 0xFFD0, 0x095A, 0x6626, 0x10B4, 0xFFC8
.dh 0x087D, 0x65CD, 0x11F0, 0xFFBF, 0x07AB, 0x655E, 0x1338, 0xFFB6
.dh 0x06E4, 0x64D9, 0x148C, 0xFFAC, 0x0628, 0x643F, 0x15EB, 0xFFA1
.dh 0x0577, 0x638F, 0x1756, 0xFF96, 0x04D1, 0x62CB, 0x18CB, 0xFF8A
.dh 0x0435, 0x61F3, 0x1A4C, 0xFF7E, 0x03A4, 0x6106, 0x1BD7, 0xFF71
.dh 0x031C, 0x6007, 0x1D6C, 0xFF64, 0x029F, 0x5EF5, 0x1F0B, 0xFF56
.dh 0x022A, 0x5DD0, 0x20B3, 0xFF48, 0x01BE, 0x5C9A, 0x2264, 0xFF3A
.dh 0x015B, 0x5B53, 0x241E, 0xFF2C, 0x0101, 0x59FC, 0x25E0, 0xFF1E
.dh 0x00AE, 0x5896, 0x27A9, 0xFF10, 0x0063, 0x5720, 0x297A, 0xFF02
.dh 0x001F, 0x559D, 0x2B50, 0xFEF4, 0xFFE2, 0x540D, 0x2D2C, 0xFEE8
.dh 0xFFAC, 0x5270, 0x2F0D, 0xFEDB, 0xFF7C, 0x50C7, 0x30F3, 0xFED0
.dh 0xFF53, 0x4F14, 0x32DC, 0xFEC6, 0xFF2E, 0x4D57, 0x34C8, 0xFEBD
.dh 0xFF0F, 0x4B91, 0x36B6, 0xFEB6, 0xFEF5, 0x49C2, 0x38A5, 0xFEB0
.dh 0xFEDF, 0x47ED, 0x3A95, 0xFEAC, 0xFECE, 0x4611, 0x3C85, 0xFEAB
.dh 0xFEC0, 0x4430, 0x3E74, 0xFEAC, 0xFEB6, 0x424A, 0x4060, 0xFEAF
.dh 0xFEAF, 0x4060, 0x424A, 0xFEB6, 0xFEAC, 0x3E74, 0x4430, 0xFEC0
.dh 0xFEAB, 0x3C85, 0x4611, 0xFECE, 0xFEAC, 0x3A95, 0x47ED, 0xFEDF
.dh 0xFEB0, 0x38A5, 0x49C2, 0xFEF5, 0xFEB6, 0x36B6, 0x4B91, 0xFF0F
.dh 0xFEBD, 0x34C8, 0x4D57, 0xFF2E, 0xFEC6, 0x32DC, 0x4F14, 0xFF53
.dh 0xFED0, 0x30F3, 0x50C7, 0xFF7C, 0xFEDB, 0x2F0D, 0x5270, 0xFFAC
.dh 0xFEE8, 0x2D2C, 0x540D, 0xFFE2, 0xFEF4, 0x2B50, 0x559D, 0x001F
.dh 0xFF02, 0x297A, 0x5720, 0x0063, 0xFF10, 0x27A9, 0x5896, 0x00AE
.dh 0xFF1E, 0x25E0, 0x59FC, 0x0101, 0xFF2C, 0x241E, 0x5B53, 0x015B
.dh 0xFF3A, 0x2264, 0x5C9A, 0x01BE, 0xFF48, 0x20B3, 0x5DD0, 0x022A
.dh 0xFF56, 0x1F0B, 0x5EF5, 0x029F, 0xFF64, 0x1D6C, 0x6007, 0x031C
.dh 0xFF71, 0x1BD7, 0x6106, 0x03A4, 0xFF7E, 0x1A4C, 0x61F3, 0x0435
.dh 0xFF8A, 0x18CB, 0x62CB, 0x04D1, 0xFF96, 0x1756, 0x638F, 0x0577
.dh 0xFFA1, 0x15EB, 0x643F, 0x0628, 0xFFAC, 0x148C, 0x64D9, 0x06E4
.dh 0xFFB6, 0x1338, 0x655E, 0x07AB, 0xFFBF, 0x11F0, 0x65CD, 0x087D
.dh 0xFFC8, 0x10B4, 0x6626, 0x095A, 0xFFD0, 0x0F83, 0x6669, 0x0A44
.dh 0xFFD8, 0x0E5F, 0x6696, 0x0B39, 0xFFDF, 0x0D46, 0x66AD, 0x0C39

audio_in_buf    equ 0x00 // 0x2E0
audio_out_buf   equ 0x02 // 0x2E2
audio_count     equ 0x04 // 0x2E4
audio_loop_addr equ 0x08 // 0x2E8
// 0x02E0
audioStruct:
    .skip 0x10

// 0x02F0
nextTaskEntry:
    .skip 0x40

// 0x0330
adpcmTable:
    .skip 0x90

// buffer spaces managed by the CPU
// 0x03C0
data_03C0:
    .skip 0xBF0
/*
#define DEFAULT_LEN_1CH     0x1A0
#define DEFAULT_LEN_2CH     0x340

#define DMEM_TEMP                   0x3C0
#define DMEM_WET_TEMP               0x3E0
#define DMEM_UNCOMPRESSED_NOTE      0x580
#define DMEM_NOTE_PAN_TEMP          0x5C0
#define DMEM_WET_SCRATCH            0x720 // = DMEM_WET_TEMP + DEFAULT_LEN_2CH
#define DMEM_SCRATCH2               0x760 // = DMEM_TEMP + DEFAULT_LEN_2CH + a bit more
#define DMEM_COMPRESSED_ADPCM_DATA  0x940 // = DMEM_LEFT_CH
#define DMEM_LEFT_CH                0x940
#define DMEM_RIGHT_CH               0xAE0
#define DMEM_WET_LEFT_CH            0xC80
#define DMEM_WET_RIGHT_CH           0xE20 // = DMEM_WET_LEFT_CH + DEFAULT_LEN_1CH
*/

// temporary area
// 0x0FB0
tmpData:
    .skip 0x50

.if . > DMEM_END
    .error "Not enough room in DMEM"
.endif

.close

A_CONTINUE    equ (0 << 0)
A_INIT        equ (1 << 0)
A_LOOP        equ (1 << 1)
A_ADPCM_SHORT equ (1 << 2)

// Frame size
ADPCMFSIZE equ 0x10 * 2

.create CODE_FILE, IMEM_START_VIRT

// Global Registers
vzero equ v0

taskDataPtr    equ $28
taskDataSize   equ $27
tmpDataPtr     equ $23
audioStructPtr equ $24

cmd_buf_remaining equ $30
cmd_ptr           equ $29
cmd_w0            equ $26     // Command word 1
cmd_w1            equ $25     // Command word 2

audio_entry:
    ori     $10, $zero, OSTask_addr
    lw      $2, OS_TASK_OFF_UDATA($10)
    lw      $3, OS_TASK_OFF_UDATA_SZ($10)
    mtc0    $zero, SP_SEMAPHORE             // release semaphore
    jal     dma_read_start
     li     $1, 0
    li      audioStructPtr, audioStruct
    li      tmpDataPtr, tmpData
    lw      taskDataPtr, OS_TASK_OFF_DATA($10)      // task_data
    lw      taskDataSize, OS_TASK_OFF_DATA_SZ($10)  // task_data_size
    mfc0    $5, DPC_STATUS
    andi    $4, $5, DPC_STATUS_XBUS_DMEM_DMA
    beqz    $4, no_dma
     andi   $4, $5, DPC_STATUS_DMA_BUSY
    beqz    $4, no_dma
     nop
dpc_dma_busy:
    mfc0    $4, DPC_STATUS
    andi    $4, $4, DPC_STATUS_DMA_BUSY
    bgtz    $4, dpc_dma_busy
no_dma:
     nop
    jal     load_acmd_list
     nop

dispatch_cmd:
    lw      cmd_w0, 0(cmd_ptr)      // first cmd word
    lw      cmd_w1, 4(cmd_ptr)      // second cmd word
    srl     $1, cmd_w0, 0x17        // cmd byte << 1
    andi    $1, $1, 0xfe
    addi    taskDataPtr, taskDataPtr, 8                 // increment task_data
    addi    taskDataSize, taskDataSize, -8              // decrement task_size
    addi    cmd_ptr, cmd_ptr, 8                         // increment to next command
    addi    cmd_buf_remaining, cmd_buf_remaining, -8    // decrement current task buffer size
    add     $2, $zero, $1
    lh      $2, (dispatchTable)($2) // jump to command handler
    jr      $2
     nop
    break

next_cmd:
    bgtz    cmd_buf_remaining, dispatch_cmd
     nop
    blez    taskDataSize, task_done          // task size less or equal to 0 -> no task
     nop
    jal     load_acmd_list
     nop
    j       dispatch_cmd
     nop
task_done:
    ori     $1, $zero, SP_SET_SIG2  // set signal 2 (task done)
    mtc0    $1, SP_STATUS
    break
    nop
forever:
    b       forever
     nop

// DMAs the acmd list to DMEM, placing it at nextTaskEntry
load_acmd_list:
    addi    $5, $ra, 0              // save $ra in $5
    add     $2, $zero, taskDataPtr
    move    $3, taskDataSize
    addi    $4, $3, -0x40
    blez    $4, @@L1                // branch forward if there is less than 0x40 bytes worth of commands left
     li     $1, nextTaskEntry
    li      $3, 0x40                // load full 0x40
@@L1:
    move    cmd_buf_remaining, $3
    jal     dma_read_start
     addi   $3, $3, -1
    jr      $5                      // return with stored $ra
     li     cmd_ptr, nextTaskEntry

dma_read:
    @@mem_addr  equ $1
    @@dram_addr equ $2
    @@dma_len   equ $3

    mfc0    $4, SP_SEMAPHORE        // acquire semaphore
    bnez    $4, dma_read
     nop
@@dma_not_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_not_full
     nop
    mtc0    @@mem_addr, SP_MEM_ADDR
    mtc0    @@dram_addr, SP_DRAM_ADDR
    mtc0    @@dma_len, SP_RD_LEN
    jr      $ra
     nop

dma_write:
    @@mem_addr  equ $1
    @@dram_addr equ $2
    @@dma_len   equ $3

    mfc0    $4, SP_SEMAPHORE        // acquire semaphore
    bnez    $4, dma_write
     nop
@@dma_not_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_not_full
     nop
    mtc0    @@mem_addr, SP_MEM_ADDR
    mtc0    @@dram_addr, SP_DRAM_ADDR
    mtc0    @@dma_len, SP_WR_LEN
    jr      $ra
     nop

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Empty           | DMEM                            | Empty                           | Count                           |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Clears the DMEM buffer pointed to by `DMEM`, `count` should be a multiple of 16/0x10 bytes
 */
cmd_CLEARBUFF:
    @@mem_addr equ $2
    @@count    equ $3

    andi    @@count, cmd_w1, 0xffff     // load count
    beqz    @@count, next_cmd           // 0 count, leave early
     andi   @@mem_addr, cmd_w0, 0xffff  // load dmem addr
    vclr    $vzero
    addi    @@count, @@count, -0x10
@@loop:
    sdv     $vzero[0], 0x0(@@mem_addr)  // store vector 0 over the dmem buffer
    sdv     $vzero[0], 0x8(@@mem_addr)
    addi    @@mem_addr, @@mem_addr, 0x10
    bgtz    @@count, @@loop             // if count > 0 continue clearing
     addi   @@count, @@count, -0x10
    j       next_cmd
     nop

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Flags           | DMEM In                         | DMEM Out                        | Count                           |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Sets buffers used by ADPCM, RESAMPLE, RESAMPLE_ZOH, S8DEC commands
 */
cmd_SETBUFF:
    @@mem_addr equ $2

    srl     @@mem_addr, cmd_w1, 0x10
    sh      cmd_w0, (audio_in_buf)(audioStructPtr)      // store dmem in
    sh      @@mem_addr, (audio_out_buf)(audioStructPtr) // store dmem out
    j       next_cmd
     sh     cmd_w1, (audio_count)(audioStructPtr)       // store count

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Count           | DMEM Out                        | Left                            | Right                           |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Interleaves the two channels pointed to by `Left` and `Right` DMEM addresses into stereo (L,R,L,R,L,R,L,R)
 *   Result is placed at `DMEM Out`
 */
cmd_INTERLEAVE:
    @@count    equ $1
    @@dmem_out equ $4
    @@left_in  equ $2
    @@right_in equ $3

    @@left  equ $v1
    @@right equ $v2

    andi    @@dmem_out, cmd_w0, 0xffff
    srl     @@count, cmd_w0, 0xc
    andi    @@count, @@count, 0xff0             // count << 4
    andi    @@right_in, cmd_w1, 0xffff
    srl     @@left_in, cmd_w1, 0x10
@@loop:
    ldv     @@left[0], (@@left_in)              // load L
    ldv     @@right[0], (@@right_in)            // load R
    addi    @@count, @@count, -8
    addi    @@dmem_out, @@dmem_out, 0x10
    ssv     @@left[0], (0x00-0x10)(@@dmem_out)  //  left[0] -> out[0]
    ssv     @@left[2], (0x04-0x10)(@@dmem_out)  //  left[1] -> out[2]
    addi    @@left_in, @@left_in, 8
    ssv     @@left[4], (0x08-0x10)(@@dmem_out)  //  left[2] -> out[4]
    ssv     @@left[6], (0x0C-0x10)(@@dmem_out)  //  left[3] -> out[6]
    ssv     @@right[0], (0x02-0x10)(@@dmem_out) // right[0] -> out[1]
    addi    @@right_in, @@right_in, 8
    ssv     @@right[2], (0x06-0x10)(@@dmem_out) // right[1] -> out[3]
    ssv     @@right[4], (0x0A-0x10)(@@dmem_out) // right[2] -> out[5]
    bgtz    @@count, @@loop                     // until done
     ssv    @@right[6], (0x0E-0x10)(@@dmem_out) // right[3] -> out[7]
    j       next_cmd
     nop

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Empty           | DMEM In                         | DMEM Out                        | Count                           |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_DMEMMOVE:
    @@dmem_in  equ $2
    @@dmem_out equ $3
    @@count    equ $1

    andi    @@count, cmd_w1, 0xffff     // count
    beqz    @@count, next_cmd           // 0 count, leave early
     andi   @@dmem_in, cmd_w0, 0xffff
    srl     @@dmem_out, cmd_w1, 0x10
@@loop:
    ldv     $v1[0], 0x0(@@dmem_in)      // load from dmem in
    ldv     $v2[0], 0x8(@@dmem_in)
    addi    @@count, @@count, -0x10
    addi    @@dmem_in, @@dmem_in, 0x10
    sdv     $v1[0], 0x0(@@dmem_out)     // store to dmem out
    sdv     $v2[0], 0x8(@@dmem_out)
    bgtz    @@count, @@loop             // until done
     addi   @@dmem_out, @@dmem_out, 0x10
    j       next_cmd
     nop

/**
 * |                 |                                                 |                                                                 |
 * | Command         | Empty                                           | Address                                                         |
 * | 8               | 8                                               | 16                                                              |
 * | 31           24 | 23                                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_SETLOOP:
    @@loop_address equ $1

    sll     @@loop_address, cmd_w1, 8
    srl     @@loop_address, @@loop_address, 8                   // remove addr high byte
    j       next_cmd
     sw     @@loop_address, (audio_loop_addr)(audioStructPtr)   // store addr in audio struct

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Flags           | Gain                            | Address                                                         |
 * | 8               | 8               | 16                              | 32                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Flags:
 *   & 1 = Init
 *   & 2 = Loop
 *   & 4 = Short ADPCM / two bits per sample
 */

cmd_ADPCM:
    @@isShortADPCM  equ $8
    @@adpcmTablePtr equ $15
    @@count         equ $18
    @@outBuffer     equ $19
    @@inBuffer      equ $21
    @@flags         equ $1

/* 04001230 000230 C81F2000 */  lqv     $v31[0], (data_0000)($zero)
/* 04001234 000234 4A1BDEEC */  vclr    $v27
/* 04001238 000238 97150000 */  lhu     $21, (audio_in_buf)(audioStructPtr)
/* 0400123C 00023C 4A19CE6C */  vclr    $v25
/* 04001240 000240 4A18C62C */  vclr    $v24
/* 04001244 000244 22B40001 */  addi    $20, $21, 1
/* 04001248 000248 97130002 */  lhu     @@outBuffer, (audio_out_buf)(audioStructPtr)
/* 0400124C 00024C 4A0D6B6C */  vclr    $v13
/* 04001250 000250 4A0E73AC */  vclr    $v14
/* 04001254 000254 97120004 */  lhu     @@count, (audio_count)(audioStructPtr)  // load count
/* 04001258 000258 4A0F7BEC */  vclr    $v15
/* 0400125C 00025C 4A10842C */  vclr    $v16
/* 04001260 000260 00198A00 */  sll     $17, cmd_w1, 8
/* 04001264 000264 4A118C6C */  vclr    $v17
/* 04001268 000268 4A1294AC */  vclr    $v18
/* 0400126C 00026C 00118A02 */  srl     $17, $17, 8                 // remove addr high byte
/* 04001270 000270 4A139CEC */  vclr    $v19
/* 04001274 000274 EA7B2000 */  sqv     $v27[0], 0x00(@@outBuffer)
/* 04001278 000278 EA7B2001 */  sqv     $v27[0], 0x10(@@outBuffer)
/* 0400127C 00027C 20100040 */  li      $16, data_0040
/* 04001280 000280 200F0330 */  li      @@adpcmTablePtr, adpcmTable
/* 04001284 000284 001A0C02 */  srl     @@flags, cmd_w0, 0x10       // load flags
/* 04001288 000288 30280004 */  andi    @@isShortADPCM, @@flags, A_ADPCM_SHORT
/* 0400128C 00028C 1100000A */  beqz    @@isShortADPCM, @@L040012B8 // if regular ADPCM, skip
/* 04001290 000290 00000000 */   nop
/* 04001294 000294 200A0005 */  li      $10, 5                      // $10 = 5
/* 04001298 000298 2009000E */  li      $9, 0xe                     // $9 = 14
/* 0400129C 00029C CA191802 */  ldv     $v25[0], 0x10($16)          // $v25[0..7] = *($16 + 0x10..0x17)
/* 040012A0 0002A0 CA171803 */  ldv     $v23[0], 0x18($16)          // $v23[0..7] = *($16 + 0x18..0x1F)
/* 040012A4 0002A4 2210FFFF */  addi    $16, $16, -1                // $16--
/* 040012A8 0002A8 CA191C02 */  ldv     $v25[8], 0x10($16)          // $v25[8..15] = *($16 + 0x10..0x17)
/* 040012AC 0002AC 22100002 */  addi    $16, $16, 2                 // $16 += 2
/* 040012B0 0002B0 090004B4 */  j       @@L040012D0                 // step over regular ADPCM handling
/* 040012B4 0002B4 CA171C03 */   ldv    $v23[8], 0x18($16)          // $v23[8..15] = *($16 + 0x18..0x1F)
@@L040012B8:
/* 040012B8 0002B8 200A0009 */  li      $10, 9                      // $10 = 9
/* 040012BC 0002BC 2009000C */  li      $9, 0xc                     // $9 = 12
/* 040012C0 0002C0 CA191800 */  ldv     $v25[0], 0x0($16)           // $v25[0..7] = *($16 + 0..7)
/* 040012C4 0002C4 CA181C00 */  ldv     $v24[8], 0x0($16)           // $v24[8..15] = *($16 + 0..7)
/* 040012C8 0002C8 CA171801 */  ldv     $v23[0], 0x8($16)           // $v23[0..7] = *($16 + 8..15)
/* 040012CC 0002CC CA171C01 */  ldv     $v23[8], 0x8($16)           // $v23[8..15] = *($16 + 8..15)
@@L040012D0:
/* 040012D0 0002D0 001A0C02 */  srl     @@flags, cmd_w0, 0x10
/* 040012D4 0002D4 30210001 */  andi    @@flags, @@flags, A_INIT
/* 040012D8 0002D8 1C200008 */  bgtz    @@flags, @@L040012FC        // if (flags & 1) , skip
/* 040012DC 0002DC 001A0C02 */   srl    @@flags, cmd_w0, 0x10
/* 040012E0 0002E0 30210002 */  andi    @@flags, @@flags, A_LOOP
/* 040012E4 0002E4 10010002 */  beq     $zero, @@flags, @@L040012F0 // if !(flags & 2) , skip
/* 040012E8 0002E8 22220000 */   addi   $2, $17, 0                  // get from command addr
/* 040012EC 0002EC 8F020008 */  lw      $2, (audio_loop_addr)(audioStructPtr)   // or get from audio struct based on (flags & 2)
@@L040012F0:
/* 040012F0 0002F0 22610000 */  move    $1, @@outBuffer
/* 040012F4 0002F4 0D0006B5 */  jal     dma_read_start
/* 040012F8 0002F8 2003001F */   li     $3, ADPCMFSIZE - 1
@@L040012FC:
/* 040012FC 0002FC CA7B2001 */  lqv     $v27[0], 0x10(@@outBuffer)
/* 04001300 000300 22730020 */  addi    @@outBuffer, @@outBuffer, ADPCMFSIZE
/* 04001304 000304 12400077 */  beqz    @@count, @@L040014E4        // 0 count, leave early
/* 04001308 000308 CA811800 */   ldv    $v1[0], ($20)
/* 0400130C 00030C 92A10000 */  lbu     $1, ($21)                   // $1 = header
/* 04001310 000310 302B000F */  andi    $11, $1, 0xf                // $11 = optimalp = header & 0xF
/* 04001314 000314 000B5940 */  sll     $11, $11, 5                 // $11 <<= 5
/* 04001318 000318 4B01C8E8 */  vand    $v3, $v25, $v1[0]           // both ADPCM
/* 0400131C 00031C 016F6820 */  add     $13, $11, @@adpcmTablePtr
/* 04001320 000320 4A04212C */  vclr    $v4
/* 04001324 000324 4A0631AC */  vclr    $v6
/* 04001328 000328 15000004 */  bnez    @@isShortADPCM, @@L0400133C // if short ADPCM, skip
/* 0400132C 00032C 4B21C968 */   vand   $v5, $v25, $v1[1]           // only short ADPCM
/* 04001330 000330 4B21C128 */  vand    $v4, $v24, $v1[1]           // only regular ADPCM
/* 04001334 000334 4B41C968 */  vand    $v5, $v25, $v1[2]           // only regular ADPCM
/* 04001338 000338 4B61C1A8 */  vand    $v6, $v24, $v1[3]           // only regular ADPCM
@@L0400133C: // scale calculation
/* 0400133C 00033C 00017102 */  srl     $14, $1, 4                  // $14 = header >> 4
/* 04001340 000340 00091020 */  add     $2, $zero, $9               // $2 = $9 = (14 or 12)
/* 04001344 000344 004E7022 */  sub     $14, $2, $14                // $14 = $2 - $14 = (14 or 12) - (header >> 4)
/* 04001348 000348 21C2FFFF */  addi    $2, $14, -1                 // $2 = $14 - 1 = ((14 or 12) - 1) - (header >> 4)
/* 0400134C 00034C 34038000 */  ori     $3, $zero, 0x8000           // $3 = (1 << 0xF)
/* 04001350 000350 11C00002 */  beqz    $14, @@L0400135C            // if ((14 or 12) - (header >> 4) == 0)
/* 04001354 000354 2004FFFF */   li     $4, -1                      //     scale = -1
/* 04001358 000358 00432006 */  srlv    $4, $3, $2                  // else scale = 0x8000 >> $2 = 0x8000 >> ((14 or 12) - 1) - (header >> 4)
@@L0400135C:
/* 0400135C 00035C 4884B000 */  mtc2    $4, $v22[0]                 // $v22[0] = scale
/* 04001360 000360 C9B52000 */  lqv     $v21[0], ($13)
/* 04001364 000364 C9B42001 */  lqv     $v20[0], 0x10($13)
/* 04001368 000368 21ADFFFE */  addi    $13, $13, -2
/* 0400136C 00036C C9B32802 */  lrv     $v19[0], 0x20($13)
/* 04001370 000370 21ADFFFE */  addi    $13, $13, -2
/* 04001374 000374 C9B22802 */  lrv     $v18[0], 0x20($13)
/* 04001378 000378 21ADFFFE */  addi    $13, $13, -2
/* 0400137C 00037C C9B12802 */  lrv     $v17[0], 0x20($13)
/* 04001380 000380 21ADFFFE */  addi    $13, $13, -2
/* 04001384 000384 C9B02802 */  lrv     $v16[0], 0x20($13)
/* 04001388 000388 21ADFFFE */  addi    $13, $13, -2
/* 0400138C 00038C C9AF2802 */  lrv     $v15[0], 0x20($13)
/* 04001390 000390 21ADFFFE */  addi    $13, $13, -2
/* 04001394 000394 C9AE2802 */  lrv     $v14[0], 0x20($13)
/* 04001398 000398 21ADFFFE */  addi    $13, $13, -2
/* 0400139C 00039C C9AD2802 */  lrv     $v13[0], 0x20($13)
@@L040013A0:
/* 040013A0 0003A0 028AA020 */  add     $20, $20, $10
/* 040013A4 0003A4 4A171F86 */  vmudn   $v30, $v3, $v23
/* 040013A8 0003A8 02AAA820 */  add     $21, $21, $10
/* 040013AC 0003AC 4A17278E */  vmadn   $v30, $v4, $v23
/* 040013B0 0003B0 CA811800 */  ldv     $v1[0], ($20)
/* 040013B4 0003B4 4A172F46 */  vmudn   $v29, $v5, $v23
/* 040013B8 0003B8 92A10000 */  lbu     $1, ($21)
/* 040013BC 0003BC 4A17374E */  vmadn   $v29, $v6, $v23
/* 040013C0 0003C0 19C00003 */  blez    $14, @@L040013D0
/* 040013C4 0003C4 302B000F */   andi   $11, $1, 0xf
/* 040013C8 0003C8 4B16F785 */  vmudm   $v30, $v30, $v22[0]         // *= scale
/* 040013CC 0003CC 4B16EF45 */  vmudm   $v29, $v29, $v22[0]         // *= scale
@@L040013D0:
/* 040013D0 0003D0 000B5940 */  sll     $11, $11, 5
/* 040013D4 0003D4 4B01C8E8 */  vand    $v3, $v25, $v1[0]           // both ADPCM
/* 040013D8 0003D8 016F6820 */  add     $13, $11, @@adpcmTablePtr
/* 040013DC 0003DC 15000004 */  bnez    @@isShortADPCM, @@L040013F0 // if short ADPCM, skip
/* 040013E0 0003E0 4B21C968 */   vand   $v5, $v25, $v1[1]           // only short ADPCM
/* 040013E4 0003E4 4B21C128 */  vand    $v4, $v24, $v1[1]           // only regular ADPCM
/* 040013E8 0003E8 4B41C968 */  vand    $v5, $v25, $v1[2]           // only regular ADPCM
/* 040013EC 0003EC 4B61C1A8 */  vand    $v6, $v24, $v1[3]           // only regular ADPCM
@@L040013F0: // another scale calculation
/* 040013F0 0003F0 00017102 */  srl     $14, $1, 4                  // $14 = header >> 4
/* 040013F4 0003F4 4BDBA887 */  vmudh   $v2, $v21, $v27[6]
/* 040013F8 0003F8 00091020 */  add     $2, $zero, $9               // $2 = $9 = (14 or 12)
/* 040013FC 0003FC 4BFBA08F */  vmadh   $v2, $v20, $v27[7]
/* 04001400 000400 004E7022 */  sub     $14, $2, $14                // $14 = $2 - $14 = (14 or 12) - (header >> 4)
/* 04001404 000404 4B1E988F */  vmadh   $v2, $v19, $v30[0]
/* 04001408 000408 21C2FFFF */  addi    $2, $14, -1                 // $2 = $14 - 1 = ((14 or 12) - 1) - (header >> 4)
/* 0400140C 00040C 4B3E908F */  vmadh   $v2, $v18, $v30[1]
/* 04001410 000410 20030001 */  li      $3, 1                       // $3 = 1
/* 04001414 000414 4B5E888F */  vmadh   $v2, $v17, $v30[2]
/* 04001418 000418 00031BC0 */  sll     $3, $3, 0xf                 // $3 = $3 << 0xF = 1 << 0xF
/* 0400141C 00041C 4B7E808F */  vmadh   $v2, $v16, $v30[3]
/* 04001420 000420 11C00002 */  beqz    $14, @@L0400142C            // if ((14 or 12) - (header >> 4) == 0)
/* 04001424 000424 2004FFFF */   li     $4, -1                      //     scale = -1
/* 04001428 000428 00432006 */  srlv    $4, $3, $2                  // else scale = $3 >> $2 = 0x8000 >> ((14 or 12) - 1) - (header >> 4)
@@L0400142C:
/* 0400142C 00042C 4B9E7F0F */  vmadh   $v28, $v15, $v30[4]
/* 04001430 000430 4884B000 */  mtc2    $4, $v22[0]                 // $v22[0] = scale
/* 04001434 000434 4BBE708F */  vmadh   $v2, $v14, $v30[5]
/* 04001438 000438 4BDE688F */  vmadh   $v2, $v13, $v30[6]
/* 0400143C 00043C 4BBFF08F */  vmadh   $v2, $v30, $v31[5]
/* 04001440 000440 4B3C3E9D */  vsar    $v26, $v7, $v28[1]
/* 04001444 000444 4B1C3F1D */  vsar    $v28, $v7, $v28[0]
/* 04001448 000448 4B9FD086 */  vmudn   $v2, $v26, $v31[4]
/* 0400144C 00044C 4B9FE70F */  vmadh   $v28, $v28, $v31[4]
/* 04001450 000450 4B1D9887 */  vmudh   $v2, $v19, $v29[0]
/* 04001454 000454 21ACFFFE */  addi    $12, $13, -2
/* 04001458 000458 4B3D908F */  vmadh   $v2, $v18, $v29[1]
/* 0400145C 00045C C9932802 */  lrv     $v19[0], 0x20($12)
/* 04001460 000460 4B5D888F */  vmadh   $v2, $v17, $v29[2]
/* 04001464 000464 218CFFFE */  addi    $12, $12, -2
/* 04001468 000468 4B7D808F */  vmadh   $v2, $v16, $v29[3]
/* 0400146C 00046C C9922802 */  lrv     $v18[0], 0x20($12)
/* 04001470 000470 4B9D788F */  vmadh   $v2, $v15, $v29[4]
/* 04001474 000474 218CFFFE */  addi    $12, $12, -2
/* 04001478 000478 4BBD708F */  vmadh   $v2, $v14, $v29[5]
/* 0400147C 00047C C9912802 */  lrv     $v17[0], 0x20($12)
/* 04001480 000480 4BDD688F */  vmadh   $v2, $v13, $v29[6]
/* 04001484 000484 218CFFFE */  addi    $12, $12, -2
/* 04001488 000488 4BBFE88F */  vmadh   $v2, $v29, $v31[5]
/* 0400148C 00048C C9902802 */  lrv     $v16[0], 0x20($12)
/* 04001490 000490 4BDCA88F */  vmadh   $v2, $v21, $v28[6]
/* 04001494 000494 218CFFFE */  addi    $12, $12, -2
/* 04001498 000498 4BFCA08F */  vmadh   $v2, $v20, $v28[7]
/* 0400149C 00049C C98F2802 */  lrv     $v15[0], 0x20($12)
/* 040014A0 0004A0 4B3B3E9D */  vsar    $v26, $v7, $v27[1]
/* 040014A4 0004A4 218CFFFE */  addi    $12, $12, -2
/* 040014A8 0004A8 4B1B3EDD */  vsar    $v27, $v7, $v27[0]
/* 040014AC 0004AC C98E2802 */  lrv     $v14[0], 0x20($12)
/* 040014B0 0004B0 218CFFFE */  addi    $12, $12, -2
/* 040014B4 0004B4 C98D2802 */  lrv     $v13[0], 0x20($12)
/* 040014B8 0004B8 C9B52000 */  lqv     $v21[0], 0x00($13)
/* 040014BC 0004BC 4B9FD086 */  vmudn   $v2, $v26, $v31[4]
/* 040014C0 0004C0 C9B42001 */  lqv     $v20[0], 0x10($13)
/* 040014C4 0004C4 4B9FDECF */  vmadh   $v27, $v27, $v31[4]
/* 040014C8 0004C8 2252FFE0 */  addi    @@count, @@count, -ADPCMFSIZE
/* 040014CC 0004CC EA7C1800 */  sdv     $v28[0], 0x00(@@outBuffer)
/* 040014D0 0004D0 EA7C1C01 */  sdv     $v28[8], 0x08(@@outBuffer)
/* 040014D4 0004D4 EA7B1802 */  sdv     $v27[0], 0x10(@@outBuffer)
/* 040014D8 0004D8 EA7B1C03 */  sdv     $v27[8], 0x18(@@outBuffer)
/* 040014DC 0004DC 1E40FFB0 */  bgtz    @@count, @@L040013A0            // until done
/* 040014E0 0004E0 22730020 */   addi   @@outBuffer, @@outBuffer, ADPCMFSIZE
@@L040014E4:
/* 040014E4 0004E4 2261FFE0 */  addi    $1, @@outBuffer, -ADPCMFSIZE
/* 040014E8 0004E8 22220000 */  move    $2, $17
/* 040014EC 0004EC 090006B1 */  j       dma_write_and_nextcmd
/* 040014F0 0004F0 20030020 */   li     $3, ADPCMFSIZE

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Flags           | Pitch                           | Address                                                         |
 * | 8               | 8               | 16                              | 32                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_RESAMPLE:
    @@in_buf    equ $8
    @@count     equ $18
    @@dram_addr equ $2
    @@flags     equ $7

    @@dram_addr_save equ $22

/* 040014F4 0004F4 87080000 */  lh      @@in_buf, (audio_in_buf)(audioStructPtr)
/* 040014F8 0004F8 87130002 */  lh      $19, (audio_out_buf)(audioStructPtr)
/* 040014FC 0004FC 87120004 */  lh      @@count, (audio_count)(audioStructPtr)
/* 04001500 000500 00191200 */  sll     @@dram_addr, cmd_w1, 8
/* 04001504 000504 00021202 */  srl     @@dram_addr, @@dram_addr, 8         // remove addr high byte
/* 04001508 000508 22E10000 */  move    $1, tmpDataPtr
/* 0400150C 00050C 0002B020 */  add     @@dram_addr_save, $zero, @@dram_addr
/* 04001510 000510 2003001F */  li      $3, 0x20 - 1
/* 04001514 000514 001A3C02 */  srl     @@flags, cmd_w0, 0x10
/* 04001518 000518 30EA0001 */  andi    $10, @@flags, A_INIT
/* 0400151C 00051C 1D400005 */  bgtz    $10, @@L04001534        // if (flags & A_INIT) , skip
/* 04001520 000520 00000000 */   nop
/* 04001524 000524 0D0006B5 */  jal     dma_read_start
/* 04001528 000528 00000000 */   nop
/* 0400152C 00052C 09000550 */  j       @@L04001540             // step over (flags & A_INIT)
/* 04001530 000530 00000000 */   nop
@@L04001534:
/* 04001534 000534 A6E00008 */  sh      $zero, 8(tmpDataPtr)
/* 04001538 000538 4A10842C */  vclr    $v16
/* 0400153C 00053C EAF01800 */  sdv     $v16[0], (tmpDataPtr)
@@L04001540:
/* 04001540 000540 30EA0002 */  andi    $10, @@flags, A_LOOP
/* 04001544 000544 11400006 */  beqz    $10, @@L04001560        // if !(flags & A_LOOP) , skip
/* 04001548 000548 CAF01800 */   ldv    $v16[0], (tmpDataPtr)
/* 0400154C 00054C 2108FFFC */  addi    @@in_buf, @@in_buf, -4
/* 04001550 000550 E9100800 */  ssv     $v16[0], 0x0(@@in_buf)
/* 04001554 000554 E9100A01 */  ssv     $v16[4], 0x2(@@in_buf)
/* 04001558 000558 09000568 */  j       @@L040015A0
/* 0400155C 00055C 00000000 */   nop
@@L04001560:
/* 04001560 000560 30EA0004 */  andi    $10, @@flags, A_ADPCM_SHORT
/* 04001564 000564 1140000C */  beqz    $10, @@L04001598
/* 04001568 000568 00000000 */   nop
/* 0400156C 00056C 2108FFF0 */  addi    @@in_buf, @@in_buf, -0x10
/* 04001570 000570 E9100800 */  ssv     $v16[0], 0x0(@@in_buf)
/* 04001574 000574 E9100801 */  ssv     $v16[0], 0x2(@@in_buf)
/* 04001578 000578 E9100902 */  ssv     $v16[2], 0x4(@@in_buf)
/* 0400157C 00057C E9100903 */  ssv     $v16[2], 0x6(@@in_buf)
/* 04001580 000580 E9100A04 */  ssv     $v16[4], 0x8(@@in_buf)
/* 04001584 000584 E9100A05 */  ssv     $v16[4], 0xA(@@in_buf)
/* 04001588 000588 E9100B06 */  ssv     $v16[6], 0xC(@@in_buf)
/* 0400158C 00058C E9100B07 */  ssv     $v16[6], 0xE(@@in_buf)
/* 04001590 000590 09000568 */  j       @@L040015A0
/* 04001594 000594 00000000 */   nop
@@L04001598:
/* 04001598 000598 2108FFF8 */  addi    $8, $8, -8
/* 0400159C 00059C E9101800 */  sdv     $v16[0], ($8)
@@L040015A0:
/* 040015A0 0005A0 CAF70F04 */  lsv     $v23[14], 0x8(tmpDataPtr)   // saved pitch_accumulator
/* 040015A4 0005A4 C9101800 */  ldv     $v16[0], ($8)
/* 040015A8 0005A8 48889200 */  mtc2    $8, $v18[4]
/* 040015AC 0005AC 200A00E0 */  li      $10, 0xE0
/* 040015B0 0005B0 488A9300 */  mtc2    $10, $v18[6]
/* 040015B4 0005B4 489A9400 */  mtc2    cmd_w0, $v18[8]             // pitch
/* 040015B8 0005B8 200A0040 */  li      $10, 0x40
/* 040015BC 0005BC 488A9500 */  mtc2    $10, $v18[10]
/* 040015C0 0005C0 20090060 */  li      $9, data_0060
/* 040015C4 0005C4 C93F2001 */  lqv     $v31[0], 0x10($9)
/* 040015C8 0005C8 C9392000 */  lqv     $v25[0], 0x00($9)
/* 040015CC 0005CC 4A1FCE51 */  vsub    $v25, $v25, $v31
/* 040015D0 0005D0 C93E2002 */  lqv     $v30[0], 0x20($9)
/* 040015D4 0005D4 C93D2003 */  lqv     $v29[0], 0x30($9)
/* 040015D8 0005D8 C93C2004 */  lqv     $v28[0], 0x40($9)
/* 040015DC 0005DC C93B2005 */  lqv     $v27[0], 0x50($9)
/* 040015E0 0005E0 C93A2006 */  lqv     $v26[0], 0x60($9)
/* 040015E4 0005E4 4A1FCE51 */  vsub    $v25, $v25, $v31
/* 040015E8 0005E8 C9382007 */  lqv     $v24[0], 0x70($9)
/* 040015EC 0005EC 22F50020 */  addi    $21, tmpDataPtr, 0x20
/* 040015F0 0005F0 22F40030 */  addi    $20, tmpDataPtr, 0x30
/* 040015F4 0005F4 4A16B5AC */  vclr    $v22
/* 040015F8 0005F8 4BF7FDC5 */  vmudm   $v23, $v31, $v23[7]     // load pitch_accumulator into every vector elem
/* 040015FC 0005FC 4B92CD8D */  vmadm   $v22, $v25, $v18[4]     // (accumulate) >> 16
/* 04001600 000600 4B1EFDCE */  vmadn   $v23, $v31, $v30[0]     // result & 0xffff
/* 04001604 000604 4B52FD46 */  vmudn   $v21, $v31, $v18[2]     // load in address to every vector elem
/* 04001608 000608 4B5EB54E */  vmadn   $v21, $v22, $v30[2]     // accumulate 2*$v22
/* 0400160C 00060C 4BB2BC44 */  vmudl   $v17, $v23, $v18[5]     // 64 * $v23 >> 16
/* 04001610 000610 4B9E8C46 */  vmudn   $v17, $v17, $v30[4]     // *= 8
/* 04001614 000614 4B72FC4E */  vmadn   $v17, $v31, $v18[3]     // += 0x00c0
/* 04001618 000618 C9392000 */  lqv     $v25[0], ($9)
/* 0400161C 00061C EAB52000 */  sqv     $v21[0], ($21)
/* 04001620 000620 EA912000 */  sqv     $v17[0], ($20)
/* 04001624 000624 EAF70B84 */  ssv     $v23[7], 0x8(tmpDataPtr)
/* 04001628 000628 86B10000 */  lh      $17, ($21)
/* 0400162C 00062C 86890000 */  lh      $9, ($20)
/* 04001630 000630 86AD0008 */  lh      $13, 8($21)
/* 04001634 000634 86850008 */  lh      $5, 8($20)
/* 04001638 000638 86B00002 */  lh      $16, 2($21)
/* 0400163C 00063C 86880002 */  lh      $8, 2($20)
/* 04001640 000640 86AC000A */  lh      $12, 0xa($21)
/* 04001644 000644 8684000A */  lh      $4, 0xa($20)
/* 04001648 000648 86AF0004 */  lh      $15, 4($21)
/* 0400164C 00064C 86870004 */  lh      $7, 4($20)
/* 04001650 000650 86AB000C */  lh      $11, 0xc($21)
/* 04001654 000654 8683000C */  lh      $3, 0xc($20)
/* 04001658 000658 86AE0006 */  lh      $14, 6($21)
/* 0400165C 00065C 86860006 */  lh      $6, 6($20)
/* 04001660 000660 86AA000E */  lh      $10, 0xe($21)
/* 04001664 000664 8682000E */  lh      $2, 0xe($20)
@@L04001668:
/* 04001668 000668 CA301800 */  ldv     $v16[0], ($17)
/* 0400166C 00066C 4BF7FDC5 */  vmudm   $v23, $v31, $v23[7]
/* 04001670 000670 C92F1800 */  ldv     $v15[0], ($9)
/* 04001674 000674 4BF6FDCF */  vmadh   $v23, $v31, $v22[7]
/* 04001678 000678 C9B01C00 */  ldv     $v16[8], ($13)
/* 0400167C 00067C 4B92CD8D */  vmadm   $v22, $v25, $v18[4]
/* 04001680 000680 C8AF1C00 */  ldv     $v15[8], ($5)
/* 04001684 000684 4B1EFDCE */  vmadn   $v23, $v31, $v30[0]
/* 04001688 000688 CA0E1800 */  ldv     $v14[0], ($16)
/* 0400168C 00068C 4B52FD46 */  vmudn   $v21, $v31, $v18[2]
/* 04001690 000690 C90D1800 */  ldv     $v13[0], ($8)
/* 04001694 000694 4B5EB54E */  vmadn   $v21, $v22, $v30[2]
/* 04001698 000698 C98E1C00 */  ldv     $v14[8], ($12)
/* 0400169C 00069C 4BB2BC44 */  vmudl   $v17, $v23, $v18[5]
/* 040016A0 0006A0 C88D1C00 */  ldv     $v13[8], ($4)
/* 040016A4 0006A4 C9EC1800 */  ldv     $v12[0], ($15)
/* 040016A8 0006A8 C8EB1800 */  ldv     $v11[0], ($7)
/* 040016AC 0006AC C96C1C00 */  ldv     $v12[8], ($11)
/* 040016B0 0006B0 4B9E8C46 */  vmudn   $v17, $v17, $v30[4]
/* 040016B4 0006B4 C86B1C00 */  ldv     $v11[8], ($3)
/* 040016B8 0006B8 C9CA1800 */  ldv     $v10[0], ($14)
/* 040016BC 0006BC C8C91800 */  ldv     $v9[0], ($6)
/* 040016C0 0006C0 4B72FC4E */  vmadn   $v17, $v31, $v18[3]
/* 040016C4 0006C4 C94A1C00 */  ldv     $v10[8], ($10)
/* 040016C8 0006C8 4A0F8200 */  vmulf   $v8, $v16, $v15
/* 040016CC 0006CC C8491C00 */  ldv     $v9[8], ($2)
/* 040016D0 0006D0 4A0D71C0 */  vmulf   $v7, $v14, $v13
/* 040016D4 0006D4 EAB52000 */  sqv     $v21[0], ($21)
/* 040016D8 0006D8 4A0B6180 */  vmulf   $v6, $v12, $v11
/* 040016DC 0006DC EA912000 */  sqv     $v17[0], ($20)
/* 040016E0 0006E0 86B10000 */  lh      $17, ($21)
/* 040016E4 0006E4 4A095140 */  vmulf   $v5, $v10, $v9
/* 040016E8 0006E8 86890000 */  lh      $9, ($20)
/* 040016EC 0006EC 4A684210 */  vadd    $v8, $v8, $v8[1q]
/* 040016F0 0006F0 86AD0008 */  lh      $13, 8($21)
/* 040016F4 0006F4 4A6739D0 */  vadd    $v7, $v7, $v7[1q]
/* 040016F8 0006F8 86850008 */  lh      $5, 8($20)
/* 040016FC 0006FC 4A663190 */  vadd    $v6, $v6, $v6[1q]
/* 04001700 000700 86B00002 */  lh      $16, 2($21)
/* 04001704 000704 4A652950 */  vadd    $v5, $v5, $v5[1q]
/* 04001708 000708 86880002 */  lh      $8, 2($20)
/* 0400170C 00070C 4AC84210 */  vadd    $v8, $v8, $v8[2h]
/* 04001710 000710 86AC000A */  lh      $12, 0xa($21)
/* 04001714 000714 4AC739D0 */  vadd    $v7, $v7, $v7[2h]
/* 04001718 000718 8684000A */  lh      $4, 0xa($20)
/* 0400171C 00071C 4AC63190 */  vadd    $v6, $v6, $v6[2h]
/* 04001720 000720 86AF0004 */  lh      $15, 4($21)
/* 04001724 000724 4AC52950 */  vadd    $v5, $v5, $v5[2h]
/* 04001728 000728 86870004 */  lh      $7, 4($20)
/* 0400172C 00072C 4A88E906 */  vmudn   $v4, $v29, $v8[0h]
/* 04001730 000730 86AB000C */  lh      $11, 0xc($21)
/* 04001734 000734 4A87E10E */  vmadn   $v4, $v28, $v7[0h]
/* 04001738 000738 8683000C */  lh      $3, 0xc($20)
/* 0400173C 00073C 4A86D90E */  vmadn   $v4, $v27, $v6[0h]
/* 04001740 000740 86AE0006 */  lh      $14, 6($21)
/* 04001744 000744 4A85D10E */  vmadn   $v4, $v26, $v5[0h]
/* 04001748 000748 86860006 */  lh      $6, 6($20)
/* 0400174C 00074C 86AA000E */  lh      $10, 0xe($21)
/* 04001750 000750 2252FFF0 */  addi    @@count, @@count, -0x10
/* 04001754 000754 EA642000 */  sqv     $v4[0], ($19)
/* 04001758 000758 1A400003 */  blez    @@count, @@L04001768
/* 0400175C 00075C 8682000E */   lh     $2, 0xe($20)
/* 04001760 000760 0900059A */  j       @@L04001668
/* 04001764 000764 22730010 */   addi   $19, $19, 0x10
@@L04001768:
/* 04001768 000768 EAF70804 */  ssv     $v23[0], 0x8(tmpDataPtr)
/* 0400176C 00076C CA301800 */  ldv     $v16[0], ($17)
/* 04001770 000770 EAF01800 */  sdv     $v16[0], (tmpDataPtr)
/* 04001774 000774 00161020 */  add     @@dram_addr, $zero, @@dram_addr_save
/* 04001778 000778 22E10000 */  move    $1, tmpDataPtr
/* 0400177C 00077C 090006B1 */  j       dma_write_and_nextcmd
/* 04001780 000780 20030020 */   li     $3, 0x20

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Count2          | DMEM In                         | DMEM Out                        | Count                           |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Similar to DMEM Move but using lqv/sqv
 */
cmd_UNK16:
    @@count2   equ $15
    @@dmem_in  equ $13
    @@dmem_out equ $14
    @@count    equ $12

    srl     @@count2, cmd_w0, 0x10
    andi    @@count2, @@count2, 0xff
    andi    @@dmem_in, cmd_w0, 0xffff
    srl     @@dmem_out, cmd_w1, 0x10
@@loop_outer:
    addi    @@count2, @@count2, -1      // decr count2
    andi    @@count, cmd_w1, 0xffff     // (re)load count
@@loop_inner:
    lqv     $v1[0], 0x00(@@dmem_in)     // load from dmem in
    lqv     $v2[0], 0x10(@@dmem_in)
    addi    @@count, @@count, -0x20
    addi    @@dmem_in, @@dmem_in, 0x20
    sqv     $v1[0], 0x00(@@dmem_out)    // store to dmem out
    sqv     $v2[0], 0x10(@@dmem_out)
    bgtz    @@count, @@loop_inner       // until count done
     addi   @@dmem_out, @@dmem_out, 0x20
    bgtz    @@count2, @@loop_outer      // until count2 done
     nop
    j       next_cmd
     nop

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Count           | DMEM In                         | DMEM Out                        | Empty                           |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 * Copies `dmem_in` to `dmem_out` `count` times
 */
cmd_DUPLICATE:
    @@count    equ $15
    @@dmem_in  equ $13
    @@dmem_out equ $14

    srl     @@count, cmd_w0, 0x10
    andi    @@count, @@count, 0xff
    andi    @@dmem_in, cmd_w0, 0xffff
    srl     @@dmem_out, cmd_w1, 0x10
    lqv     $v1[0], 0x00(@@dmem_in)     // load from dmem in
    lqv     $v2[0], 0x10(@@dmem_in)
    lqv     $v3[0], 0x20(@@dmem_in)
    lqv     $v4[0], 0x30(@@dmem_in)
    lqv     $v5[0], 0x40(@@dmem_in)
    lqv     $v6[0], 0x50(@@dmem_in)
    lqv     $v7[0], 0x60(@@dmem_in)
    lqv     $v8[0], 0x70(@@dmem_in)
@@loop:
    addi    @@count, @@count, -1        // decr count
    sqv     $v1[0], 0x00(@@dmem_out)    // store to dmem out
    sqv     $v2[0], 0x10(@@dmem_out)
    sqv     $v3[0], 0x20(@@dmem_out)
    sqv     $v4[0], 0x30(@@dmem_out)
    sqv     $v5[0], 0x40(@@dmem_out)
    sqv     $v6[0], 0x50(@@dmem_out)
    sqv     $v7[0], 0x60(@@dmem_out)
    sqv     $v8[0], 0x70(@@dmem_out)
    bgtz    @@count, @@loop             // until done
     addi   @@dmem_out, @@dmem_out, 0x80
    j       next_cmd
     nop

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Empty           | Count                           | DMEM In                         | DMEM Out                        |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Interleave
 */
cmd_INTERL:
    @@count    equ $12
    @@dmem_in  equ $13
    @@dmem_out equ $14

    andi    @@count, cmd_w0, 0xffff
    andi    @@dmem_out, cmd_w1, 0xffff
    srl     @@dmem_in, cmd_w1, 0x10
@@loop:
    lsv     $v1[0], 0x00(@@dmem_in)     // load from dmem in with interleaving
    lsv     $v2[0], 0x08(@@dmem_in)
    lsv     $v3[0], 0x10(@@dmem_in)
    lsv     $v4[0], 0x18(@@dmem_in)
    lsv     $v1[2], 0x04(@@dmem_in)
    lsv     $v2[2], 0x0C(@@dmem_in)
    lsv     $v3[2], 0x14(@@dmem_in)
    lsv     $v4[2], 0x1C(@@dmem_in)
    addi    @@dmem_in, @@dmem_in, 0x20
    addi    @@count, @@count, -8        // decr count
    slv     $v1[0], 0x0(@@dmem_out)     // store to dmem out
    slv     $v2[0], 0x4(@@dmem_out)
    slv     $v3[0], 0x8(@@dmem_out)
    slv     $v4[0], 0xC(@@dmem_out)
    bgtz    @@count, @@loop             // until done
     addi   @@dmem_out, @@dmem_out, 0x10
    j       next_cmd
     nop

/**
 * |                 |                 |                 |       |   |   |   |   |   |                 |                 |                 |                 |
 * | Command         | DMEM Address    | Count           | Empty | S | X0| X1| X2| X3| DMEM Left       | DMEM Right      | DMEM Wet Left   | DMEM Wet Right  |
 * | 8               | 8               | 8               | 3     | 1 | 1 | 1 | 1 | 1 | 8               | 8               | 8               | 8               |
 * | 31           24 | 23           16 | 15            8 | 7   5 | 4 | 3 | 2 | 1 | 0 | 31           24 | 23           16 | 15            8 | 7             0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 | 0 | 0 | 0 | 0 | 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 |
 *
 * S  = swapLR
 * X0 = stereoHeadsetEffects
 * X1 = usesHeadsetPanEffects
 * X2 = stereoStrongRight
 * X3 = stereoStrongLeft
 *
 * Common w1 configurations:
 *  envmixer w1
 *  p1                  p2                  p3                p4
 *  DMEM_NOTE_PAN_TEMP, DMEM_RIGHT_CH,      DMEM_WET_LEFT_CH, DMEM_WET_RIGHT_CH
 *  DMEM_LEFT_CH,       DMEM_NOTE_PAN_TEMP, DMEM_WET_LEFT_CH, DMEM_WET_RIGHT_CH
 *  DMEM_LEFT_CH,       DMEM_RIGHT_CH,      DMEM_WET_LEFT_CH, DMEM_WET_RIGHT_CH
 *
 *  Envelope Mixer
 */
cmd_ENVMIXER:
    @@mem_addr       equ $19
    @@count          equ $20
    @@swapLR         equ $10
    @@left_addr      equ $14
    @@right_addr     equ $15
    @@wet_left_addr  equ $16
    @@wet_right_addr equ $17

/* 04001884 000884 4A04212C */  vclr    $v4
/* 04001888 000888 4A00002C */  vclr    $vzero
/* 0400188C 00088C C8032000 */  lqv     $v3[0], (data_0000)($zero)
/* 04001890 000890 02B5A820 */  add     $21, $21, $21       // rampLeft += rampLeft
/* 04001894 000894 48952000 */  mtc2    $21, $v4[0]         // rampLeft
/* 04001898 000898 48952100 */  mtc2    $21, $v4[2]         // rampLeft
/* 0400189C 00089C 001A6302 */  srl     $12, cmd_w0, 0xc        // Extract DMEM addr
/* 040018A0 0008A0 31930FF0 */  andi    @@mem_addr, $12, 0xff0
/* 040018A4 0008A4 02D6B020 */  add     $22, $22, $22       // rampRight += rampRight
/* 040018A8 0008A8 48962200 */  mtc2    $22, $v4[4]         // rampRight
/* 040018AC 0008AC 48962300 */  mtc2    $22, $v4[6]         // rampRight
/* 040018B0 0008B0 00196502 */  srl     $12, cmd_w1, 0x14       // Extract left
/* 040018B4 0008B4 318E0FF0 */  andi    @@left_addr, $12, 0xff0
/* 040018B8 0008B8 016B5820 */  add     $11, $11, $11       // rampReverb += rampReverb
/* 040018BC 0008BC 488B2400 */  mtc2    $11, $v4[8]         // rampReverb
/* 040018C0 0008C0 488B2500 */  mtc2    $11, $v4[10]        // rampReverb
/* 040018C4 0008C4 00196302 */  srl     $12, cmd_w1, 0xc        // Extract right
/* 040018C8 0008C8 318F0FF0 */  andi    @@right_addr, $12, 0xff0
/* 040018CC 0008CC 00196102 */  srl     $12, cmd_w1, 4          // Extract wet left
/* 040018D0 0008D0 31900FF0 */  andi    $16, $12, 0xff0
/* 040018D4 0008D4 00196100 */  sll     $12, cmd_w1, 4          // Extract wet right
/* 040018D8 0008D8 31910FF0 */  andi    @@wet_right_addr, $12, 0xff0
/* 040018DC 0008DC 334C0002 */  andi    $12, cmd_w0, 2          // stereoStrongRight
/* 040018E0 0008E0 000C6042 */  srl     $12, $12, 1
/* 040018E4 0008E4 000C6022 */  neg     $12, $12
/* 040018E8 0008E8 488C1000 */  mtc2    $12, $v2[0]
/* 040018EC 0008EC 334C0001 */  andi    $12, cmd_w0, 1          // stereoStrongLeft
/* 040018F0 0008F0 000C6022 */  neg     $12, $12
/* 040018F4 0008F4 488C1100 */  mtc2    $12, $v2[2]
/* 040018F8 0008F8 334C0008 */  andi    $12, cmd_w0, 8          // stereoHeadsetEffects
/* 040018FC 0008FC 000C6042 */  srl     $12, $12, 1
/* 04001900 000900 000C6022 */  neg     $12, $12
/* 04001904 000904 488C1200 */  mtc2    $12, $v2[4]
/* 04001908 000908 334C0004 */  andi    $12, cmd_w0, 4          // usesHeadsetPanEffects
/* 0400190C 00090C 000C6042 */  srl     $12, $12, 1
/* 04001910 000910 000C6022 */  neg     $12, $12
/* 04001914 000914 488C1300 */  mtc2    $12, $v2[6]
/* 04001918 000918 001A6202 */  srl     $12, cmd_w0, 8          // Extract count
/* 0400191C 00091C 319400FF */  andi    @@count, $12, 0xff
/* 04001920 000920 4A000010 */  vadd    $vzero, $vzero, $vzero
/* 04001924 000924 334A0010 */  andi    @@swapLR, cmd_w0, 0x10  // Extract swapLR
/* 04001928 000928 CA682000 */  lqv     $v8[0], 0x00(@@mem_addr)
@@loop:
/* 0400192C 00092C CA6F2001 */  lqv     $v15[0], 0x10(@@mem_addr)
/* 04001930 000930 22730020 */  addi    @@mem_addr, @@mem_addr, 0x20
/* 04001934 000934 4B014245 */  vmudm   $v9, $v8, $v1[0]        // vol left
/* 04001938 000938 4B414285 */  vmudm   $v10, $v8, $v1[2]       // vol right
/* 0400193C 00093C 2294FFF0 */  addi    @@count, @@count, -0x10
/* 04001940 000940 C9CB2000 */  lqv     $v11[0], 0x00(@@left_addr)
/* 04001944 000944 C9EC2000 */  lqv     $v12[0], 0x00(@@right_addr)
/* 04001948 000948 4B217C05 */  vmudm   $v16, $v15, $v1[1]      // vol left + ramp left
/* 0400194C 00094C 4B617C45 */  vmudm   $v17, $v15, $v1[3]      // vol right + ramp right
/* 04001950 000950 C9D22001 */  lqv     $v18[0], 0x10(@@left_addr)
/* 04001954 000954 C9F32001 */  lqv     $v19[0], 0x10(@@right_addr)
/* 04001958 000958 4B024A6C */  vxor    $v9, $v9, $v2[0]        // stereoStrongRight
/* 0400195C 00095C 4B2252AC */  vxor    $v10, $v10, $v2[1]      // stereoStrongLeft
/* 04001960 000960 CA0D2000 */  lqv     $v13[0], 0x00(@@wet_left_addr)
/* 04001964 000964 CA2E2000 */  lqv     $v14[0], 0x00(@@wet_right_addr)
/* 04001968 000968 4A095AD0 */  vadd    $v11, $v11, $v9
/* 0400196C 00096C 4A0A6310 */  vadd    $v12, $v12, $v10
/* 04001970 000970 4B814A45 */  vmudm   $v9, $v9, $v1[4]        // reverb vol
/* 04001974 000974 4B815285 */  vmudm   $v10, $v10, $v1[4]      // reverb vol
/* 04001978 000978 4B02842C */  vxor    $v16, $v16, $v2[0]      // stereoStrongRight
/* 0400197C 00097C 4B228C6C */  vxor    $v17, $v17, $v2[1]      // stereoStrongLeft
/* 04001980 000980 CA142001 */  lqv     $v20[0], 0x10(@@wet_left_addr)
/* 04001984 000984 CA352001 */  lqv     $v21[0], 0x10(@@wet_right_addr)
/* 04001988 000988 4A109490 */  vadd    $v18, $v18, $v16
/* 0400198C 00098C 4A119CD0 */  vadd    $v19, $v19, $v17
/* 04001990 000990 4BA18405 */  vmudm   $v16, $v16, $v1[5]      // reverb vol + ramp reverb
/* 04001994 000994 4BA18C45 */  vmudm   $v17, $v17, $v1[5]      // reverb vol + ramp reverb
/* 04001998 000998 4B424A6C */  vxor    $v9, $v9, $v2[2]        // stereoHeadsetEffects
/* 0400199C 00099C 4B6252AC */  vxor    $v10, $v10, $v2[3]      // usesHeadsetPanEffects
/* 040019A0 0009A0 E9CB2000 */  sqv     $v11[0], 0x00(@@left_addr)
/* 040019A4 0009A4 4B42842C */  vxor    $v16, $v16, $v2[2]      // stereoHeadsetEffects
/* 040019A8 0009A8 4B628C6C */  vxor    $v17, $v17, $v2[3]      // usesHeadsetPanEffects
/* 040019AC 0009AC 15400015 */  bnez    @@swapLR, @@swap
/* 040019B0 0009B0 E9EC2000 */   sqv    $v12[0], 0x00(@@right_addr)
/* 040019B4 0009B4 4A096B50 */  vadd    $v13, $v13, $v9
/* 040019B8 0009B8 4A0A7390 */  vadd    $v14, $v14, $v10
/* 040019BC 0009BC E9D22001 */  sqv     $v18[0], 0x10(@@left_addr)
/* 040019C0 0009C0 E9F32001 */  sqv     $v19[0], 0x10(@@right_addr)
/* 040019C4 0009C4 4A10A510 */  vadd    $v20, $v20, $v16
/* 040019C8 0009C8 4A11AD50 */  vadd    $v21, $v21, $v17
@@L040019CC:
/* 040019CC 0009CC 21CE0020 */  addi    @@left_addr, @@left_addr, 0x20
/* 040019D0 0009D0 EA0D2000 */  sqv     $v13[0], 0x00(@@wet_left_addr)
/* 040019D4 0009D4 EA2E2000 */  sqv     $v14[0], 0x00(@@wet_right_addr)
/* 040019D8 0009D8 21EF0020 */  addi    @@right_addr, @@right_addr, 0x20
/* 040019DC 0009DC CA682000 */  lqv     $v8[0], 0x00(@@mem_addr)
/* 040019E0 0009E0 EA142001 */  sqv     $v20[0], 0x10(@@wet_left_addr)
/* 040019E4 0009E4 22100020 */  addi    @@wet_left_addr, @@wet_left_addr, 0x20
/* 040019E8 0009E8 4A040854 */  vaddc   $v1, $v1, $v4
/* 040019EC 0009EC EA352001 */  sqv     $v21[0], 0x10(@@wet_right_addr)
/* 040019F0 0009F0 22310020 */  addi    @@wet_right_addr, @@wet_right_addr, 0x20
/* 040019F4 0009F4 1E80FFCD */  bgtz    @@count, @@loop
/* 040019F8 0009F8 4A000010 */   vadd   $vzero, $vzero, $vzero
/* 040019FC 0009FC 09000423 */  j       next_cmd
/* 04001A00 000A00 4A00002C */   vclr   $vzero
@@swap:
/* 04001A04 000A04 4A0A6B50 */  vadd    $v13, $v13, $v10
/* 04001A08 000A08 4A097390 */  vadd    $v14, $v14, $v9
/* 04001A0C 000A0C E9D22001 */  sqv     $v18[0], 0x10(@@left_addr)
/* 04001A10 000A10 E9F32001 */  sqv     $v19[0], 0x10(@@right_addr)
/* 04001A14 000A14 4A11A510 */  vadd    $v20, $v20, $v17
/* 04001A18 000A18 09000673 */  j       @@L040019CC
/* 04001A1C 000A1C 4A10AD50 */   vadd   $v21, $v21, $v16

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Reverb Vol      | Ramp Reverb                     | Ramp Left                       | Ramp Right                      |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Envelope Setup (1)
 */
cmd_ENVSETUP1:
    @@reverb_vol  equ $12   // ?
    @@ramp_reverb equ $11
    @@ramp_left   equ $21   // TODO this is global
    @@ramp_right  equ $22   // TODO this is global

    vclr    $v1
    andi    @@ramp_reverb, cmd_w0, 0xffff
    srl     @@reverb_vol, cmd_w0, 8
    andi    @@reverb_vol, @@reverb_vol, 0xff00
    mtc2    @@reverb_vol, $v1[8]    // vol before ramp?
    add     @@reverb_vol, @@reverb_vol, @@ramp_reverb
    mtc2    @@reverb_vol, $v1[10]   // vol after ramp?
    srl     @@ramp_left, cmd_w1, 0x10
    j       next_cmd
     andi   @@ramp_right, cmd_w1, 0xffff

/**
 * |                 |                                                 |                                 |                                 |
 * | Command         | Empty                                           | Vol Left                        | Vol Right                       |
 * | 8               | 24                                              | 16                              | 16                              |
 * | 31           24 | 23                                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Envelope Setup (2)
 */
cmd_ENVSETUP2:
    @@vol_left  equ $12
    @@vol_right equ $12

    srl     @@vol_left, cmd_w1, 0x10
    mtc2    @@vol_left, $v1[0]          // apply vol left
    add     @@vol_left, @@vol_left, $21
    mtc2    @@vol_left, $v1[2]          // gain?
    andi    @@vol_right, cmd_w1, 0xffff
    mtc2    @@vol_right, $v1[4]         // apply vol right
    add     @@vol_right, @@vol_right, $22
    j       next_cmd
     mtc2   @@vol_right, $v1[6]         // gain?



func_04001A6C: // setup buff and noop
    srl     $3, cmd_w0, 0xc     // Count
    andi    $3, $3, 0xff0
    andi    $1, cmd_w0, 0xffff  // DMEM addr
setup_loadadpcm:
    sll     $2, cmd_w1, 8
    jr      $ra
     srl    $2, $2, 8           // DRAM addr, remove highest byte

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Count           | DMEM Addr                       | Address                                                         |
 * | 8               | 8               | 16                              | 16                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_LOADBUFF:
    jal     func_04001A6C
     nop
    j       dma_read_and_nextcmd
     nop

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Count           | DMEM Addr                       | Address                                                         |
 * | 8               | 8               | 16                              | 16                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_SAVEBUFF:
    jal     func_04001A6C
     nop
    j       dma_write_and_nextcmd
     nop

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Empty           | Count                           | Address                                                         |
 * | 8               | 8               | 16                              | 16                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_LOADADPCM:
    jal     setup_loadadpcm
     li     $1, adpcmTable
    j       dma_read_and_nextcmd
     andi   $3, cmd_w0, 0xffff

dma_read_and_nextcmd:
    jal     dma_read_start
     addi   $3, $3, -1
    j       next_cmd
     nop

dma_write_and_nextcmd:
    jal     dma_write_start
     addi   $3, $3, -1
    j       next_cmd
     nop

/**
 * DMA read
 *
 * Args:
 *  $1 : dmem addr
 *  $2 : dram addr
 *  $3 : len
 */
dma_read_start:
    mfc0    $4, SP_SEMAPHORE        // acquire semaphore
    bnez    $4, dma_read            //! is this a bug? why not branch to dma_read_start ?
     nop
@@dma_not_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_not_full
     nop
    mtc0    $1, SP_MEM_ADDR
    mtc0    $2, SP_DRAM_ADDR
    j       @dma_wait_start
     mtc0   $3, SP_RD_LEN
dma_write_start:
    mfc0    $4, SP_SEMAPHORE        // acquire semaphore
    bnez    $4, dma_write
     nop
@@dma_not_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_not_full
     nop
    mtc0    $1, SP_MEM_ADDR
    mtc0    $2, SP_DRAM_ADDR
    mtc0    $3, SP_WR_LEN
@dma_wait_start:
    li      $4, 1
@@dma_wait:
    bnez    $4, @@dma_wait
     mfc0   $4, SP_DMA_BUSY
    jr      $ra
     mtc0   $zero, SP_SEMAPHORE     // release semaphore

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Count           | Gain                            | DMEM In                         | DMEM Out                        |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_MIXER:
    @@count    equ $18
    @@gain     equ $17
    @@dmem_in  equ $20
    @@dmem_out equ $19

    lqv     $v31[0], (data_0000)($zero)
    srl     @@count, cmd_w0, 0xc
    andi    @@count, @@count, 0xff0         // count << 4
    andi    @@dmem_out, cmd_w1, 0xffff      // load dmem out
    srl     @@dmem_in, cmd_w1, 0x10         // load dmem in
    andi    @@gain, cmd_w0, 0xffff
    mtc2    @@gain, $v30[0]                 // set gain
    lqv     $v27[0], 0x00(@@dmem_out)       // do mix
    lqv     $v29[0], 0x00(@@dmem_in)
    lqv     $v26[0], 0x10(@@dmem_out)
    lqv     $v28[0], 0x10(@@dmem_in)
@@loop:
    vmulf   $v27, $v27, $v31[6]             // out * 0.9999(0x7FFF)
    addi    @@count, @@count, -0x20
    vmacf   $v27, $v29, $v30[0]             // out * 1 + in * gain
    addi    @@dmem_in, @@dmem_in, 0x20
    vmulf   $v26, $v26, $v31[6]             // out * 0.9999(0x7FFF)
    vmacf   $v26, $v28, $v30[0]             // out * 1 + in * gain
    lqv     $v29[0], (@@dmem_in)
    sqv     $v27[0], (@@dmem_out)           // out = out * 1 + in * gain
    lqv     $v27[0], 0x20(@@dmem_out)
    sqv     $v26[0], 0x10(@@dmem_out)       // out = out * 1 + in * gain
    lqv     $v28[0], 0x10(@@dmem_in)
    addi    @@dmem_out, @@dmem_out, 0x20
    bgtz    @@count, @@loop                 // until done
     lqv    $v26[0], 0x10(@@dmem_out)
    j       next_cmd
     nop

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Flags           | Empty                           | Address                                                         |
 * | 8               | 8               | 16                              | 32                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  "Decompress" signed 8-bit PCM to signed 16-bit PCM
 */
cmd_S8DEC:
    @@flags     equ $1
    @@dram_addr equ $17

    @@input_buffer equ $13
    @@output_buffer equ $14
    @@count equ $12

    @@input_data_lo equ $v4
    @@input_data_hi equ $v5

/* 04001BA0 000BA0 970D0000 */  lhu     @@input_buffer, (audio_in_buf)(audioStructPtr)
/* 04001BA4 000BA4 4A0210AC */  vclr    $v2
/* 04001BA8 000BA8 970E0002 */  lhu     @@output_buffer, (audio_out_buf)(audioStructPtr)
/* 04001BAC 000BAC 4A0318EC */  vclr    $v3
/* 04001BB0 000BB0 970C0004 */  lhu     @@count, (audio_count)(audioStructPtr)
/* 04001BB4 000BB4 00198A00 */  sll     @@dram_addr, cmd_w1, 8
/* 04001BB8 000BB8 00118A02 */  srl     @@dram_addr, @@dram_addr, 8
/* 04001BBC 000BBC E9C22000 */  sqv     $v2[0], 0x00(@@output_buffer)
/* 04001BC0 000BC0 E9C32001 */  sqv     $v3[0], 0x10(@@output_buffer)
/* 04001BC4 000BC4 001A0C02 */  srl     @@flags, cmd_w0, 0x10
/* 04001BC8 000BC8 30210001 */  andi    @@flags, @@flags, A_INIT
/* 04001BCC 000BCC 1C200008 */  bgtz    @@flags, @@no_dma
/* 04001BD0 000BD0 001A0C02 */   srl    @@flags, cmd_w0, 0x10
/* 04001BD4 000BD4 30210002 */  andi    @@flags, @@flags, A_LOOP
/* 04001BD8 000BD8 10010002 */  beq     $zero, @@flags, @@no_loop
/* 04001BDC 000BDC 22220000 */   addi   $2, @@dram_addr, 0                      // DMA read from dram_addr
/* 04001BE0 000BE0 8F020008 */  lw      $2, (audio_loop_addr)(audioStructPtr)   // DMA read from loop_addr
@@no_loop:
/* 04001BE4 000BE4 21C10000 */  addi    $1, @@output_buffer, 0  // read into output buffer? (DMEM)
/* 04001BE8 000BE8 0D0006B5 */  jal     dma_read_start
/* 04001BEC 000BEC 2003001F */   li     $3, 0x20 - 1
@@no_dma:
/* 04001BF0 000BF0 21CE0020 */  addi    @@output_buffer, @@output_buffer, 0x20
/* 04001BF4 000BF4 1180000B */  beqz    @@count, @@done     // Count is 0, done
/* 04001BF8 000BF8 00000000 */   nop
/* 04001BFC 000BFC C9A23000 */  lpv     $v2[0], 0x0(@@input_buffer)     // load packed from input buffer [0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA] -> [0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00]
/* 04001C00 000C00 C9A33001 */  lpv     $v3[0], 0x8(@@input_buffer)
/* 04001C04 000C04 21AD0010 */  addi    @@input_buffer, @@input_buffer, 0x10
@@loop:
/* 04001C08 000C08 218CFFE0 */  addi    @@count, @@count, -0x20
/* 04001C0C 000C0C C9A43000 */  lpv     @@input_data_lo[0], (@@input_buffer)
/* 04001C10 000C10 E9C22000 */  sqv     $v2[0], (@@output_buffer)
/* 04001C14 000C14 C9A53001 */  lpv     @@input_data_hi[0], 0x8(@@input_buffer)
/* 04001C18 000C18 E9C32001 */  sqv     $v3[0], 0x10(@@output_buffer)
/* 04001C1C 000C1C 1D800005 */  bgtz    @@count, @@not_done
/* 04001C20 000C20 21CE0020 */   addi   @@output_buffer, @@output_buffer, 0x20
@@done:
/* 04001C24 000C24 21C1FFE0 */  addi    $1, @@output_buffer, -0x20  // Write last 0x20 of output buffer to dram_addr
/* 04001C28 000C28 22220000 */  move    $2, @@dram_addr
/* 04001C2C 000C2C 090006B1 */  j       dma_write_and_nextcmd
/* 04001C30 000C30 20030020 */   li     $3, 0x20
@@not_done:
/* 04001C34 000C34 218CFFE0 */  addi    @@count, @@count, -0x20
/* 04001C38 000C38 C9A23002 */  lpv     $v2[0], 0x10(@@input_buffer)
/* 04001C3C 000C3C E9C42000 */  sqv     @@input_data_lo[0], 0x00(@@output_buffer)
/* 04001C40 000C40 C9A33003 */  lpv     $v3[0], 0x18(@@input_buffer)
/* 04001C44 000C44 E9C52001 */  sqv     @@input_data_hi[0], 0x10(@@output_buffer)
/* 04001C48 000C48 21AD0020 */  addi    @@input_buffer, @@input_buffer, 0x20
/* 04001C4C 000C4C 1D80FFEE */  bgtz    @@count, @@loop
/* 04001C50 000C50 21CE0020 */   addi   @@output_buffer, @@output_buffer, 0x20
/* 04001C54 000C54 09000709 */  j       @@done
/* 04001C58 000C58 00000000 */   nop

/**
 * |                 |         |         |                                 |                                 |                                 |
 * | Command         | HI Gain | LO Gain | Count                           | DMEM Address                    | Empty                           |
 * | 8               | 4       | 4       | 16                              | 16                              | 16                              |
 * | 31           24 | 23   20 | 19   16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 | 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_HILOGAIN:
    @@gain      equ $15
    @@count     equ $12
    @@dmem_addr equ $13

    andi    @@count, cmd_w0, 0xffff     // load count
    srl     @@dmem_addr, cmd_w1, 0x10   // extract dmem addr
    srl     @@gain, cmd_w0, 4
    andi    @@gain, @@gain, 0xf000
    mtc2    @@gain, $v3[2]              // move LO(gain) << 0xC to $v3[2]
    srl     @@gain, cmd_w0, 0x14
    andi    @@gain, @@gain, 0xf
    mtc2    @@gain, $v3[0]              // move HI(gain) to $v3[0]
@@loop:
    lqv     $v1[0], 0x00(@@dmem_addr)   // load
    lqv     $v2[0], 0x10(@@dmem_addr)
    vmudm   $v4, $v1, $v3[1]            // dmem * LO(gain)
    vmadh   $v4, $v1, $v3[0]            // dmem = dmem * LO(gain) + dmem * HI(gain)
    vmudm   $v5, $v2, $v3[1]
    vmadh   $v5, $v2, $v3[0]
    sqv     $v4[0], 0x00(@@dmem_addr)   // store
    sqv     $v5[0], 0x10(@@dmem_addr)
    addi    @@count, @@count, -0x20     // decr count in multiples of 0x20
    bgtz    @@count, @@loop             // until done
     addi   @@dmem_addr, @@dmem_addr, 0x20
    j       next_cmd
     vclr   $vzero

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Flags           | Count or DMEM Address           | DRAM Address                                                    |
 * | 8               | 8               | 16                              | 32                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_FILTER:
    @@flags     equ $12
    @@count     equ $15
    @@dmem_addr equ $11
    @@dramAddr  equ $2

    @@buffer1 equ $13
    @@buffer2 equ $14

/* 04001CB0 000CB0 22ED0000 */  move    @@buffer1, tmpDataPtr
/* 04001CB4 000CB4 4A00002C */  vclr    $vzero
/* 04001CB8 000CB8 22EE0020 */  addi    @@buffer2, tmpDataPtr, 0x20
/* 04001CBC 000CBC E9A02000 */  sqv     $vzero[0], 0x00(@@buffer1)      // reset buffer1
/* 04001CC0 000CC0 00191200 */  sll     @@dramAddr, cmd_w1, 8
/* 04001CC4 000CC4 E9A02001 */  sqv     $vzero[0], 0x10(@@buffer1)
/* 04001CC8 000CC8 00021202 */  srl     @@dramAddr, @@dramAddr, 8
/* 04001CCC 000CCC 001A6402 */  srl     @@flags, cmd_w0, 0x10
/* 04001CD0 000CD0 318C00FF */  andi    @@flags, @@flags, 0xff
/* 04001CD4 000CD4 1180000B */  beqz    @@flags, @@dma_buffer           // branch if flags = A_CONTINUE
/* 04001CD8 000CD8 00000000 */   nop
/* 04001CDC 000CDC 218CFFFF */  addi    @@flags, @@flags, -1
/* 04001CE0 000CE0 1180000A */  beqz    @@flags, @@no_dma               // branch if flags = A_INIT
/* 04001CE4 000CE4 00000000 */   nop
/* 04001CE8 000CE8 334FFFFF */  andi    @@count, cmd_w0, 0xffff
/* 04001CEC 000CEC 4A00002C */  vclr    $vzero
/* 04001CF0 000CF0 E9C02000 */  sqv     $vzero[0], 0x00(@@buffer2)
/* 04001CF4 000CF4 E9C02002 */  sqv     $vzero[0], 0x20(@@buffer2)
/* 04001CF8 000CF8 21C10010 */  addi    $1, @@buffer2, 0x10
/* 04001CFC 000CFC 090006AD */  j       dma_read_and_nextcmd
/* 04001D00 000D00 20030010 */   li     $3, 0x10
@@dma_buffer:
/* 04001D04 000D04 01A00820 */  add     $1, @@buffer1, $zero            // read buffer1 from dram address
/* 04001D08 000D08 0D0006B5 */  jal     dma_read_start
@@no_dma:
/* 04001D0C 000D0C 2003001F */   li     $3, 0x20 - 1
/* 04001D10 000D10 C9D82001 */  lqv     $v24[0], 0x10(@@buffer2)
/* 04001D14 000D14 C9B92001 */  lqv     $v25[0], 0x10(@@buffer1)
/* 04001D18 000D18 4A0E73AC */  vclr    $v14
/* 04001D1C 000D1C 4A000000 */  vmulf   $vzero, $vzero, $vzero          // Clear accumulator?
/* 04001D20 000D20 200C4000 */  li      $12, 0x4000                     // 0.5(0x4000)
/* 04001D24 000D24 488C7800 */  mtc2    $12, $v15[0]
/* 04001D28 000D28 4B0FC388 */  vmacf   $v14, $v24, $v15[0]             // buffer2[0x10..0x20] * 0.5 + 0
/* 04001D2C 000D2C 4B0FCB88 */  vmacf   $v14, $v25, $v15[0]             // buffer1[0x10..0x20] * 0.5 + buffer2[0x10..0x20] * 0.5 + 0
/* 04001D30 000D30 E9CE2001 */  sqv     $v14[0], 0x10(@@buffer2)        // store average of buffer1 and buffer2
/* 04001D34 000D34 E9AE2001 */  sqv     $v14[0], 0x10(@@buffer1)
/* 04001D38 000D38 01A00820 */  add     $1, @@buffer1, $zero            // set up dram address for later dma write to rdram
/* 04001D3C 000D3C 334BFFFF */  andi    @@dmem_addr, cmd_w0, 0xffff     // set up dmem address
/* 04001D40 000D40 C9D82001 */  lqv     $v24[0], 0x10(@@buffer2)        // buffer2[0x10..0x20] ldv is used for most of these as lqv cannot perform the same kind of unaligned memory access
/* 04001D44 000D44 C9DC1801 */  ldv     $v28[0], 0x08(@@buffer2)        // buffer2[0x08..0x10]
/* 04001D48 000D48 C9DC1C02 */  ldv     $v28[8], 0x10(@@buffer2)        // buffer2[0x10..0x18]
/* 04001D4C 000D4C C9D41803 */  ldv     $v20[0], 0x18(@@buffer2)        // buffer2[0x18..0x20]
/* 04001D50 000D50 C9D41C04 */  ldv     $v20[8], 0x20(@@buffer2)        // buffer2[0x20..0x28]
/* 04001D54 000D54 21CE0002 */  addi    @@buffer2, @@buffer2, 2     // 2
/* 04001D58 000D58 C9DF1800 */  ldv     $v31[0], 0x00(@@buffer2)        // buffer2[0x02..0x0A]
/* 04001D5C 000D5C C9DF1C01 */  ldv     $v31[8], 0x08(@@buffer2)        // buffer2[0x0A..0x12]
/* 04001D60 000D60 C9D11802 */  ldv     $v17[0], 0x10(@@buffer2)        // buffer2[0x12..0x1A]
/* 04001D64 000D64 C9D11C03 */  ldv     $v17[8], 0x18(@@buffer2)        // buffer2[0x1A..0x22]
/* 04001D68 000D68 C9DB1801 */  ldv     $v27[0], 0x08(@@buffer2)        // buffer2[0x0A..0x12]
/* 04001D6C 000D6C C9DB1C02 */  ldv     $v27[8], 0x10(@@buffer2)        // buffer2[0x12..0x1A]
/* 04001D70 000D70 C9D51803 */  ldv     $v21[0], 0x18(@@buffer2)        // buffer2[0x1A..0x22]
/* 04001D74 000D74 C9D51C04 */  ldv     $v21[8], 0x20(@@buffer2)        // buffer2[0x22..0x2A]
/* 04001D78 000D78 21CE0002 */  addi    @@buffer2, @@buffer2, 2     // 4
/* 04001D7C 000D7C C9DE1800 */  ldv     $v30[0], 0x00(@@buffer2)        // buffer2[0x04..0x0C]
/* 04001D80 000D80 C9DE1C01 */  ldv     $v30[8], 0x08(@@buffer2)        // buffer2[0x0C..0x14]
/* 04001D84 000D84 C9DA1801 */  ldv     $v26[0], 0x08(@@buffer2)        // buffer2[0x0C..0x14]
/* 04001D88 000D88 C9DA1C02 */  ldv     $v26[8], 0x10(@@buffer2)        // buffer2[0x14..0x1C]
/* 04001D8C 000D8C C9D21802 */  ldv     $v18[0], 0x10(@@buffer2)        // buffer2[0x14..0x1C]
/* 04001D90 000D90 C9D21C03 */  ldv     $v18[8], 0x18(@@buffer2)        // buffer2[0x1C..0x24]
/* 04001D94 000D94 C9D61803 */  ldv     $v22[0], 0x18(@@buffer2)        // buffer2[0x1C..0x24]
/* 04001D98 000D98 C9D61C04 */  ldv     $v22[8], 0x20(@@buffer2)        // buffer2[0x24..0x2C]
/* 04001D9C 000D9C 21CE0002 */  addi    @@buffer2, @@buffer2, 2     // 6
/* 04001DA0 000DA0 C9DD1800 */  ldv     $v29[0], 0x00(@@buffer2)        // buffer2[0x06..0x0E]
/* 04001DA4 000DA4 C9DD1C01 */  ldv     $v29[8], 0x08(@@buffer2)        // buffer2[0x0E..0x16]
/* 04001DA8 000DA8 C9D91801 */  ldv     $v25[0], 0x08(@@buffer2)        // buffer2[0x0E..0x16]
/* 04001DAC 000DAC C9D91C02 */  ldv     $v25[8], 0x10(@@buffer2)        // buffer2[0x16..0x1E]
/* 04001DB0 000DB0 C9D31802 */  ldv     $v19[0], 0x10(@@buffer2)        // buffer2[0x16..0x1E]
/* 04001DB4 000DB4 C9D31C03 */  ldv     $v19[8], 0x18(@@buffer2)        // buffer2[0x1E..0x26]
/* 04001DB8 000DB8 C9D71803 */  ldv     $v23[0], 0x18(@@buffer2)        // buffer2[0x1E..0x26]
/* 04001DBC 000DBC C9D71C04 */  ldv     $v23[8], 0x20(@@buffer2)        // buffer2[0x26..0x2E]
/* 04001DC0 000DC0 C9AF2000 */  lqv     $v15[0], (@@buffer1)
@@loop:
/* 04001DC4 000DC4 C9702000 */  lqv     $v16[0], (@@dmem_addr)
/* 04001DC8 000DC8 4A0E73AC */  vclr    $v14
/* 04001DCC 000DCC 4A000000 */  vmulf   $vzero, $vzero, $vzero  // 0                                        What about buffer1[0] ?
/* 04001DD0 000DD0 4B2FBB88 */  vmacf   $v14, $v23, $v15[1]     // += buffer2[0x1E..0x2E] * buffer1[1]      Indices on buffer1 and dmem_buf are u16 elements
/* 04001DD4 000DD4 4B4FB388 */  vmacf   $v14, $v22, $v15[2]     // += buffer2[0x1C..0x2C] * buffer1[2]
/* 04001DD8 000DD8 4B6FAB88 */  vmacf   $v14, $v21, $v15[3]     // += buffer2[0x1A..0x2A] * buffer1[3]
/* 04001DDC 000DDC 4B8FA388 */  vmacf   $v14, $v20, $v15[4]     // += buffer2[0x18..0x28] * buffer1[4]
/* 04001DE0 000DE0 4BAF9B88 */  vmacf   $v14, $v19, $v15[5]     // += buffer2[0x16..0x26] * buffer1[5]
/* 04001DE4 000DE4 4BCF9388 */  vmacf   $v14, $v18, $v15[6]     // += buffer2[0x14..0x24] * buffer1[6]
/* 04001DE8 000DE8 4BEF8B88 */  vmacf   $v14, $v17, $v15[7]     // += buffer2[0x12..0x22] * buffer1[7]
/* 04001DEC 000DEC 4B10C388 */  vmacf   $v14, $v24, $v16[0]     // += buffer2[0x10..0x20] * dmem_buf[0]
/* 04001DF0 000DF0 4B30CB88 */  vmacf   $v14, $v25, $v16[1]     // += buffer2[0x0E..0x1E] * dmem_buf[1]
/* 04001DF4 000DF4 4B50D388 */  vmacf   $v14, $v26, $v16[2]     // += buffer2[0x0C..0x1C] * dmem_buf[2]
/* 04001DF8 000DF8 4B70DB88 */  vmacf   $v14, $v27, $v16[3]     // += buffer2[0x0A..0x1A] * dmem_buf[3]
/* 04001DFC 000DFC 4B90E388 */  vmacf   $v14, $v28, $v16[4]     // += buffer2[0x08..0x18] * dmem_buf[4]
/* 04001E00 000E00 4BB0EB88 */  vmacf   $v14, $v29, $v16[5]     // += buffer2[0x06..0x16] * dmem_buf[5]
/* 04001E04 000E04 4BD0F388 */  vmacf   $v14, $v30, $v16[6]     // += buffer2[0x04..0x14] * dmem_buf[6]
/* 04001E08 000E08 4BF0FB88 */  vmacf   $v14, $v31, $v16[7]     // += buffer2[0x02..0x12] * dmem_buf[7]
/* 04001E0C 000E0C 21EFFFF0 */  addi    @@count, @@count, -0x10
/* 04001E10 000E10 E96E2000 */  sqv     $v14[0], (@@dmem_addr)
/* 04001E14 000E14 216B0010 */  addi    @@dmem_addr, @@dmem_addr, 0x10
/* 04001E18 000E18 1DE0FFEA */  bgtz    @@count, @@loop
/* 04001E1C 000E1C 4A1003D4 */   vaddc  $v15, $vzero, $v16      // move dmem_buf to buffer1
/* 04001E20 000E20 E9B02000 */  sqv     $v16[0], (@@buffer1)    // store last dmem_buf to buffer1
/* 04001E24 000E24 090006B1 */  j       dma_write_and_nextcmd
/* 04001E28 000E28 2003001F */   li     $3, 0x20 - 1

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Count           | Unused Parameter                | DMEM In                         | DMEM Out                        |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 *  Additive Mixer
 */
cmd_ADDMIXER:
    @@count    equ $18
    @@dmem_in  equ $20
    @@dmem_out equ $19

    vaddc   $v31, $v31, $v31
    srl     @@count, cmd_w0, 0xc
    andi    @@count, @@count, 0xff0
    andi    @@dmem_out, cmd_w1, 0xffff
    srl     @@dmem_in, cmd_w1, 0x10
    lqv     $v27[0], 0x00(@@dmem_out)
@@loop:
    lqv     $v29[0], 0x00(@@dmem_in)
    lqv     $v26[0], 0x10(@@dmem_out)
    lqv     $v28[0], 0x10(@@dmem_in)
    lqv     $v25[0], 0x20(@@dmem_out)
    lqv     $v23[0], 0x20(@@dmem_in)
    lqv     $v24[0], 0x30(@@dmem_out)
    lqv     $v22[0], 0x30(@@dmem_in)
    addi    @@dmem_in, @@dmem_in, 0x40
    vadd    $v27, $v27, $v29            // out + in
    vadd    $v26, $v26, $v28
    vadd    $v25, $v25, $v23
    vadd    $v24, $v24, $v22
    addi    @@count, @@count, -0x40
    sqv     $v27[0], 0x00(@@dmem_out)
    sqv     $v26[0], 0x10(@@dmem_out)
    sqv     $v25[0], 0x20(@@dmem_out)
    sqv     $v24[0], 0x30(@@dmem_out)
    addi    @@dmem_out, @@dmem_out, 0x40
    bgtz    @@count, @@loop
     lqv    $v27[0], (@@dmem_out)
    j       next_cmd
     nop

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Empty           | Pitch                           | Empty                           | Start Offset                    |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 * Zero-Order Hold Resampling
 */
cmd_RESAMPLE_ZOH:
    @@count      equ $13
    @@in_buffer  equ $14
    @@out_buffer equ $15
    @@pitch      equ $12
    @@position   equ $10

    lh      @@in_buffer, (audio_in_buf)(audioStructPtr)
    lh      @@out_buffer, (audio_out_buf)(audioStructPtr)
    lh      @@count, (audio_count)(audioStructPtr)
    andi    @@pitch, cmd_w0, 0xffff
    sll     @@pitch, @@pitch, 2
    andi    @@position, cmd_w1, 0xffff
    sll     @@in_buffer, @@in_buffer, 0x10
    or      @@position, @@position, @@in_buffer     // position = (in_buffer << 0x10) | (start offset)
@@loop:
    srl     $11, @@position, 0x10
    andi    $11, $11, ~1
    lsv     $v1[0], ($11)                           // load from (position >> 0x10 & ~1)
    add     @@position, @@position, @@pitch         // position += pitch
    srl     $11, @@position, 0x10
    andi    $11, $11, ~1
    lsv     $v1[2], ($11)                           // load from (position >> 0x10 & ~1)
    add     @@position, @@position, @@pitch         // position += pitch
    srl     $11, @@position, 0x10
    andi    $11, $11, ~1
    lsv     $v1[4], ($11)                           // load from (position >> 0x10 & ~1)
    add     @@position, @@position, @@pitch         // position += pitch
    srl     $11, @@position, 0x10
    andi    $11, $11, ~1
    lsv     $v1[6], ($11)                           // load from (position >> 0x10 & ~1)
    add     @@position, @@position, @@pitch         // position += pitch
    addi    @@count, @@count, -8
    sdv     $v1[0], (@@out_buffer)                  // store 8 bytes (4 samples) to out
    addi    @@out_buffer, @@out_buffer, 8
    bgtz    @@count, @@loop
     nop
    jal     next_cmd            //! oversight? should just be "j"? $ra isn't saved and is quickly overwritten so this never returns?
     nop

/**
 * |                 |                 |                                 |                                 |                                 |
 * | Command         | Empty           | Count                           | DMEM In                         | DMEM Out                        |
 * | 8               | 8               | 16                              | 16                              | 16                              |
 * | 31           24 | 23           16 | 15                            0 | 31                           16 | 15                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_UNK3:
    @@count    equ $18
    @@dmem_in  equ $20
    @@dmem_out equ $19

    lqv     $v31[0], (data_0000)($zero)
    andi    @@count, cmd_w0, 0xffff
    andi    @@dmem_out, cmd_w1, 0xffff
    srl     @@dmem_in, cmd_w1, 0x10
    lqv     $v29[0], 0x00(@@dmem_in)
    lqv     $v28[0], 0x10(@@dmem_in)
    lqv     $v27[0], 0x00(@@dmem_out)
    lqv     $v26[0], 0x10(@@dmem_out)
@@loop:
    addi    @@count, @@count, -0x20
    addi    @@dmem_in, @@dmem_in, 0x20
    vmudn   $v27, $v28, $v29            // out[0x10..0x20] = in[0x10..0x20] * in[0x00..0x10]
    vmadn   $v26, $v28, $v29            // out[0x00..0x10] = in[0x10..0x20] * in[0x00..0x10] + in[0x10..0x20] * in[0x00..0x10]
    sqv     $v27[0], 0x10(@@dmem_out)
    sqv     $v26[0], 0x00(@@dmem_out)
    lqv     $v29[0], 0x00(@@dmem_in)
    lqv     $v28[0], 0x10(@@dmem_in)
    bgtz    @@count, @@loop
     addi   @@dmem_out, @@dmem_out, 0x20
    j       next_cmd
     nop

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Size            | Unknown                         | DRAM Address                                                    |
 * | 8               | 8               | 16                              | 32                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 * Appears to be some kind of debug feature?
 */
cmd_UNK0:
    jal     func_04001A6C
     nop
    move    $1, tmpDataPtr
    jal     dma_read_start
     addi   $3, $3, -1
    lw      $6, 4($1)           // increment offset 4
    addi    $6, $6, 1
    sw      $6, 4($1)
    sw      taskDataSize, 8($1)         // store task_size to offset 8
    sw      cmd_buf_remaining, 0xc($1)  // store current alist buffer size to offset 0xc
    andi    $6, cmd_w0, 0xffff
    jal     dma_write_start     // write this struct out to dram
     sh     $6, ($1)            // store unknown value to offset 0
    j       next_cmd
     nop

.align 0x10

.if . > IMEM_END_VIRT
    .error "Not enough room in IMEM"
.endif

.close
