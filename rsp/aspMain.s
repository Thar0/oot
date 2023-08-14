.rsp
#include "rsp.inc"
#include "rcp.h"
#include "abi.h"

// Flags
#define A_ADPCM_SHORT 4

// Frame size
#define ADPCMFSIZE (0x10 * 2)

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
useful_constants:
.dh 0x0000, 0x0001, 0x0002, 0xFFFF, 0x0020, (1 << 11), 0x7FFF, 0x4000

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
adpcm_dec_masks:
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
// related to resampling
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
abi_cmd_buffer:
    .skip 0x40

// 0x0330
// stores the adpcm codebook, maximum space available is order=2 npredictors=4 or equivalent
adpcmCodebook:
    .skip 9 * 0x10

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

g_rampLeft      equ $21 // Clobbered by cmd_ADPCM and cmd_RESAMPLE
g_rampRight     equ $22 // Clobbered by cmd_RESAMPLE
gv_envSettings  equ $v1 // Clobbered by cmd_DMEMMOVE, cmd_UNK16, cmd_DUPLICATE, cmd_INTERL, cmd_HILOGAIN, cmd_RESAMPLE_ZOH

// DMA function args
DMA_MEM_ADDR equ $1
DMA_DRAM_ADDR equ $2
DMA_LENGTH equ $3

audio_entry:
    ori     $10, $zero, OSTask_addr
    lw      DMA_DRAM_ADDR, OS_TASK_OFF_UDATA($10)
    lw      DMA_LENGTH, OS_TASK_OFF_UDATA_SZ($10)
    mtc0    $zero, SP_SEMAPHORE             // release semaphore
    jal     dma_read_start
     li     DMA_MEM_ADDR, 0
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
    andi    $1, $1, 0xFE
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

// DMAs the acmd list to DMEM, placing it at abi_cmd_buffer
load_acmd_list:
    move    $5, $ra                 // save $ra in $5
    add     DMA_DRAM_ADDR, $zero, taskDataPtr
    move    DMA_LENGTH, taskDataSize
    addi    $4, DMA_LENGTH, -0x40
    blez    $4, @@stepover          // branch forward if there is less than 0x40 bytes worth of commands left
     li     DMA_MEM_ADDR, abi_cmd_buffer
    li      DMA_LENGTH, 0x40        // load full 0x40
@@stepover:
    move    cmd_buf_remaining, DMA_LENGTH
    jal     dma_read_start
     addi   DMA_LENGTH, DMA_LENGTH, -1
    jr      $5                      // return with stored $ra
     li     cmd_ptr, abi_cmd_buffer

dma_read:
    // spin until the semaphore is acquired
@@wait_semaphore:
    mfc0    $4, SP_SEMAPHORE
    bnez    $4, @@wait_semaphore
     nop
    // wait until dma not full
@@dma_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_full
     nop
    // start the transfer
    mtc0    DMA_MEM_ADDR, SP_MEM_ADDR
    mtc0    DMA_DRAM_ADDR, SP_DRAM_ADDR
    mtc0    DMA_LENGTH, SP_RD_LEN
    // do not wait for completion (?)
    jr      $ra
     nop

dma_write:
    // spin until the semaphore is acquired
@@wait_semaphore:
    mfc0    $4, SP_SEMAPHORE
    bnez    $4, @@wait_semaphore
     nop
    // wait until dma not full
@@dma_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_full
     nop
    // start the transfer
    mtc0    DMA_MEM_ADDR, SP_MEM_ADDR
    mtc0    DMA_DRAM_ADDR, SP_DRAM_ADDR
    mtc0    DMA_LENGTH, SP_WR_LEN
    // do not wait for completion (?)
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

    andi    @@count, cmd_w1, 0xFFFF     // load count
    beqz    @@count, next_cmd           // 0 count, leave early
     andi   @@mem_addr, cmd_w0, 0xFFFF  // load dmem addr
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

    andi    @@dmem_out, cmd_w0, 0xFFFF
    srl     @@count, cmd_w0, 0xC
    andi    @@count, @@count, 0xFF0             // count << 4
    andi    @@right_in, cmd_w1, 0xFFFF
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

    andi    @@count, cmd_w1, 0xFFFF     // count
    beqz    @@count, next_cmd           // 0 count, leave early
     andi   @@dmem_in, cmd_w0, 0xFFFF
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
    @@isShortADPCM      equ $8
    @@adpcmBookPtr      equ $15
    @@count             equ $18
    @@outBuffer         equ $19
    @@inBuffer          equ $21
    @@flags             equ $1
    @@bytesPerFrame     equ $10
    @@pagePtr           equ $13
    @@pagePtr2          equ $12
    @@frameHeader       equ $1
    @@pageNumber        equ $11

    @@frameData         equ $v1
    @@frameData0        equ $v3
    @@frameData1        equ $v4
    @@frameData2        equ $v5
    @@frameData3        equ $v6

    @@adpcmStateAddr    equ $17

    @@bookvec1          equ $v21
    @@bookvec2          equ $v20
    @@bookvec3          equ $v19
    @@bookvec4          equ $v18
    @@bookvec5          equ $v17
    @@bookvec6          equ $v16
    @@bookvec7          equ $v15
    @@bookvec8          equ $v14
    @@bookvec9          equ $v13

    @@output1           equ $v28
    @@output2           equ $v27

    @@resultTemp        equ $v26

    @@masks1            equ $v25
    @@masks2            equ $v24
    @@mulFactors        equ $v23

    @@v__               equ $v7
    @@v___              equ $v2

    @@svec1             equ $v30
    @@svec2             equ $v29

    @@scaleFactor       equ $4
    @@scaleFactorVec    equ $v22

    lqv     $v31[0], (useful_constants)($zero) // $v31 <- [0x0000, 0x0001, 0x0002, 0xFFFF, 0x0020, 0x0800, 0x7FFF, 0x4000]
    vclr    @@output2
    lhu     $21, (audio_in_buf)(audioStructPtr)
    vclr    @@masks1
    vclr    @@masks2
    addi    $20, $21, 1
    lhu     @@outBuffer, (audio_out_buf)(audioStructPtr)
    vclr    @@bookvec9
    vclr    @@bookvec8
    lhu     @@count, (audio_count)(audioStructPtr)  // load count
    vclr    @@bookvec7
    vclr    @@bookvec6
    sll     @@adpcmStateAddr, cmd_w1, 8
    vclr    @@bookvec5
    vclr    @@bookvec4
    srl     @@adpcmStateAddr, @@adpcmStateAddr, 8   // remove addr high byte
    vclr    @@bookvec3
    sqv     @@output2[0], 0x00(@@outBuffer)         // clear 0x20 bytes of outBuffer to 0
    sqv     @@output2[0], 0x10(@@outBuffer)
    li      $16, adpcm_dec_masks
    //  $16 + 0x00 : [0xF000, 0x0F00, 0x00F0, 0x000F]
    //  $16 + 0x08 : [0x0001, 0x0010, 0x0100, 0x1000]
    //  $16 + 0x10 : [0xC000, 0x3000, 0x0C00, 0x0300]
    //  $16 + 0x18 : [0x0001, 0x0004, 0x0010, 0x0040]
    li      @@adpcmBookPtr, adpcmCodebook
    srl     @@flags, cmd_w0, 0x10       // load flags
    andi    @@isShortADPCM, @@flags, A_ADPCM_SHORT
    beqz    @@isShortADPCM, @@stepover_short // if regular ADPCM, skip
     nop
    li      @@bytesPerFrame, 5          // $10 = 5 (bytes per frame)
    li      $9, 14                      // $9 = 14
    ldv     @@masks1[0], 0x10($16)      // masks1 = [0xC000, 0x3000, 0x0C00, 0x0300, 0x0000, 0x0000, 0x0000, 0x0000]
    ldv     @@mulFactors[0], 0x18($16)  // mulFactors = [0x0001, 0x0004, 0x0010, 0x0040, 0x0000, 0x0000, 0x0000, 0x0000]
    addi    $16, $16, -1                // $16--
    ldv     @@masks1[8], 0x10($16)      // masks1 = [0xC000, 0x3000, 0x0C00, 0x0300, 0x00C0, 0x0030, 0x000C, 0x0003]
    addi    $16, $16, 2                 // $16 += 2
    j       @@stepover_regular          // step over regular ADPCM handling
     ldv    @@mulFactors[8], 0x18($16)  // mulFactors = [0x0001, 0x0004, 0x0010, 0x0040, 0x0100, 0x0400, 0x1000, 0x40__]  (last byte is read from first byte of data_0060)
