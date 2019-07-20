/*
* @file    MCSPI_reg.h
* @author  Aniruddha Kanhere
* @date    13 July 2019
* @version 1
* @brief   Prototypes of functions for interaction with MCSPI hardware.
*          Set/reset, wait and other config functions. Definitions of registers
*          and their base addresses
*/
#ifndef __MCSPI_REG_H__
#define __MCSPI_REG_H__

#ifndef USER_SPACE
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>

#include "my_gpio.h"         //for BIT() macro
#include "MCSPI_misc.h"
#include "cm_per.h"          //for enabling the clock to SPI0 module


#define DRIVER_NAME "MCSPI_Driver"

#include <linux/platform_data/spi-omap2-mcspi.h>
#endif

//  -- MCSPI0 address space --
#define MCSPI0_START         0x48030000
#define MCSPI0_END           0x48030FFF
#define MCSPI0_BASE          MCSPI0_START
#define MCSPI0_ADDR_SIZE     MCSPI0_END - MCSPI0_START

//  -- MCSPI1 address space --
#define MCSPI1_START         0x481A0000
#define MCSPI1_END           0x481A0FFF
#define MCSPI1_BASE          MCSPI1_START
#define MCSPI1_ADDR_SIZE     MCSPI1_END - MCSPI1_START

// -- Register offsets --
#define MCSPI_REVISION       0x000 //McSPI revision register
#define MCSPI_SYSCONFIG      0x110 //McSPI system configuration register
#define MCSPI_SYSSTATUS      0x114 //McSPI system status register
#define MCSPI_IRQSTATUS      0x118 //McSPI interrupt status register
#define MCSPI_IRQENABLE      0x11C //McSPI interrupt enable register
#define MCSPI_SYST           0x124 //McSPI system register
#define MCSPI_MODULCTRL      0x128 //McSPI module control register
#define MCSPI_CH0CONF        0x12C //McSPI channel 0 configuration register
#define MCSPI_CH0STAT        0x130 //McSPI channel 0 status register
#define MCSPI_CH0CTRL        0x134 //McSPI channel 0 control register
#define MCSPI_TX0            0x138 //McSPI channel 0 FIFO transmit buffer register
#define MCSPI_RX0            0x13C //McSPI channel 0 FIFO receive buffer register
#define MCSPI_CH1CONF        0x140 //McSPI channel 1 configuration register
#define MCSPI_CH1STAT        0x144 //McSPI channel 1 status register
#define MCSPI_CH1CTRL        0x148 //McSPI channel 1 control register
#define MCSPI_TX1            0x14C //McSPI channel 1 FIFO transmit buffer register
#define MCSPI_RX1            0x150 //McSPI channel 1 FIFO receive buffer register
#define MCSPI_CH2CONF        0x154 //McSPI channel 2 configuration register
#define MCSPI_CH2STAT        0x158 //McSPI channel 2 status register
#define MCSPI_CH2CTRL        0x15C //McSPI channel 2 control register
#define MCSPI_TX2            0x160 //McSPI channel 2 FIFO transmit buffer register
#define MCSPI_RX2            0x164 //McSPI channel 2 FIFO receive buffer register
#define MCSPI_CH3CONF        0x168 //McSPI channel 3 configuration register
#define MCSPI_CH3STAT        0x16C //McSPI channel 3 status register
#define MCSPI_CH3CTRL        0x170 //McSPI channel 3 control register
#define MCSPI_TX3            0x174 //McSPI channel 3 FIFO transmit buffer register
#define MCSPI_RX3            0x178 //McSPI channel 3 FIFO receive buffer register
#define MCSPI_XFERLEVEL      0x17C //McSPI transfer levels register
#define MCSPI_DAFTX          0x180 //McSPI DMA address aligned FIFO tx register
#define MCSPI_DAFRX          0x1A0 //McSPI DMA address aligned FIFO rx register


//--------------------  SYSCONFIG -------------------------
#define MCSPI_SYSCONFIG_CLOCKACTIVITY(val)		(0x03 << 8)
#define MCSPI_SYSCONFIG_SIDLEMODE(val)		    (0x03 << 3)
#define MCSPI_SYSCONFIG_SOFTRESET(val)		    BIT(1)
#define MCSPI_SYSCONFIG_AUTOIDLE(val)		      BIT(0)


//-------------------- SYSSTATUS -------------------------
#define MCSPI_SYSSTATUS_RESETDONE_MASK		0x01UL


