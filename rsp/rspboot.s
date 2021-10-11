.rsp

.create CODE_FILE, 0x04001000

entry:
    j       lbl_04001064
     addi   $1, $zero, 0xFC0

lbl_04001008:
    lw      $2, 0x10($1)
    addi    $3, $zero, 0xF80-1
    addi    $7, $zero, 0x1080
    mtc0    $7, SP_MEM_ADDR
    mtc0    $2, SP_DRAM_ADDR
    mtc0    $3, SP_RD_LEN
lbl_04001020:
    mfc0    $4, SP_DMA_BUSY
    bnez    $4, lbl_04001020
     nop
    jal     lbl_0400103C
     nop
    jr      $7
     mtc0   $zero, SP_SEMAPHORE

lbl_0400103C:
    mfc0    $8, SP_STATUS
    andi    $8, $8, 0x80
    bnez    $8, lbl_04001050
     nop
    jr      $ra
lbl_04001050:
     mtc0   $zero, SP_SEMAPHORE
    li      $8, 0x5200
    mtc0    $8, SP_STATUS
    break
    nop

lbl_04001064:
    lw      $2, 4($1)
    andi    $2, $2, 0x2
    beqz    $2, lbl_0400108C
     nop
    jal     lbl_0400103C
     nop
    mfc0    $2, DPC_STATUS
    andi    $2, $2, 0x100
    bgtz    $2, lbl_0400103C
     nop
lbl_0400108C:
    lw      $2, 0x18($1)
    lw      $3, 0x1C($1)
lbl_04001094:
    addi    $3, $3, -1
lbl_04001098:
    mfc0    $30, SP_DMA_FULL
    bnez    $30, lbl_04001098
     nop
    mtc0    $zero, SP_MEM_ADDR
    mtc0    $2, SP_DRAM_ADDR
    mtc0    $3, SP_RD_LEN
lbl_040010b0:
    mfc0    $4, SP_DMA_BUSY
    bnez    $4, lbl_040010b0
     nop
    jal     lbl_0400103C
     nop
    j       lbl_04001008
     nop
    nop

.close