@@stepover_short:
    li      @@bytesPerFrame, 9          // $10 = 9 (bytes per frame)
    li      $9, 12                      // $9 = 12
    ldv     @@masks1[0], 0x0($16)       // masks1 = [0xF000, 0x0F00, 0x00F0, 0x000F, 0x0000, 0x0000, 0x0000, 0x0000]
    ldv     @@masks2[8], 0x0($16)       // masks2 = [0x0000, 0x0000, 0x0000, 0x0000, 0xF000, 0x0F00, 0x00F0, 0x000F]
    ldv     @@mulFactors[0], 0x8($16)
    ldv     @@mulFactors[8], 0x8($16)   // mulFactors = [0x0001, 0x0010, 0x0100, 0x1000, 0x0001, 0x0010, 0x0100, 0x1000]
@@stepover_regular:
    srl     @@flags, cmd_w0, 0x10
    andi    @@flags, @@flags, A_INIT
    bgtz    @@flags, @@is_init          // if (flags & 1) , skip
     srl    @@flags, cmd_w0, 0x10
    andi    @@flags, @@flags, A_LOOP
    beq     $zero, @@flags, @@no_loop   // if !(flags & 2) , skip
     move   DMA_DRAM_ADDR, @@adpcmStateAddr     // get from command addr
    lw      DMA_DRAM_ADDR, (audio_loop_addr)(audioStructPtr)    // or get from audio struct based on (flags & 2)
@@no_loop:
    // DMA saved adpcm state into the first 0x20 bytes of outBuffer
    move    DMA_MEM_ADDR, @@outBuffer
    jal     dma_read_start
     li     DMA_LENGTH, ADPCMFSIZE - 1
@@is_init:
    lqv     @@output2[0], 0x10(@@outBuffer)         // read last 0x10 bytes of the adpcm state, if A_INIT is set then this is 0
    addi    @@outBuffer, @@outBuffer, ADPCMFSIZE    // increment outBuffer by 0x20
    beqz    @@count, @@early_ret                    // 0 count, leave early
     ldv    @@frameData[0], ($20)                   // load frame data
    lbu     @@frameHeader, ($21)                    // $1 = header
    andi    @@pageNumber, @@frameHeader, 0xF        // pageNumber = optimalp = header & 0xF        (codebook page number)
    sll     @@pageNumber, @@pageNumber, 5           // pageNumber *= 32
    vand    @@frameData0, @@masks1, @@frameData[0]  // both ADPCM
    add     @@pagePtr, @@pageNumber, @@adpcmBookPtr // get pointer to specific codebook page
    vclr    @@frameData1
    vclr    @@frameData3
    bnez    @@isShortADPCM, @@skip_masks1           // if short ADPCM, skip
     vand   @@frameData2, @@masks1, @@frameData[1]  // only short ADPCM (overwritten below if regular)
    vand    @@frameData1, @@masks2, @@frameData[1]  // only regular ADPCM
    vand    @@frameData2, @@masks1, @@frameData[2]  // only regular ADPCM
    vand    @@frameData3, @@masks2, @@frameData[3]  // only regular ADPCM
@@skip_masks1: // scale calculation
    // FACTOR = $9 = 14 if short else 12
    srl     $14, @@frameHeader, 4       // $14 = header >> 4    = log2(scale)
    add     $2, $zero, $9               // $2                   = FACTOR
    sub     $14, $2, $14                // $14 = $2 - $14       = FACTOR - (header >> 4)
    addi    $2, $14, -1                 // $2 = $14 - 1         = (FACTOR - 1) - (header >> 4)
    ori     $3, $zero, 0x8000           // $3 = (1 << 0xF)
    beqz    $14, @@scale1_stepover      // if (FACTOR - (header >> 4) == 0)
     li     @@scaleFactor, -1           //     scale = -1
    srlv    @@scaleFactor, $3, $2       // else scale = 0x8000 >> $2 = 0x8000 >> ((FACTOR - 1) - (header >> 4))
@@scale1_stepover:
    // Store scale to vector register
    mtc2    @@scaleFactor, @@scaleFactorVec[0]
    // Load ADPCM book page into vector registers, creates the IIR filter matrix minus identity (transposed)
    //  [ c0  d0  0   0   0   0   0   0   0   0 ]
    //  [ c1  d1  d0  0   0   0   0   0   0   0 ]
    //  [ c2  d2  d1  d0  0   0   0   0   0   0 ]
    //  [ c3  d3  d2  d1  d0  0   0   0   0   0 ]
    //  [ c4  d4  d3  d2  d1  d0  0   0   0   0 ]
    //  [ c5  d5  d4  d3  d2  d1  d0  0   0   0 ]
    //  [ c6  d6  d5  d4  d3  d2  d1  d0  0   0 ]
    //  [ c7  d7  d6  d5  d4  d3  d2  d1  d0  0 ]
    // where
    // c0...c7 / d0...d7 are codebook predictors
    lqv     @@bookvec1[0], 0x00(@@pagePtr) // loads [c0..c7] column 1
    lqv     @@bookvec2[0], 0x10(@@pagePtr) // loads [d0..d7] column 2
    addi    @@pagePtr, @@pagePtr, -2
    lrv     @@bookvec3[0], 0x20(@@pagePtr) // loads [0,d0..d6] column 3
    addi    @@pagePtr, @@pagePtr, -2
    lrv     @@bookvec4[0], 0x20(@@pagePtr) // loads [0,0,d0..d5] column 4
    addi    @@pagePtr, @@pagePtr, -2
    lrv     @@bookvec5[0], 0x20(@@pagePtr) // loads [0,0,0,d0..d4] column 5
    addi    @@pagePtr, @@pagePtr, -2
    lrv     @@bookvec6[0], 0x20(@@pagePtr) // loads [0,0,0,0,d0..d3] column 6
    addi    @@pagePtr, @@pagePtr, -2
    lrv     @@bookvec7[0], 0x20(@@pagePtr) // loads [0,0,0,0,0,d0..d2] column 7
    addi    @@pagePtr, @@pagePtr, -2
    lrv     @@bookvec8[0], 0x20(@@pagePtr) // loads [0,0,0,0,0,0,d0,d1] column 8
    addi    @@pagePtr, @@pagePtr, -2
    lrv     @@bookvec9[0], 0x20(@@pagePtr) // loads [0,0,0,0,0,0,0,d0] column 9
    // column 10 does not need loading as it is all 0
@@main_loop:
    // (SU) incr by bytes per frame and load frame data, (VU) prepare svectors by scaling unpacked frame data
    add     $20, $20, @@bytesPerFrame   ::  vmudn   @@svec1, @@frameData0, @@mulFactors  // mulFactors = [0x0001, 0x0010, 0x0100, 0x1000, 0x0001, 0x0010, 0x0100, 0x1000]
    add     $21, $21, @@bytesPerFrame   ::  vmadn   @@svec1, @@frameData1, @@mulFactors  // svec1 = @@frameData0 + @@mulFactors + @@frameData1 * @@mulFactors
    ldv     @@frameData[0], ($20)       ::  vmudn   @@svec2, @@frameData2, @@mulFactors
    lbu     @@frameHeader, ($21)        ::  vmadn   @@svec2, @@frameData3, @@mulFactors  // svec2 = @@frameData2 * @@mulFactors + @@frameData3 * @@mulFactors
    blez    $14, @@skip_scale_mul
     andi   @@pageNumber, @@frameHeader, 0xF        // get page number for new frame
    vmudm   @@svec1, @@svec1, @@scaleFactorVec[0]   // *= scale
    vmudm   @@svec2, @@svec2, @@scaleFactorVec[0]   // *= scale
