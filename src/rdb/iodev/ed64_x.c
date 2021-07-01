#include "iodev.h"

#define REG_BASE 0xBF800000
#define REG_FPG_CFG 0x0000
#define REG_USB_CFG 0x0001
#define REG_TIMER 0x0003
#define REG_BOOT_CFG 0x0004
#define REG_EDID 0x0005
#define REG_I2C_CMD 0x0006
#define REG_I2C_DAT 0x0007
#define REG_FPG_DAT 0x0080
#define REG_USB_DAT 0x0100
#define REG_SYS_CFG 0x2000
#define REG_KEY 0x2001
#define REG_DMA_STA 0x2002
#define REG_DMA_ADDR 0x2002
#define REG_DMA_LEN 0x2003
#define REG_RTC_SET 0x2004
#define REG_GAM_CFG 0x2006
#define REG_IOM_CFG 0x2007
#define REG_SD_CMD_RD 0x2008
#define REG_SD_CMD_WR 0x2009
#define REG_SD_DAT_RD 0x200A
#define REG_SD_DAT_WR 0x200B
#define REG_SD_STATUS 0x200C
#define REG_SDIO_ARD 0x2080
#define REG_IOM_DAT 0x2100
#define REG_DD_TBL 0x2200
#define REGS_PTR ((volatile u32*)REG_BASE)

#define FPG_CFG_NCFG 0x0001
#define FPG_STA_CDON 0x0001
#define FPG_STA_NSTAT 0x0002

#define USB_LE_CFG 0x8000
#define USB_LE_CTR 0x4000

#define USB_CFG_ACT 0x0200
#define USB_CFG_RD 0x0400
#define USB_CFG_WR 0x0000

#define USB_STA_ACT 0x0200
#define USB_STA_RXF 0x0400
#define USB_STA_TXE 0x0800
#define USB_STA_PWR 0x1000
#define USB_STA_BSY 0x2000

#define I2C_CMD_DAT 0x10
#define I2C_CMD_STA 0x20
#define I2C_CMD_END 0x30

#define CFG_BROM_ON 0x0001
#define CFG_REGS_OFF 0x0002
#define CFG_SWAP_ON 0x0004

#define DMA_STA_BUSY 0x0001
#define DMA_STA_ERROR 0x0002
#define DMA_STA_LOCK 0x0080

#define IOM_CFG_SS 0x0001
#define IOM_CFG_RST 0x0002
#define IOM_CFG_ACT 0x0080
#define IOM_STA_CDN 0x0001

#define SD_CFG_BITLEN 0x000F
#define SD_CFG_SPD 0x0010
#define SD_STA_BUSY 0x0080

#define SPI_SPEED_50 0x0000
#define SPI_SPEED_25 0x0001
#define SPI_SPEED_LO 0x0002
#define SPI_SPEED 0x0003
#define SPI_SS 0x0004
#define SPI_RD 0x0008
#define SPI_WR 0x0000
#define SPI_DAT 0x0010
#define SPI_CMD 0x0000
#define SPI_BIT 0x0020
#define SPI_BYTE 0x0000

/* much of this code is based directly on 
 *  https://github.com/glankk/oot/blob/everdrive-v3-syncprintf/src/boot/ed64_v3.c and
 *  https://github.com/glankk/gz/blob/master/src/gz/ed64_x.c
 */

#define BLK_SIZE 512
#define CART_ADDR 0xB4000000

static u32 cart_irqf, cart_lat, cart_pwd;

#define REG_RD(reg) (REGS_PTR[REG_FPG_CFG], REGS_PTR[reg])
#define REG_WR(reg, dat) (REGS_PTR[REG_FPG_CFG], REGS_PTR[reg] = (dat))

static void cart_lock_safe(void) {
    __osPiGetAccess();
    cart_irqf = __osDisableInt();

    cart_lat = HW_REG(PI_BSD_DOM2_LAT_REG, u32);
    cart_pwd = HW_REG(PI_BSD_DOM2_PWD_REG, u32);

    REG_WR(REG_KEY, 0xAA55);
}

static void cart_lock(void) {
    __osPiGetAccess();
    cart_irqf = __osDisableInt();

    cart_lat = HW_REG(PI_BSD_DOM2_LAT_REG, u32);
    cart_pwd = HW_REG(PI_BSD_DOM2_PWD_REG, u32);

    HW_REG(PI_BSD_DOM2_LAT_REG, u32) = 4;
    HW_REG(PI_BSD_DOM2_PWD_REG, u32) = 12;

    REG_WR(REG_KEY, 0xAA55);
}

