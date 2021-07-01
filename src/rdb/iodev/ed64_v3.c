#include "iodev.h"

#define REG_BASE 0xA8040000
#define REG_CFG 0
#define REG_STATUS 1
#define REG_DMA_LEN 2
#define REG_DMA_ADDR 3
#define REG_MSG 4
#define REG_DMA_CFG 5
#define REG_SPI 6
#define REG_SPI_CFG 7
#define REG_KEY 8
#define REG_SAV_CFG 9
#define REG_SEC 10
#define REG_VER 11
#define REG_CFG_CNT 16
#define REG_CFG_DAT 17
#define REG_MAX_MSG 18
#define REG_CRC 19
#define REGS_PTR ((vu32*)REG_BASE)

#define CFG_SDRAM_ON 0x0001
#define CFG_SWAP 0x0002
#define CFG_WR_MOD 0x0004
#define CFG_WR_ADDR_MASK 0x0008

#define STATUS_DMA_BUSY 0x0001
#define STATUS_DMA_TOUT 0x0002
#define STATUS_TXE 0x0004
#define STATUS_RXF 0x0008
#define STATUS_SPI 0x0010

#define DMA_SD_TO_RAM 0x0001
#define DMA_RAM_TO_SD 0x0002
#define DMA_FIFO_TO_RAM 0x0003
#define DMA_RAM_TO_FIFO 0x0004

#define SPI_SPEED_50 0x0000
#define SPI_SPEED_25 0x0001
#define SPI_SPEED_LO 0x0002
#define SPI_SPEED 0x0003
#define SPI_SS 0x0004
#define SPI_RD 0x0008
#define SPI_WR 0x0000
#define SPI_DAT 0x0010
#define SPI_CMD 0x0000
#define SPI_1CLK 0x0020
#define SPI_BYTE 0x0000

/* much of this code is based directly on 
 *  https://github.com/glankk/oot/blob/everdrive-v3-syncprintf/src/boot/ed64_v3.c and 
 *  https://github.com/glankk/gz/blob/master/src/gz/ed64_v2.c
 */

#define BLK_SIZE 512
#define CART_ADDR 0xB3FFF800

static u32 cart_irqf, cart_lat, cart_pwd;

#define REG_RD(reg) (REGS_PTR[REG_CFG], REGS_PTR[reg])
#define REG_WR(reg, dat) (REGS_PTR[REG_CFG], REGS_PTR[reg] = (dat))

static void cart_lock_safe(void) {
    __osPiGetAccess();
    cart_irqf = __osDisableInt();

    cart_lat = HW_REG(PI_BSD_DOM2_LAT_REG, u32);
    cart_pwd = HW_REG(PI_BSD_DOM2_PWD_REG, u32);

    REG_WR(REG_KEY, 0x1234);
}

static void cart_lock(void) {
    __osPiGetAccess();
    cart_irqf = __osDisableInt();

    cart_lat = HW_REG(PI_BSD_DOM2_LAT_REG, u32);
    cart_pwd = HW_REG(PI_BSD_DOM2_PWD_REG, u32);

    HW_REG(PI_BSD_DOM2_LAT_REG, u32) = 4;
    HW_REG(PI_BSD_DOM2_PWD_REG, u32) = 12;

    REG_WR(REG_KEY, 0x1234);
}

static void cart_unlock(void) {
    REG_WR(REG_KEY, 0x0000);

    HW_REG(PI_BSD_DOM2_LAT_REG, u32) = cart_lat;
    HW_REG(PI_BSD_DOM2_PWD_REG, u32) = cart_pwd;

    __osPiRelAccess();
    __osRestoreInt(cart_irqf);
}

static s32 ed64_v3_fifo_poll_unsafe(void) {
    s32 ret = 0;

    if (PiMaybeGetAccess()) {
        REG_WR(REG_KEY, 0x1234);

        ret = !(REG_RD(REG_STATUS) & STATUS_RXF);

        REG_WR(REG_KEY, 0x0000);
        __osPiRelAccess();
    }

    return ret;
}

static s32 ed64_v3_fifo_poll(void) {
    s32 ret;

    cart_lock();

    ret = !(REG_RD(REG_STATUS) & STATUS_RXF);

    cart_unlock();

    return ret;
}

static s32 ed64_v3_fifo_read(void* dst, size_t n_blocks) {
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

static s32 ed64_v3_fifo_write(void* src, size_t n_blocks) {
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

static s32 ed64_v3_probe(void) {
    s32 ret = false;
    s32 i;

    cart_lock_safe();

    /* check firmware version */
    if ((u16)REG_RD(REG_VER) >= 0x0116) {
        /* check spi device */
        /* for a v2 device we expect a write with this config to trigger one
         * clock with DAT0-DAT3 high */
        REG_WR(REG_SPI_CFG, SPI_SPEED_LO | SPI_SS | SPI_RD | SPI_DAT | SPI_1CLK);
        REG_WR(REG_SPI, 0x00);
        for (i = 0; i <= 32; i++) {
            if (!(REG_RD(REG_STATUS) & STATUS_SPI) && (u16)REG_RD(REG_SPI) == 0x0F) {
                /* spi seems to work as expected */
                ret = true;
                break;
            }
        }
    }

    cart_unlock();

    return ret;
}

IODev gDevED64_v3 = {
    ed64_v3_probe, ed64_v3_fifo_poll, ed64_v3_fifo_poll_unsafe, ed64_v3_fifo_read, ed64_v3_fifo_write
};