@@skip_scale_mul:
    sll     @@pageNumber, @@pageNumber, 5           // pageNumber *= 32
    // unpack frame data
    vand    @@frameData0, @@masks1, @@frameData[0]  // both ADPCM
    add     @@pagePtr, @@pageNumber, @@adpcmBookPtr // pointer to new codebook page
    bnez    @@isShortADPCM, @@skip_masks2           // if short ADPCM, skip
     vand   @@frameData2, @@masks1, @@frameData[1]  // only short ADPCM
    vand    @@frameData1, @@masks2, @@frameData[1]  // only regular ADPCM
    vand    @@frameData2, @@masks1, @@frameData[2]  // only regular ADPCM
    vand    @@frameData3, @@masks2, @@frameData[3]  // only regular ADPCM
@@skip_masks2:
    // (SU) another scale calculation,      (VU) Matrix multiplication between the matrix formed by [bookvec1..bookvec7,0] with the vector [p6, p7, s0..s7]
    // $14 = header >> 4
    srl     $14, @@frameHeader, 4       ::  vmudh   @@v___, @@bookvec1, @@output2[6] // p6, a previously decoded sample
    // $2 = $9 = FACTOR
    add     $2, $zero, $9               ::  vmadh   @@v___, @@bookvec2, @@output2[7] // p7, a previously decoded sample
    // $14 = $2 - $14 = FACTOR - (header >> 4)
    sub     $14, $2, $14                ::  vmadh   @@v___, @@bookvec3, @@svec1[0] // s0
    // $2 = $14 - 1 = (FACTOR - 1) - (header >> 4)
    addi    $2, $14, -1                 ::  vmadh   @@v___, @@bookvec4, @@svec1[1] // s1
    // $3 = 1
    li      $3, 1                       ::  vmadh   @@v___, @@bookvec5, @@svec1[2] // s2
    // $3 = $3 << 0xF = 1 << 0xF
    sll     $3, $3, 0xF                 ::  vmadh   @@v___, @@bookvec6, @@svec1[3] // s3
    beqz    $14, @@scale2_stepover      // if (FACTOR - (header >> 4) == 0)
     li     @@scaleFactor, -1           //     scale = -1
    srlv    @@scaleFactor, $3, $2       // else scale = $3 >> $2 = 0x8000 >> ((FACTOR - 1) - (header >> 4))
@@scale2_stepover:
    // Finish the matrix multiplication and load new scale factor
    vmadh   @@output1, @@bookvec7, @@svec1[4]       // s4
    mtc2    @@scaleFactor, @@scaleFactorVec[0]      // scaleFactorVec[0] = scaleFactor
    vmadh   @@v___, @@bookvec8, @@svec1[5]          // s5
    vmadh   @@v___, @@bookvec9, @@svec1[6]          // s6
    // Add identityMatrix * svec1
    vmadh   @@v___, @@svec1, $v31[5]                // $v31[5] = 1 << 11         result += svec1 * 1.0(4.11)       also covers s7
    vsar    @@resultTemp, @@v__, @@output1[1]       // resultTemp <- ACC_MID
    vsar    @@output1, @@v__, @@output1[0]          // output1 <- ACC_LO
    vmudn   @@v___, @@resultTemp, $v31[4]           // $v31[4] = 0x0020
    vmadh   @@output1, @@output1, $v31[4]           // $v31[4] = 0x0020
    // (VU) Another matrix multiplication           (SU) Load next book data as soon as possible
    vmudh   @@v___, @@bookvec3, @@svec2[0]      ::  addi    @@pagePtr2, @@pagePtr, -2
    vmadh   @@v___, @@bookvec4, @@svec2[1]      ::  lrv     @@bookvec3[0], 0x20(@@pagePtr2)
    vmadh   @@v___, @@bookvec5, @@svec2[2]      ::  addi    @@pagePtr2, @@pagePtr2, -2
    vmadh   @@v___, @@bookvec6, @@svec2[3]      ::  lrv     @@bookvec4[0], 0x20(@@pagePtr2)
    vmadh   @@v___, @@bookvec7, @@svec2[4]      ::  addi    @@pagePtr2, @@pagePtr2, -2
    vmadh   @@v___, @@bookvec8, @@svec2[5]      ::  lrv     @@bookvec5[0], 0x20(@@pagePtr2)
    vmadh   @@v___, @@bookvec9, @@svec2[6]      ::  addi    @@pagePtr2, @@pagePtr2, -2
    // $v31[5] = 0x0800
    vmadh   @@v___, @@svec2, $v31[5]            ::  lrv     @@bookvec6[0], 0x20(@@pagePtr2)
    vmadh   @@v___, @@bookvec1, @@output1[6]    ::  addi    @@pagePtr2, @@pagePtr2, -2
    vmadh   @@v___, @@bookvec2, @@output1[7]    ::  lrv     @@bookvec7[0], 0x20(@@pagePtr2)
    // resultTemp <- ACC_MID
    vsar    @@resultTemp, @@v__, @@output2[1]   ::  addi    @@pagePtr2, @@pagePtr2, -2
    // output2 <- ACC_LO
    vsar    @@output2, @@v__, @@output2[0]      ::  lrv     @@bookvec8[0], 0x20(@@pagePtr2)
    // Finish loading the next book page
    addi    @@pagePtr2, @@pagePtr2, -2
    lrv     @@bookvec9[0], 0x20(@@pagePtr2)
    lqv     @@bookvec1[0], 0x00(@@pagePtr)      ::  vmudn   @@v___, @@resultTemp, $v31[4]   // $v31[4] = 0x0020
    lqv     @@bookvec2[0], 0x10(@@pagePtr)      ::  vmadh   @@output2, @@output2, $v31[4]   // $v31[4] = 0x0020
    addi    @@count, @@count, -ADPCMFSIZE       // decr count
    // Save output to destination buffer
    sdv     @@output1[0], 0x00(@@outBuffer)
    sdv     @@output1[8], 0x08(@@outBuffer)
    sdv     @@output2[0], 0x10(@@outBuffer)
    sdv     @@output2[8], 0x18(@@outBuffer)
    bgtz    @@count, @@main_loop                // loop until done
     addi   @@outBuffer, @@outBuffer, ADPCMFSIZE
@@early_ret:
    // Save the most recent output data to the adpcm state to resume further frame decoding later
    addi    DMA_MEM_ADDR, @@outBuffer, -ADPCMFSIZE
    move    DMA_DRAM_ADDR, @@adpcmStateAddr
    j       dma_write_and_nextcmd
     li     DMA_LENGTH, ADPCMFSIZE

RESAMPLE_STATE_SIZE equ 32
/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Flags           | Pitch                           | Address (pointer to s16[16])                                    |
 * | 8               | 8               | 16                              | 32                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_RESAMPLE:
    @@in_buf    equ $8
    @@out_buf   equ $19 // 16 byte alignment assumed
    @@count     equ $18
    @@flags     equ $7

    @@dram_addr_save equ $22
    @@v_resample_state equ $v16

    lh      @@in_buf, (audio_in_buf)(audioStructPtr)
    lh      @@out_buf, (audio_out_buf)(audioStructPtr)
    lh      @@count, (audio_count)(audioStructPtr)
    sll     DMA_DRAM_ADDR, cmd_w1, 8
    srl     DMA_DRAM_ADDR, DMA_DRAM_ADDR, 8     // remove addr high byte
    move    DMA_MEM_ADDR, tmpDataPtr
    add     @@dram_addr_save, $zero, DMA_DRAM_ADDR
    li      DMA_LENGTH, RESAMPLE_STATE_SIZE - 1
    srl     @@flags, cmd_w0, 0x10
    andi    $10, @@flags, A_INIT
    bgtz    $10, @@resample_init                // if (flags & A_INIT) , skip
     nop
    jal     dma_read_start
     nop
    j       @@no_init                           // step over (flags & A_INIT)
     nop