//-------------------- MODULCTRL -------------------------
#define MCSPI_MODULCTRL_PIN34(val)        (val << 1)
#define MCSPI_CS_SENSITIVE_DISABLED       0x00UL
#define MCSPI_CS_SENSITIVE_ENABLED        0x01UL

#define MCSPI_MODULCTRL_MS(val)           (val << 2)
#define MCSPI_MODULCTRL_MASTER            0x00UL
#define MCSPI_MODULCTRL_SLAVE             0x01UL

#define MCSPI_MODULCTRL_INITDLY(val)      (val << 4)
#define MCSPI_MODULCTRL_INITDLY_NO_DLY    0x00UL
#define MCSPI_MODULCTRL_INITDLY_4_CLK     0x01UL
#define MCSPI_MODULCTRL_INITDLY_8_CLK     0x02UL
#define MCSPI_MODULCTRL_INITDLY_16_CLK    0x03UL
#define MCSPI_MODULCTRL_INITDLY_32_CLK    0x04UL

#define MCSPI_MODULCTRL_MOA(val)			    (val << 7)
#define MCSPI_MODULCTRL_FDAA(val)         (val << 8)


//---------------------- CHCTRL --------------------------
#define MCSPI_CHCTRL_EN(val)              (val << 0)


//---------------------- CHCONF --------------------------
#define MCSPI_CHCONF_PHA(val)             (val << 0)
#define MCSPI_CHCONF_PHA_ODD              0x00UL
#define MCSPI_CHCONF_PHA_EVEN             0x01UL

#define MCSPI_CHCONF_POL(val)             (val << 1)
#define MCSPI_CHCONF_POL_ACTIVE_HIGH      0x00UL
#define MCSPI_CHCONF_POL_ACTIVE_LOW       0x01UL

#define MCSPI_CHCONF_CLKD(val)            (val << 2)
#define CLK_1                             0x00UL
#define CLK_2                             0x01UL
#define CLK_4                             0x02UL
#define CLK_8                             0x03UL
#define CLK_16                            0x04UL
#define CLK_32                            0x05UL
#define CLK_64                            0x06UL
#define CLK_128                           0x07UL
#define CLK_256                           0x08UL
#define CLK_512                           0x09UL
#define CLK_1024                          0x0AUL
#define CLK_2048                          0x0BUL
#define CLK_4096                          0x0CUL
#define CLK_8192                          0x0DUL
#define CLK_16384                         0x0EUL
#define CLK_32768                         0x0FUL

#define MCSPI_CHCONF_EPOL(val)            (val << 6)
#define MCSPI_CHCONF_WL(val)				      (val << 7)
#define MCSPI_CHCONF_WL_8BIT              0x07UL
#define MCSPI_CHCONF_WL_16BIT             0x0FUL
#define MCSPI_CHCONF_WL_32BIT             0x1FUL

#define MCSPI_CHCONF_TRM(val)             (val << 12)
#define MCSPI_CHCONF_TRM_TX               0x02UL
#define MCSPI_CHCONF_TRM_RX               0x01UL
#define MCSPI_CHCONF_TRM_TX_RX            0x00UL

#define MCSPI_CHCONF_DPE0(val)			      (val << 16)
#define MCSPI_CHCONF_DPE1(val)			      (val << 17)
#define MCSPI_CHCONF_IS(val)              (val << 18)
#define MCSPI_CHCONF_FFEW(val)            (val << 27)
#define MCSPI_CHCONF_FFER(val)            (val << 28)

#define MCSPI_D0_IN_D1_OUT                0x00UL
#define MCSPI_D1_IN_D0_OUT                0x01UL

#define MCSPI_CS_ACTIVE_LOW               0x00UL
#define MCSPI_CS_ACTIVE_HIGH              0x01UL

//---------------------- CHSTAT ----------------------
#define MCSPI_CHSTAT_RXS_MASK             BIT(0)
#define MCSPI_CHSTAT_TXS_MASK             BIT(1)
#define MCSPI_CHSTAT_EOT_MASK	            BIT(2)
#define MCSPI_CHSTAT_TXFFE_MASK           BIT(3)
#define MCSPI_CHSTAT_TXFFF_MASK           BIT(4)
#define MCSPI_CHSTAT_RXFFE_MASK           BIT(5)
#define MCSPI_CHSTAT_RXFFF_MASK           BIT(6)


