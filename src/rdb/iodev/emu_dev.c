#include "iodev.h"

#define REG_BASE 0xBFF00000

#define REG_CFG 0      // R/W | Config register, see below
#define REG_STATUS 1   // R   | Status register, see below
#define REG_DMA_LEN 2  // W   | FIFO DMA length in (blocks of 512) - 1
#define REG_DMA_ADDR 3 // W   | FIFO DMA cart addr in blocks of 0x800
#define REG_DMA_CFG 4  // W   | FIFO DMA config, determines direction and begins DMA on write
#define REG_DEVID 5    // R   | For identification, returns the 32-bit 'EMUD'
#define REG_INTR_R 6   // W   | Write a 0 to clear IP6 interrupt
#define REG_INTR_W 7   // W   | Write a 0 to clear IP7 interrupt
#define REGS_PTR ((vu32*)REG_BASE)

#define CFG_INTR_R_ENABLED (1 << 0) // 1=IP6 interrupt on recv data enabled, disabled by default
#define CFG_INTR_W_ENABLED (1 << 1) // 1=IP7 interrupt on send data enabled, disabled by default

#define STATUS_DMA_BUSY (1 << 0) // 1=DMA currently taking place
#define STATUS_DMA_TOUT (1 << 1) // 1=Last DMA timed out
#define STATUS_TXE (1 << 2)      // 0=Data can be sent to host
#define STATUS_RXF (1 << 3)      // 0=Data exists on fifo

#define DMA_FIFO_TO_RAM (1 << 0) // FIFO -> Cart
#define DMA_RAM_TO_FIFO (1 << 1) // Cart -> FIFO

#define BLK_SIZE 512
#define CART_ADDR 0xB4000000

/* Fictitious device based on the operation of the Everdrive 64 v3, intended for use on project64 with a js script.
 *  (currently?) without disk features, however it can signal IP6 and IP7 interrupts if configured to do so
 */

static u32 cart_irqf, cart_lat, cart_pwd;

#define REG_RD(reg) (REGS_PTR[REG_CFG], REGS_PTR[reg])
#define REG_WR(reg, dat) (REGS_PTR[REG_CFG], REGS_PTR[reg] = (dat))

static void cart_lock(void) {
    __osPiGetAccess();
    cart_irqf = __osDisableInt();

    cart_lat = HW_REG(PI_BSD_DOM2_LAT_REG, u32);
    cart_pwd = HW_REG(PI_BSD_DOM2_PWD_REG, u32);
}

static void cart_unlock(void) {
    HW_REG(PI_BSD_DOM2_LAT_REG, u32) = cart_lat;
    HW_REG(PI_BSD_DOM2_PWD_REG, u32) = cart_pwd;

    __osPiRelAccess();
    __osRestoreInt(cart_irqf);
}

static s32 emu_dev_fifo_poll_unsafe(void) {
    s32 ret = 0;

    if (PiMaybeGetAccess()) {
        ret = !(REG_RD(REG_STATUS) & STATUS_RXF);

        __osPiRelAccess();
    }

    return ret;
}

static s32 emu_dev_fifo_poll(void) {
    s32 ret;

    cart_lock();

    ret = !(REG_RD(REG_STATUS) & STATUS_RXF);

    cart_unlock();

    return ret;
}

static s32 emu_dev_fifo_read(void* dst, size_t n_blocks) {
    cart_lock();

    /* wait for rx buffer full (RXF low) */
    while (REG_RD(REG_STATUS) & STATUS_RXF) {
        ;
    }

    /* dma fifo to cart */
    REG_WR(REG_DMA_LEN, n_blocks - 1);
    REG_WR(REG_DMA_ADDR, CART_ADDR >> 11);
    REG_WR(REG_DMA_CFG, DMA_FIFO_TO_RAM);
    while (REG_RD(REG_STATUS) & STATUS_DMA_BUSY) {
        ;
    }

    /* check for dma timeout */
    if (REG_RD(REG_STATUS) & STATUS_DMA_TOUT) {
        cart_unlock();
        return -1;
    }

    {
        /* copy to ram */
        u32 n_bytes = BLK_SIZE * n_blocks;

        osInvalDCache((void*)dst, n_bytes);
        HW_REG(PI_DRAM_ADDR_REG, u32) = (u32)dst & 0x1FFFFFFF;
        HW_REG(PI_CART_ADDR_REG, u32) = CART_ADDR & 0x1FFFFFFF;
        HW_REG(PI_WR_LEN_REG, u32) = n_bytes - 1;
        while (HW_REG(PI_STATUS_REG, u32) & PI_STATUS_BUSY) {
            ;
        }
        HW_REG(PI_STATUS_REG, u32) = PI_STATUS_CLEAR_INTR;
    }

    cart_unlock();

    return 0;
}

static s32 emu_dev_fifo_write(void* src, size_t n_blocks) {
    s32 ret = 0;

    cart_lock();

    /* wait for tx buffer empty (TXE low) */
    while (REG_RD(REG_STATUS) & STATUS_TXE) {
        ;
    }

    {
        /* copy to cart */
        u32 n_bytes = BLK_SIZE * n_blocks;

        osWritebackDCache((void*)src, n_bytes);
        HW_REG(PI_DRAM_ADDR_REG, u32) = (u32)src & 0x1FFFFFFF;
        HW_REG(PI_CART_ADDR_REG, u32) = CART_ADDR & 0x1FFFFFFF;
        HW_REG(PI_RD_LEN_REG, u32) = n_bytes - 1;
        while (HW_REG(PI_STATUS_REG, u32) & PI_STATUS_BUSY) {
            ;
        }
        HW_REG(PI_STATUS_REG, u32) = PI_STATUS_CLEAR_INTR;
    }

    /* dma cart to fifo */
    REG_WR(REG_DMA_LEN, n_blocks - 1);
    REG_WR(REG_DMA_ADDR, CART_ADDR >> 11);
    REG_WR(REG_DMA_CFG, DMA_RAM_TO_FIFO);
    while (REG_RD(REG_STATUS) & STATUS_DMA_BUSY) {
        ;
    }

    /* check for dma timeout */
    if (REG_RD(REG_STATUS) & STATUS_DMA_TOUT) {
        ret = -1;
    }

    cart_unlock();

    return ret;
}

static s32 emu_dev_probe(void) {
    s32 ret;

    cart_lock();

    /* check magic number */
    ret = (REG_RD(REG_DEVID) == 'EMUD'); /* 0x454D5544 */

    cart_unlock();

    return ret;
}

IODev gDevEmu = {
    emu_dev_probe, emu_dev_fifo_poll, emu_dev_fifo_poll_unsafe, emu_dev_fifo_read, emu_dev_fifo_write 
};