@@resample_init:
    sh      $zero, 8(tmpDataPtr)                // zero pitch accumulator
    vclr    @@v_resample_state                  // zero resample state
    sdv     @@v_resample_state[0], (tmpDataPtr) // store state
@@no_init:
    andi    $10, @@flags, A_LOOP
    beqz    $10, @@no_loop        // if !(flags & A_LOOP) , skip
     ldv    @@v_resample_state[0], (tmpDataPtr) // load resample state
    addi    @@in_buf, @@in_buf, -4
    ssv     @@v_resample_state[0], 0x0(@@in_buf)
    ssv     @@v_resample_state[4], 0x2(@@in_buf)
    j       @@do_resample
     nop
@@no_loop:
    andi    $10, @@flags, A_ADPCM_SHORT
    beqz    $10, @@not_short        // if !(flags & A_ADPCM_SHORT) , skip
     nop
    addi    @@in_buf, @@in_buf, -0x10
    ssv     @@v_resample_state[0], 0x0(@@in_buf)
    ssv     @@v_resample_state[0], 0x2(@@in_buf)
    ssv     @@v_resample_state[2], 0x4(@@in_buf)
    ssv     @@v_resample_state[2], 0x6(@@in_buf)
    ssv     @@v_resample_state[4], 0x8(@@in_buf)
    ssv     @@v_resample_state[4], 0xA(@@in_buf)
    ssv     @@v_resample_state[6], 0xC(@@in_buf)
    ssv     @@v_resample_state[6], 0xE(@@in_buf)
    j       @@do_resample
     nop
@@not_short:
    addi    @@in_buf, @@in_buf, -8
    sdv     @@v_resample_state[0], (@@in_buf)
@@do_resample:
    lsv     $v23[14], 0x8(tmpDataPtr)   // $v23[7] = pitch_accumulator
    ldv     @@v_resample_state[0], (@@in_buf)
    mtc2    @@in_buf, $v18[4]           // $v18[2] <- &in_buf
    li      $10, data_00E0
    mtc2    $10, $v18[6]                // $v18[3] <- data_00E0
    mtc2    cmd_w0, $v18[8]             // $v18[4] <- pitch
    li      $10, 64
    mtc2    $10, $v18[10]               // $v18[5] <- 64
    li      $9, data_0060
    lqv     $v31[0], 0x10($9)           // $v31 <- [1, 1, 1, 1, 1, 1, 1, 1]
    lqv     $v25[0], 0x00($9)           // $v25 <- [2, 4, 6, 8, 10, 12, 14, 16]
    vsub    $v25, $v25, $v31            // $v25 = $v25 - $v31 = [1, 3, 5, 7, 9, 11, 13, 15]
    lqv     $v30[0], 0x20($9)           // $v30 <- [0, 1, 2, 4, 8, 16, 256, 512]
    lqv     $v29[0], 0x30($9)           // $v29 <- [1, 0, 0, 0, 1, 0, 0, 0]
    lqv     $v28[0], 0x40($9)           // $v28 <- [0, 1, 0, 0, 0, 1, 0, 0]
    lqv     $v27[0], 0x50($9)           // $v27 <- [0, 0, 1, 0, 0, 0, 1, 0]
    lqv     $v26[0], 0x60($9)           // $v26 <- [0, 0, 0, 1, 0, 0, 0, 1]
    vsub    $v25, $v25, $v31            // $v25 = $v25 - $v31 = [0, 2, 4, 6, 8, 10, 12, 14]
    lqv     $v24[0], 0x70($9)           // $v24 <- [0x2000, 0x4000, 0x6000, 0x8000, 0xA000, 0xC000, 0xE000, 0xFFFF]
    addi    $21, tmpDataPtr, RESAMPLE_STATE_SIZE + 0x00
    addi    $20, tmpDataPtr, RESAMPLE_STATE_SIZE + 0x10
    vclr    $v22
    vmudm   $v23, $v31, $v23[7]         // $v23 = ($ACC  = [pitch_accumulator ... x8]) >> 16
    vmadm   $v22, $v25, $v18[4]         // $v22 = ($ACC += ([0, 2, 4, 6, 8, 10, 12, 14] * pitch)) >> 16
    vmadn   $v23, $v31, $v30[0]         // $v23 = ($ACC += 0) & 0xFFFF
    vmudn   $v21, $v31, $v18[2]         // $v21 = ($ACC  = [&in_buf ... x8]) & 0xFFFF
    vmadn   $v21, $v22, $v30[2]         // $v21 = ($ACC += $v22 * 2) & 0xFFFF
    vmudl   $v17, $v23, $v18[5]         // $v17 = ($ACC  = ($v23 * 64) >> 16) & 0xFFFF
    vmudn   $v17, $v17, $v30[4]         // $v17 = ($ACC  = $v17 * 8) & 0xFFFF
    vmadn   $v17, $v31, $v18[3]         // $v17 = ($ACC += [data_00E0 ... x8]) & 0xFFFF
    lqv     $v25[0], 0x00($9)           // $v25 <- [2, 4, 6, 8, 10, 12, 14, 16]
    sqv     $v21[0], ($21)              // input buffer addresses
    sqv     $v17[0], ($20)              // data_00E0 addresses
    ssv     $v23[7], 0x8(tmpDataPtr)    // store pitch_accumulator
    lh      $17,  0($21)                // load every address computed above
    lh      $9,   0($20)
    lh      $13,  8($21)
    lh      $5,   8($20)
    lh      $16,  2($21)
    lh      $8,   2($20)
    lh      $12, 10($21)
    lh      $4,  10($20)
    lh      $15,  4($21)
    lh      $7,   4($20)
    lh      $11, 12($21)
    lh      $3,  12($20)
    lh      $14,  6($21)
    lh      $6,   6($20)
    lh      $10, 14($21)
    lh      $2,  14($20)