//---------------------- IRQSTATUS ----------------------
#define MCSPI_IRQ_TX0_EMPTY_MASK          BIT(0)
#define MCSPI_IRQ_TX0_UNDERFLOW_MASK      BIT(1)
#define MCSPI_IRQ_RX0_FULL_MASK           BIT(2)
#define MCSPI_IRQ_RX0_OVERFLOW_MASK       BIT(3)

#define MCSPI_IRQ_EOW                     BIT(17)

#define MCSPI_IRQ_RESET				0xFFFFFFFF


//--------------------- XFERLEVEL ----------------------
#define MCSPI_XFER_AFL                    (0x07UL << 8)
#define MCSPI_XFER_AEL                    (0x07UL)
#define MCSPI_XFER_WCNT                   (0xFFFF << 16)

#ifndef USER_SPACE
struct MCSPI{
  void __iomem *base_addr;
  int  channel_number;                //can be 0,1,2 or 3
  unsigned int role;                 //can be MCSPI_MODULCTRL_MASTER/SLAVE
  unsigned int word_length;          //can be MCSPI_CHCONF_WL_(8/16/32)BIT
  unsigned int tx_rx;                //MCSPI_CHCONF_TRM_(TX/RX/TX_RX)
  unsigned int pin_direction;        //MCSPI_D0_IN_D1_OUT/MCSPI_D1_IN_D0_OUT
  unsigned int CS_polarity;          //MCSPI_CS_ACTIVE_(LOW/HIGH)
  unsigned int CS_sensitive;         //can be 1/0
  unsigned int polarity;             //MCSPI_CHCONF_POL_ACTIVE_LOW/HIGH
  unsigned int phase;                //MCSPI_CHCONF_PHA_ODD/EVEN
  unsigned int clock_div;            //Clock divider - CLK_1, 2,..., 16384, 32768
};


/*..............................................................................
    @breif:      Read/write to the given register
    @parameters: base_addr: the base address of the register bank
                 reg:       the offset of the register from the base_addr
                 val:       the value to be written to the register
    @return:     void
..............................................................................*/
u32 MCSPI_read_reg(void __iomem *base_addr, u32 reg);
void MCSPI_write_reg(void __iomem *base_addr,	u32 reg, u32 val);

/*..............................................................................
    @breif:      Sets the SPI mode MASTER or SLAVE
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_mode_set(struct MCSPI *dev);


/*..............................................................................
    @breif:      Sets the SPI word length for transfer
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_wl_set(struct MCSPI *dev);


/*..............................................................................
    @breif:      Sets the SPI polarity
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_pol_pha_set(struct MCSPI *dev);


/*..............................................................................
    @breif:      Enables or disables the SPI module
    @parameters: dev: the device struct for the SPI module
                 enable: can be 0/1 for disable/enable
    @return:     void
..............................................................................*/
void MCSPI_enable(struct MCSPI *dev, u8 enable);


/*..............................................................................
    @breif:      Chip select polarity and pin_selection
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_Set_CS(struct MCSPI *dev);


/*..............................................................................
    @breif:      Set Clock divider
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_Set_CLKD(struct MCSPI *dev);


/*..............................................................................
    @breif:      Set Clock divider
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_reset(struct MCSPI *dev);


/*..............................................................................
    @breif:      enable/disable SPI0 clock
    @parameters: base_addr: The base address of CM_PER registers
                 offset:    the offset of the CM_PER_SPI(0/1)_CLKCTRL register
    @return:     void
..............................................................................*/
void enable_clock(void __iomem *base_addr, u32 offset);
void disable_clock(void __iomem *clock_base, u32 offset);


/*..............................................................................
    @breif:      waits for the given bit to set/reset befor the given timeout,
                 else returns -1
    @parameters: addr: The register address to look for
                 bit:  the bit for which we are supposed to wait
                 timeout: timeout in milliseconds
    @return:     0 on success; -1 on error
..............................................................................*/
int MCSPI_wait_for_bit_set(void __iomem *addr, u32 bit, unsigned int timeout);
int MCSPI_wait_for_bit_reset(void __iomem *addr, u32 bit, unsigned int timeout);


void MCSPI_set_bit(void __iomem *addr, u32 bit);
void MCSPI_reset_bit(void __iomem *addr, u32 bit);
/*..............................................................................
    @breif:      IRQ handler for MCSPI controller
    @parameters: irq: the irq number
                 dev_id: pointer to temp_struct
    @return:     whether irq was handled or not
..............................................................................*/
irq_handler_t MCSPI_irq_handler(unsigned int irq, void *dev_id);
#endif //USER_SPACE

#endif //__MCSPI_REG_H__