static void cart_unlock(void) {
    REG_WR(REG_KEY, 0x0000);

    HW_REG(PI_BSD_DOM2_LAT_REG, u32) = cart_lat;
    HW_REG(PI_BSD_DOM2_PWD_REG, u32) = cart_pwd;

    __osPiRelAccess();
    __osRestoreInt(cart_irqf);
}

static s32 ed64_x_fifo_poll_unsafe(void) {
    s32 ret = 0;

    if (PiMaybeGetAccess()) {
        REG_WR(REG_KEY, 0xAA55);

        ret = ((REG_RD(REG_USB_CFG) & (USB_STA_PWR | USB_STA_RXF)) == USB_STA_PWR);

        REG_WR(REG_KEY, 0x0000);
        __osPiRelAccess();
    }

    return ret;
}

static s32 ed64_x_fifo_poll(void) {
    s32 ret;

    cart_lock();

    ret = ((REG_RD(REG_USB_CFG) & (USB_STA_PWR | USB_STA_RXF)) == USB_STA_PWR);

    cart_unlock();

    return ret;
}

static s32 ed64_x_fifo_read(void* dst, size_t n_blocks) {
    char* p = dst;

    cart_lock();

    while (n_blocks != 0) {
        /* wait for power on and rx buffer full (PWR high, RXF low) */
        while ((REG_RD(REG_USB_CFG) & (USB_STA_PWR | USB_STA_RXF)) != USB_STA_PWR) {
            ;
        }

        /* receive */
        REG_WR(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_RD | USB_CFG_ACT);
        while (REG_RD(REG_USB_CFG) & USB_STA_ACT) {
            ;
        }

        /* copy from rx buffer */
        REG_WR(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_RD);
        {
            osInvalDCache(p, BLK_SIZE);
            HW_REG(PI_DRAM_ADDR_REG, u32) = (u32)p & 0x1FFFFFFF;
            HW_REG(PI_CART_ADDR_REG, u32) = ((u32)&REGS_PTR[REG_USB_DAT]) & 0x1FFFFFFF;
            HW_REG(PI_WR_LEN_REG, u32) = BLK_SIZE - 1;
            while (HW_REG(PI_STATUS_REG, u32) & PI_STATUS_BUSY) {
                ;
            }
            HW_REG(PI_STATUS_REG, u32) = PI_STATUS_CLEAR_INTR;
        }

        p += BLK_SIZE;
        n_blocks--;
    }

    cart_unlock();

    return 0;
}

static s32 ed64_x_fifo_write(void* src, size_t n_blocks) {
    char* p = src;

    cart_lock();

    while (n_blocks != 0) {
        /* wait for power on and tx buffer empty (PWR high, TXE low) */
        while ((REG_RD(REG_USB_CFG) & (USB_STA_PWR | USB_STA_TXE)) != USB_STA_PWR) {
            ;
        }

        //* copy to tx buffer */
        REG_WR(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_WR);
        {
            osWritebackDCache(p, BLK_SIZE);
            HW_REG(PI_DRAM_ADDR_REG, u32) = (u32)p & 0x1FFFFFFF;
            HW_REG(PI_CART_ADDR_REG, u32) = ((u32)&REGS_PTR[REG_USB_DAT]) & 0x1FFFFFFF;
            HW_REG(PI_RD_LEN_REG, u32) = BLK_SIZE - 1;
            while (HW_REG(PI_STATUS_REG, u32) & PI_STATUS_BUSY) {
                ;
            }
            HW_REG(PI_STATUS_REG, u32) = PI_STATUS_CLEAR_INTR;
        }

        /* transmit */
        REG_WR(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_WR | USB_CFG_ACT);
        while (REG_RD(REG_USB_CFG) & USB_STA_ACT) {
            ;
        }

        p += BLK_SIZE;
        n_blocks--;
    }

    cart_unlock();

    return 0;
}

static s32 ed64_x_probe(void) {
    s32 ret;

    cart_lock_safe();

    /* check magic number */
    ret = ((REG_RD(REG_EDID) >> 16) == 0xED64);

    cart_unlock();

    return ret;
}

IODev gDevED64_x = {
    ed64_x_probe, ed64_x_fifo_poll, ed64_x_fifo_poll_unsafe, ed64_x_fifo_read, ed64_x_fifo_write
};