@@loop:
    // This loop is split into two halves, in each half the SU and VU perform different tasks.
    // First half
    // SU: Loads data from addresses        VU: Computes next addresses
    ldv     $v16[0], ($17)              ::  vmudm   $v23, $v31, $v23[7]     // $v23 = ($ACC  = [pitch_accumulator ... x8]) >> 16
    ldv     $v15[0], ($9)               ::  vmadh   $v23, $v31, $v22[7]     // $v23 = ($ACC += $v22[7] << 16) >> 16
    ldv     $v16[8], ($13)              ::  vmadm   $v22, $v25, $v18[4]     // $v22 = ($ACC += [2, 4, 6, 8, 10, 12, 14, 16] * pitch) >> 16
    ldv     $v15[8], ($5)               ::  vmadn   $v23, $v31, $v30[0]     // $v23 = ($ACC += 0) & 0xFFFF
    ldv     $v14[0], ($16)              ::  vmudn   $v21, $v31, $v18[2]     // $v21 = ($ACC  = [&in_buf ... x8]) & 0xFFFF
    ldv     $v13[0], ($8)               ::  vmadn   $v21, $v22, $v30[2]     // $v21 = ($ACC += $v22 * 2) & 0xFFFF
    ldv     $v14[8], ($12)              ::  vmudl   $v17, $v23, $v18[5]     // $v17 = ($ACC  = ($v23 * 64) >> 16) & 0xFFFF
    ldv     $v13[8], ($4)
    ldv     $v12[0], ($15)
    ldv     $v11[0], ($7)
    ldv     $v12[8], ($11)              ::  vmudn   $v17, $v17, $v30[4]     // $v17 = ($ACC  = $v17 * 8) & 0xFFFF
    ldv     $v11[8], ($3)
    ldv     $v10[0], ($14)
    ldv     $v9[0], ($6)                ::  vmadn   $v17, $v31, $v18[3]     // $v17 = ($ACC += [data_00E0 ... x8]) & 0xFFFF
    ldv     $v10[8], ($10)              ::  vmulf   $v8, $v16, $v15         // $v8 = $v16 * $v15
    ldv     $v9[8], ($2)                ::  vmulf   $v7, $v14, $v13         // $v7 = $v14 * $v13
    sqv     $v21[0], ($21)              ::  vmulf   $v6, $v12, $v11         // $v6 = $v12 * $v11
    sqv     $v17[0], ($20)
    // Second half
    // SU: Loads next addresses             VU: Computes results from loaded data
    lh      $17,  0($21)                ::  vmulf   $v5, $v10, $v9          // $v5 = $v10 * $v9
    lh      $9,   0($20)                ::  vadd    $v8, $v8, $v8[1q]       // $v8 += $v8[1q]
    lh      $13,  8($21)                ::  vadd    $v7, $v7, $v7[1q]
    lh      $5,   8($20)                ::  vadd    $v6, $v6, $v6[1q]
    lh      $16,  2($21)                ::  vadd    $v5, $v5, $v5[1q]
    lh      $8,   2($20)                ::  vadd    $v8, $v8, $v8[2h]       // $v8 += $v8[2h]
    lh      $12, 10($21)                ::  vadd    $v7, $v7, $v7[2h]
    lh      $4,  10($20)                ::  vadd    $v6, $v6, $v6[2h]
    lh      $15,  4($21)                ::  vadd    $v5, $v5, $v5[2h]
    lh      $7,   4($20)                ::  vmudn   $v4, $v29, $v8[0h]      // $v4 = ($ACC  = [1, 0, 0, 0, 1, 0, 0, 0] * $v8[0h]) & 0xFFFF
    lh      $11, 12($21)                ::  vmadn   $v4, $v28, $v7[0h]      // $v4 = ($ACC += [0, 1, 0, 0, 0, 1, 0, 0] * $v7[0h]) & 0xFFFF
    lh      $3,  12($20)                ::  vmadn   $v4, $v27, $v6[0h]      // $v4 = ($ACC += [0, 0, 1, 0, 0, 0, 1, 0] * $v6[0h]) & 0xFFFF
    lh      $14,  6($21)                ::  vmadn   $v4, $v26, $v5[0h]      // $v4 = ($ACC += [0, 0, 0, 1, 0, 0, 0, 1] * $v5[0h]) & 0xFFFF
    lh      $6,   6($20)
    lh      $10, 14($21)
    // End
    addi    @@count, @@count, -0x10
    sqv     $v4[0], (@@out_buf)         // store results
    blez    @@count, @@resample_done
     lh     $2, 14($20)
    j       @@loop                      // until done
     addi   @@out_buf, @@out_buf, 0x10
@@resample_done:
    ssv     $v23[0], 0x8(tmpDataPtr)    // save pitch accumulator
    ldv     $v16[0], ($17)
    sdv     $v16[0], (tmpDataPtr)       // resample state
    add     DMA_DRAM_ADDR, $zero, @@dram_addr_save
    move    DMA_MEM_ADDR, tmpDataPtr    // write the new resample state back to DRAM
    j       dma_write_and_nextcmd
     li     DMA_LENGTH, RESAMPLE_STATE_SIZE

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
    andi    @@count2, @@count2, 0xFF
    andi    @@dmem_in, cmd_w0, 0xFFFF
    srl     @@dmem_out, cmd_w1, 0x10
@@loop_outer:
    addi    @@count2, @@count2, -1      // decr count2
    andi    @@count, cmd_w1, 0xFFFF     // (re)load count
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
 * Copies `dmem_in` to `dmem_out` `count` times. DMEM in and DMEM out should be 0x10 byte aligned
 */
cmd_DUPLICATE:
    @@count    equ $15
    @@dmem_in  equ $13
    @@dmem_out equ $14

    srl     @@count, cmd_w0, 0x10
    andi    @@count, @@count, 0xFF
    andi    @@dmem_in, cmd_w0, 0xFFFF
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

    andi    @@count, cmd_w0, 0xFFFF
    andi    @@dmem_out, cmd_w1, 0xFFFF
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
    @@env_state_addr    equ $19
    @@count             equ $20
    @@swapLR            equ $10
    @@left_addr         equ $14
    @@right_addr        equ $15
    @@wet_left_addr     equ $16
    @@wet_right_addr    equ $17

    @@left_data_1       equ $v18
    @@right_data_1      equ $v19

    @@wet_left_data     equ $v13
    @@wet_right_data    equ $v14

    @@envState0         equ $v8
    @@envState1         equ $v15

    @@rampDelta         equ $v4

    @@vStereoEffects    equ $v2

    @@leftAdd           equ $v9
    @@rightAdd          equ $v10

    @@leftRampAdd       equ $v16
    @@rightRampAdd      equ $v17

    @@leftData          equ $v11
    @@rightData         equ $v12

    @@wet_left_result   equ $v20
    @@wet_right_result  equ $v21

    ENVMIXER_STATE_SIZE equ (2 * 0x10)

    vclr    @@rampDelta
    vclr    $vzero
    lqv     $v3[0], (useful_constants)($zero)   // Unused?

    add     g_rampLeft, g_rampLeft, g_rampLeft      // g_rampLeft += g_rampLeft
    mtc2    g_rampLeft, @@rampDelta[0]              // g_rampLeft
    mtc2    g_rampLeft, @@rampDelta[2]              // g_rampLeft

    // Extract DMEM addr
    srl     $12, cmd_w0, 0xC
    andi    @@env_state_addr, $12, 0xFF0

    add     g_rampRight, g_rampRight, g_rampRight   // g_rampRight += g_rampRight
    mtc2    g_rampRight, @@rampDelta[4]             // g_rampRight
    mtc2    g_rampRight, @@rampDelta[6]             // g_rampRight

    // Extract left
    srl     $12, cmd_w1, 0x14
    andi    @@left_addr, $12, 0xFF0

    add     $11, $11, $11               // rampReverb += rampReverb
    mtc2    $11, @@rampDelta[8]         // rampReverb
    mtc2    $11, @@rampDelta[10]        // rampReverb

    // Extract right
    srl     $12, cmd_w1, 0xC
    andi    @@right_addr, $12, 0xFF0

    // Extract wet left
    srl     $12, cmd_w1, 4
    andi    @@wet_left_addr, $12, 0xFF0

    // Extract wet right
    sll     $12, cmd_w1, 4
    andi    @@wet_right_addr, $12, 0xFF0

    // Extract stereoStrongRight (? should be left?)
    andi    $12, cmd_w0, 2
    srl     $12, $12, 1
    neg     $12, $12
    mtc2    $12, @@vStereoEffects[0]    // stereoStrongRight ? 0xFFFF : 0x0000

    // Extract stereoStrongLeft (? should be right?)
    andi    $12, cmd_w0, 1
    neg     $12, $12
    mtc2    $12, @@vStereoEffects[2]    // stereoStrongLeft ? 0xFFFF : 0x0000

    // Extract stereoHeadsetEffects
    andi    $12, cmd_w0, 8
    srl     $12, $12, 1
    neg     $12, $12
    mtc2    $12, @@vStereoEffects[4]    // stereoHeadsetEffects ? 0xFFFC : 0x0000

    // Extract usesHeadsetPanEffects
    andi    $12, cmd_w0, 4
    srl     $12, $12, 1
    neg     $12, $12
    mtc2    $12, @@vStereoEffects[6]    // usesHeadsetPanEffects ? 0xFFFE : 0x0000

    // Extract count
    srl     $12, cmd_w0, 8
    andi    @@count, $12, 0xFF

    vadd    $vzero, $vzero, $vzero  // Useless?

    // Extract swapLR
    andi    @@swapLR, cmd_w0, 0x10

    // Begin main loop, load saved envmixer state
    lqv     @@envState0[0], 0x00(@@env_state_addr)
