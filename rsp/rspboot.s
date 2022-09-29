.rsp
#include "rsp.inc"
#include "rsp.h"
#include "rdp.h"
#include "sptask.h"

.create CODE_FILE, 0x04001000

entry:
    j       start
     addi   $1, $zero, OSTask_addr

load_ucode_text_and_enter:
    lw      $2, OS_TASK_OFF_UCODE($1)
    addi    $3, $zero, (IMEM_SIZE - (RSPBOOT_ENTRYPOINT - IMEM_START))-1
    addi    $7, $zero, RSPBOOT_ENTRYPOINT
    mtc0    $7, SP_MEM_ADDR
    mtc0    $2, SP_DRAM_ADDR
    mtc0    $3, SP_RD_LEN
@@while_dma_busy:
    mfc0    $4, SP_DMA_BUSY
    bnez    $4, @@while_dma_busy
     nop
    jal     check_yield
     nop
    jr      $7
     mtc0   $zero, SP_SEMAPHORE

check_yield:
    mfc0    $8, SP_STATUS
    andi    $8, $8, SP_STATUS_YIELD
    bnez    $8, yield_break
     nop
    jr      $ra
yield_break:
     mtc0   $zero, SP_SEMAPHORE
    li      $8, (SP_SET_SIG2 | SP_SET_SIG1 | SP_CLR_SIG0)
    mtc0    $8, SP_STATUS
    break
    nop

start:
    lw      $2, OS_TASK_OFF_FLAGS($1)
    andi    $2, $2, OS_TASK_DP_WAIT
    beqz    $2, load_ucode_data
     nop
    jal     check_yield
     nop
    mfc0    $2, DPC_STATUS
    andi    $2, $2, DPC_STATUS_DMA_BUSY
    bgtz    $2, check_yield
     nop
load_ucode_data:
    lw      $2, OS_TASK_OFF_UDATA($1)
    lw      $3, OS_TASK_OFF_UDATA_SZ($1)
    addi    $3, $3, -1
@@while_dma_full:
    mfc0    $30, SP_DMA_FULL
    bnez    $30, @@while_dma_full
     nop
    mtc0    $zero, SP_MEM_ADDR
    mtc0    $2, SP_DRAM_ADDR
    mtc0    $3, SP_RD_LEN
@@while_dma_busy:
    mfc0    $4, SP_DMA_BUSY
    bnez    $4, @@while_dma_busy
     nop
    jal     check_yield
     nop
    j       load_ucode_text_and_enter
     nop

.align 8

.close