@@loop:
    lqv     @@envState1[0], 0x10(@@env_state_addr)
    addi    @@env_state_addr, @@env_state_addr, ENVMIXER_STATE_SIZE

    // By assumption of having run ENVSETUP and ENVSETUP2:
    // gv_envSettings = [ vol_left, vol_left + ramp_left, vol_right, vol_right + ramp_right, vol_reverb, vol_reverb + ramp_reverb, 0, 0 ]

    // Multiply volume
    vmudm   @@leftAdd, @@envState0, gv_envSettings[0]       // vol left
    vmudm   @@rightAdd, @@envState0, gv_envSettings[2]      // vol right

    // decr count
    addi    @@count, @@count, -0x10

    // Load data
    lqv     @@leftData[0], 0x00(@@left_addr)
    lqv     @@rightData[0], 0x00(@@right_addr)

    // Multiply volume + ramp
    vmudm   @@leftRampAdd, @@envState1, gv_envSettings[1]   // vol left + ramp left
    vmudm   @@rightRampAdd, @@envState1, gv_envSettings[3]  // vol right + ramp right

    lqv     @@left_data_1[0], 0x10(@@left_addr)
    lqv     @@right_data_1[0], 0x10(@@right_addr)

    vxor    @@leftAdd, @@leftAdd, @@vStereoEffects[0]       // stereoStrongRight 0xFFFF or 0x0000, inverts volume?
    vxor    @@rightAdd, @@rightAdd, @@vStereoEffects[1]     // stereoStrongLeft 0xFFFF or 0x0000, inverts volume?

    lqv     @@wet_left_data[0], 0x00(@@wet_left_addr)
    lqv     @@wet_right_data[0], 0x00(@@wet_right_addr)

    vadd    @@leftData, @@leftData, @@leftAdd
    vadd    @@rightData, @@rightData, @@rightAdd

    vmudm   @@leftAdd, @@leftAdd, gv_envSettings[4]         // reverb vol
    vmudm   @@rightAdd, @@rightAdd, gv_envSettings[4]       // reverb vol

    vxor    @@leftRampAdd, @@leftRampAdd, @@vStereoEffects[0]       // leftRampAdd ^= stereoStrongRight
    vxor    @@rightRampAdd, @@rightRampAdd, @@vStereoEffects[1]     // stereoStrongLeft

    lqv     @@wet_left_result[0], 0x10(@@wet_left_addr)
    lqv     @@wet_right_result[0], 0x10(@@wet_right_addr)

    vadd    @@left_data_1, @@left_data_1, @@leftRampAdd
    vadd    @@right_data_1, @@right_data_1, @@rightRampAdd

    vmudm   @@leftRampAdd, @@leftRampAdd, gv_envSettings[5]     // reverb vol + ramp reverb
    vmudm   @@rightRampAdd, @@rightRampAdd, gv_envSettings[5]   // reverb vol + ramp reverb

    vxor    @@leftAdd, @@leftAdd, @@vStereoEffects[2]           // stereoHeadsetEffects
    vxor    @@rightAdd, @@rightAdd, @@vStereoEffects[3]         // usesHeadsetPanEffects

    sqv     @@leftData[0], 0x00(@@left_addr)                    // store left 0

    vxor    @@leftRampAdd, @@leftRampAdd, @@vStereoEffects[2]   // stereoHeadsetEffects
    vxor    @@rightRampAdd, @@rightRampAdd, @@vStereoEffects[3] // usesHeadsetPanEffects

    bnez    @@swapLR, @@swap
     sqv    @@rightData[0], 0x00(@@right_addr) // store right 0

    vadd    @@wet_left_data, @@wet_left_data, @@leftAdd
    vadd    @@wet_right_data, @@wet_right_data, @@rightAdd

    sqv     @@left_data_1[0], 0x10(@@left_addr) // store left
    sqv     @@right_data_1[0], 0x10(@@right_addr)

    vadd    @@wet_left_result, @@wet_left_result, @@leftRampAdd
    vadd    @@wet_right_result, @@wet_right_result, @@rightRampAdd

@@swap_ret:
    addi    @@left_addr, @@left_addr, ENVMIXER_STATE_SIZE

    sqv     @@wet_left_data[0], 0x00(@@wet_left_addr)
    sqv     @@wet_right_data[0], 0x00(@@wet_right_addr)

    addi    @@right_addr, @@right_addr, ENVMIXER_STATE_SIZE

    lqv     @@envState0[0], 0x00(@@env_state_addr)
    sqv     @@wet_left_result[0], 0x10(@@wet_left_addr)
    addi    @@wet_left_addr, @@wet_left_addr, ENVMIXER_STATE_SIZE
    vaddc   gv_envSettings, gv_envSettings, @@rampDelta     // rampDelta = [ rampLeft, rampLeft, rampRight, rampRight, rampReverb, rampReverb, 0, 0 ]
    sqv     @@wet_right_result[0], 0x10(@@wet_right_addr)
    addi    @@wet_right_addr, @@wet_right_addr, ENVMIXER_STATE_SIZE
    bgtz    @@count, @@loop
     vadd   $vzero, $vzero, $vzero
    j       next_cmd
     vclr   $vzero
@@swap:
    vadd    @@wet_left_data, @@wet_left_data, @@rightAdd
    vadd    @@wet_right_data, @@wet_right_data, @@leftAdd

    sqv     @@left_data_1[0], 0x10(@@left_addr)
    sqv     @@right_data_1[0], 0x10(@@right_addr)

    vadd    @@wet_left_result, @@wet_left_result, @@rightRampAdd
    j       @@swap_ret
     vadd   @@wet_right_result, @@wet_right_result, @@leftRampAdd

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

    vclr    gv_envSettings
    andi    @@ramp_reverb, cmd_w0, 0xFFFF
    srl     @@reverb_vol, cmd_w0, 8
    andi    @@reverb_vol, @@reverb_vol, 0xFF00
    mtc2    @@reverb_vol, gv_envSettings[8]     // vol reverb
    add     @@reverb_vol, @@reverb_vol, @@ramp_reverb
    mtc2    @@reverb_vol, gv_envSettings[10]    // vol reverb + ramp reverb
    srl     g_rampLeft, cmd_w1, 0x10
    j       next_cmd
     andi   g_rampRight, cmd_w1, 0xFFFF

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
    mtc2    @@vol_left, gv_envSettings[0]       // vol left
    add     @@vol_left, @@vol_left, g_rampLeft
    mtc2    @@vol_left, gv_envSettings[2]       // vol left + ramp left
    andi    @@vol_right, cmd_w1, 0xFFFF
    mtc2    @@vol_right, gv_envSettings[4]      // vol right
    add     @@vol_right, @@vol_right, g_rampRight
    j       next_cmd
     mtc2   @@vol_right, gv_envSettings[6]      // vol right + ramp right

setup_buff_noop:
    srl     $3, cmd_w0, 0xC     // Count
    andi    $3, $3, 0xFF0
    andi    $1, cmd_w0, 0xFFFF  // DMEM addr
setup_loadadpcm:
    sll     DMA_DRAM_ADDR, cmd_w1, 8
    jr      $ra
     srl    DMA_DRAM_ADDR, DMA_DRAM_ADDR, 8 // DRAM addr, remove highest byte

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Count           | DMEM Addr                       | Address                                                         |
 * | 8               | 8               | 16                              | 16                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 */
cmd_LOADBUFF:
    jal     setup_buff_noop
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
    jal     setup_buff_noop
     nop
    j       dma_write_and_nextcmd
     nop

/**
 * |                 |                 |                                 |                                                                 |
 * | Command         | Empty           | Count                           | Address                                                         |
 * | 8               | 8               | 16                              | 16                                                              |
 * | 31           24 | 23           16 | 15                            0 | 31                                                            0 |
 * | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
 *
 * Loads the ADPCM codebook.
 * Count is the size in bytes: 16 * order * npredictors
 * Address points to the book data
 */
cmd_LOADADPCM:
    jal     setup_loadadpcm
     li     DMA_MEM_ADDR, adpcmCodebook
    j       dma_read_and_nextcmd
     andi   DMA_LENGTH, cmd_w0, 0xFFFF

dma_read_and_nextcmd:
    jal     dma_read_start
     addi   DMA_LENGTH, DMA_LENGTH, -1
    j       next_cmd
     nop

dma_write_and_nextcmd:
    jal     dma_write_start
     addi   DMA_LENGTH, DMA_LENGTH, -1
    j       next_cmd
     nop

/**
 * DMA read
 */
dma_read_start:
    // attempt to acquire semaphore
    mfc0    $4, SP_SEMAPHORE
    // branch away if it was not acquired (reg contains 0 if acquired)
    bnez    $4, dma_read
     nop
    // we have the semaphore now, wait until dma not full
@@dma_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_full
     nop
    // start the transfer and jump to dma wait
    mtc0    DMA_MEM_ADDR, SP_MEM_ADDR
    mtc0    DMA_DRAM_ADDR, SP_DRAM_ADDR
    j       dma_wait
     mtc0   DMA_LENGTH, SP_RD_LEN

/**
 * DMA write
 */
dma_write_start:
    // attempt to acquire semaphore
    mfc0    $4, SP_SEMAPHORE
    // branch away if it was not acquired (reg contains 0 if acquired)
    bnez    $4, dma_write
     nop
    // we have the semaphore now, wait until dma not full
@@dma_full:
    mfc0    $4, SP_DMA_FULL
    bnez    $4, @@dma_full
     nop
    // start the transfer and fallthrough to dma wait
    mtc0    DMA_MEM_ADDR, SP_MEM_ADDR
    mtc0    DMA_DRAM_ADDR, SP_DRAM_ADDR
    mtc0    DMA_LENGTH, SP_WR_LEN
    // FALLTHROUGH

/**
 * DMA wait
 */
dma_wait:
    // wait until dma is no longer busy
    li      $4, 1
@@dma_busy:
    bnez    $4, @@dma_busy
     mfc0   $4, SP_DMA_BUSY
    // release semaphore and exit
    jr      $ra
     mtc0   $zero, SP_SEMAPHORE

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

    lqv     $v31[0], (useful_constants)($zero)
    srl     @@count, cmd_w0, 0xC
    andi    @@count, @@count, 0xFF0         // count << 4
    andi    @@dmem_out, cmd_w1, 0xFFFF      // load dmem out
    srl     @@dmem_in, cmd_w1, 0x10         // load dmem in
    andi    @@gain, cmd_w0, 0xFFFF
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

    lhu     @@input_buffer, (audio_in_buf)(audioStructPtr)
    vclr    $v2
    lhu     @@output_buffer, (audio_out_buf)(audioStructPtr)
    vclr    $v3
    lhu     @@count, (audio_count)(audioStructPtr)
    sll     @@dram_addr, cmd_w1, 8
    srl     @@dram_addr, @@dram_addr, 8
    sqv     $v2[0], 0x00(@@output_buffer)   // clear 0x20 bytes from output buffer
    sqv     $v3[0], 0x10(@@output_buffer)
    srl     @@flags, cmd_w0, 0x10
    andi    @@flags, @@flags, A_INIT
    bgtz    @@flags, @@no_dma
     srl    @@flags, cmd_w0, 0x10
    andi    @@flags, @@flags, A_LOOP
    beq     $zero, @@flags, @@no_loop
     move   DMA_DRAM_ADDR, @@dram_addr                          // DMA read from dram_addr
    lw      DMA_DRAM_ADDR, (audio_loop_addr)(audioStructPtr)    // DMA read from loop_addr
@@no_loop:
    move    DMA_MEM_ADDR, @@output_buffer   // read into output buffer? (DMEM)
    jal     dma_read_start
     li     DMA_LENGTH, 0x20 - 1
@@no_dma:
    addi    @@output_buffer, @@output_buffer, 0x20
    beqz    @@count, @@done     // Count is 0, done
     nop
    lpv     $v2[0], 0x0(@@input_buffer)     // load packed from input buffer [0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA] -> [0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00, 0xAA00]
    lpv     $v3[0], 0x8(@@input_buffer)
    addi    @@input_buffer, @@input_buffer, 0x10
@@loop:
    addi    @@count, @@count, -0x20
    lpv     @@input_data_lo[0], (@@input_buffer)
    sqv     $v2[0], (@@output_buffer)
    lpv     @@input_data_hi[0], 0x8(@@input_buffer)
    sqv     $v3[0], 0x10(@@output_buffer)
    bgtz    @@count, @@not_done
     addi   @@output_buffer, @@output_buffer, 0x20
@@done:
    addi    DMA_MEM_ADDR, @@output_buffer, -0x20  // Write last 0x20 of output buffer to dram_addr
    move    DMA_DRAM_ADDR, @@dram_addr
    j       dma_write_and_nextcmd
     li     DMA_LENGTH, 0x20
@@not_done:
    addi    @@count, @@count, -0x20
    lpv     $v2[0], 0x10(@@input_buffer)
    sqv     @@input_data_lo[0], 0x00(@@output_buffer)
    lpv     $v3[0], 0x18(@@input_buffer)
    sqv     @@input_data_hi[0], 0x10(@@output_buffer)
    addi    @@input_buffer, @@input_buffer, 0x20
    bgtz    @@count, @@loop
     addi   @@output_buffer, @@output_buffer, 0x20
    j       @@done
     nop

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

    andi    @@count, cmd_w0, 0xFFFF     // load count
    srl     @@dmem_addr, cmd_w1, 0x10   // extract dmem addr
    srl     @@gain, cmd_w0, 4
    andi    @@gain, @@gain, 0xF000
    mtc2    @@gain, $v3[2]              // move LO(gain) << 0xC to $v3[2]
    srl     @@gain, cmd_w0, 0x14
    andi    @@gain, @@gain, 0xF
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
 *
 *  Discrete convolution
 */
cmd_FILTER:
    @@flags     equ $12
    @@count     equ $15
    @@dmem_addr equ $11

    @@buffer1 equ $13
    @@buffer2 equ $14

    move    @@buffer1, tmpDataPtr
    vclr    $vzero
    addi    @@buffer2, tmpDataPtr, 0x20
    sqv     $vzero[0], 0x00(@@buffer1)      // reset buffer1
    sll     DMA_DRAM_ADDR, cmd_w1, 8
    sqv     $vzero[0], 0x10(@@buffer1)
    srl     DMA_DRAM_ADDR, DMA_DRAM_ADDR, 8
    srl     @@flags, cmd_w0, 0x10
    andi    @@flags, @@flags, 0xFF
    beqz    @@flags, @@dma_buffer           // branch if flags = A_CONTINUE
     nop
    addi    @@flags, @@flags, -1
    beqz    @@flags, @@no_dma               // branch if flags = A_INIT
     nop
    andi    @@count, cmd_w0, 0xFFFF
    vclr    $vzero
    sqv     $vzero[0], 0x00(@@buffer2)
    sqv     $vzero[0], 0x20(@@buffer2)
    addi    DMA_MEM_ADDR, @@buffer2, 0x10
    j       dma_read_and_nextcmd
     li     DMA_LENGTH, 0x10
@@dma_buffer:
    add     DMA_MEM_ADDR, @@buffer1, $zero            // read buffer1 from dram address
    jal     dma_read_start
@@no_dma:
     li     DMA_LENGTH, 0x20 - 1
    lqv     $v24[0], 0x10(@@buffer2)
    lqv     $v25[0], 0x10(@@buffer1)
    vclr    $v14
    vmulf   $vzero, $vzero, $vzero          // Clear accumulator?
    li      $12, 0x4000                     // 0.5(0x4000)
    mtc2    $12, $v15[0]
    vmacf   $v14, $v24, $v15[0]             // buffer2[0x10..0x20] * 0.5 + 0
    vmacf   $v14, $v25, $v15[0]             // buffer1[0x10..0x20] * 0.5 + buffer2[0x10..0x20] * 0.5 + 0
    sqv     $v14[0], 0x10(@@buffer2)        // store average of buffer1 and buffer2
    sqv     $v14[0], 0x10(@@buffer1)
    add     DMA_MEM_ADDR, @@buffer1, $zero  // set up dram address for later dma write to rdram
    andi    @@dmem_addr, cmd_w0, 0xFFFF     // set up dmem address
    // ldv is used for most of these as lqv cannot perform the same kind of unaligned memory access
    lqv     $v24[0], 0x10(@@buffer2)        // buffer2[0x10..0x20]
    ldv     $v28[0], 0x08(@@buffer2)        // buffer2[0x08..0x10]
    ldv     $v28[8], 0x10(@@buffer2)        // buffer2[0x10..0x18]
    ldv     $v20[0], 0x18(@@buffer2)        // buffer2[0x18..0x20]
    ldv     $v20[8], 0x20(@@buffer2)        // buffer2[0x20..0x28]
    addi    @@buffer2, @@buffer2, 2     // 2
    ldv     $v31[0], 0x00(@@buffer2)        // buffer2[0x02..0x0A]
    ldv     $v31[8], 0x08(@@buffer2)        // buffer2[0x0A..0x12]
    ldv     $v17[0], 0x10(@@buffer2)        // buffer2[0x12..0x1A]
    ldv     $v17[8], 0x18(@@buffer2)        // buffer2[0x1A..0x22]
    ldv     $v27[0], 0x08(@@buffer2)        // buffer2[0x0A..0x12]
    ldv     $v27[8], 0x10(@@buffer2)        // buffer2[0x12..0x1A]
    ldv     $v21[0], 0x18(@@buffer2)        // buffer2[0x1A..0x22]
    ldv     $v21[8], 0x20(@@buffer2)        // buffer2[0x22..0x2A]
    addi    @@buffer2, @@buffer2, 2     // 4
    ldv     $v30[0], 0x00(@@buffer2)        // buffer2[0x04..0x0C]
    ldv     $v30[8], 0x08(@@buffer2)        // buffer2[0x0C..0x14]
    ldv     $v26[0], 0x08(@@buffer2)        // buffer2[0x0C..0x14]
    ldv     $v26[8], 0x10(@@buffer2)        // buffer2[0x14..0x1C]
    ldv     $v18[0], 0x10(@@buffer2)        // buffer2[0x14..0x1C]
    ldv     $v18[8], 0x18(@@buffer2)        // buffer2[0x1C..0x24]
    ldv     $v22[0], 0x18(@@buffer2)        // buffer2[0x1C..0x24]
    ldv     $v22[8], 0x20(@@buffer2)        // buffer2[0x24..0x2C]
    addi    @@buffer2, @@buffer2, 2     // 6
    ldv     $v29[0], 0x00(@@buffer2)        // buffer2[0x06..0x0E]
    ldv     $v29[8], 0x08(@@buffer2)        // buffer2[0x0E..0x16]
    ldv     $v25[0], 0x08(@@buffer2)        // buffer2[0x0E..0x16]
    ldv     $v25[8], 0x10(@@buffer2)        // buffer2[0x16..0x1E]
    ldv     $v19[0], 0x10(@@buffer2)        // buffer2[0x16..0x1E]
    ldv     $v19[8], 0x18(@@buffer2)        // buffer2[0x1E..0x26]
    ldv     $v23[0], 0x18(@@buffer2)        // buffer2[0x1E..0x26]
    ldv     $v23[8], 0x20(@@buffer2)        // buffer2[0x26..0x2E]
    lqv     $v15[0], (@@buffer1)
@@loop:
    lqv     $v16[0], (@@dmem_addr)
    vclr    $v14
    vmulf   $vzero, $vzero, $vzero  // 0                                        What about buffer1[0] ?
    vmacf   $v14, $v23, $v15[1]     // += buffer2[0x1E..0x2E] * buffer1[1]      Indices on buffer1 and dmem_buf are u16 elements
    vmacf   $v14, $v22, $v15[2]     // += buffer2[0x1C..0x2C] * buffer1[2]
    vmacf   $v14, $v21, $v15[3]     // += buffer2[0x1A..0x2A] * buffer1[3]
    vmacf   $v14, $v20, $v15[4]     // += buffer2[0x18..0x28] * buffer1[4]
    vmacf   $v14, $v19, $v15[5]     // += buffer2[0x16..0x26] * buffer1[5]
    vmacf   $v14, $v18, $v15[6]     // += buffer2[0x14..0x24] * buffer1[6]
    vmacf   $v14, $v17, $v15[7]     // += buffer2[0x12..0x22] * buffer1[7]
    vmacf   $v14, $v24, $v16[0]     // += buffer2[0x10..0x20] * dmem_buf[0]
    vmacf   $v14, $v25, $v16[1]     // += buffer2[0x0E..0x1E] * dmem_buf[1]
    vmacf   $v14, $v26, $v16[2]     // += buffer2[0x0C..0x1C] * dmem_buf[2]
    vmacf   $v14, $v27, $v16[3]     // += buffer2[0x0A..0x1A] * dmem_buf[3]
    vmacf   $v14, $v28, $v16[4]     // += buffer2[0x08..0x18] * dmem_buf[4]
    vmacf   $v14, $v29, $v16[5]     // += buffer2[0x06..0x16] * dmem_buf[5]
    vmacf   $v14, $v30, $v16[6]     // += buffer2[0x04..0x14] * dmem_buf[6]
    vmacf   $v14, $v31, $v16[7]     // += buffer2[0x02..0x12] * dmem_buf[7]
    addi    @@count, @@count, -0x10
    sqv     $v14[0], (@@dmem_addr)
    addi    @@dmem_addr, @@dmem_addr, 0x10
    bgtz    @@count, @@loop
     vaddc  $v15, $vzero, $v16      // move dmem_buf to buffer1
    sqv     $v16[0], (@@buffer1)    // store last dmem_buf to buffer1
    j       dma_write_and_nextcmd
     li     DMA_LENGTH, 0x20 - 1

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
    srl     @@count, cmd_w0, 0xC
    andi    @@count, @@count, 0xFF0
    andi    @@dmem_out, cmd_w1, 0xFFFF
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
    andi    @@pitch, cmd_w0, 0xFFFF
    sll     @@pitch, @@pitch, 2
    andi    @@position, cmd_w1, 0xFFFF
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

    lqv     $v31[0], (useful_constants)($zero)
    andi    @@count, cmd_w0, 0xFFFF
    andi    @@dmem_out, cmd_w1, 0xFFFF
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
    jal     setup_buff_noop
     nop
    move    DMA_MEM_ADDR, tmpDataPtr
    jal     dma_read_start
     addi   DMA_LENGTH, DMA_LENGTH, -1
    lw      $6, 4(DMA_MEM_ADDR)         // increment offset 4
    addi    $6, $6, 1
    sw      $6, 4(DMA_MEM_ADDR)
    sw      taskDataSize, 8(DMA_MEM_ADDR)           // store task_size to offset 8
    sw      cmd_buf_remaining, 0xC(DMA_MEM_ADDR)    // store current alist buffer size to offset 0xc
    andi    $6, cmd_w0, 0xFFFF
    jal     dma_write_start     // write this struct out to dram
     sh     $6, (DMA_MEM_ADDR)  // store unknown value to offset 0
    j       next_cmd
     nop

.align 0x10

.if . > IMEM_END_VIRT
    .error "Not enough room in IMEM"
.endif

.close