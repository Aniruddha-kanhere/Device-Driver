/*
* @file    MCSPI_reg.c
* @author  Aniruddha Kanhere
* @date    13 July 2019
* @version 1
* @brief   Definitions of functions for interaction with MCSPI hardware.
*          Set/reset, wait and other config functions
*/

#include "MCSPI_reg.h"

MODULE_LICENSE      ("GPL v2");                           ///< The license type -- this affects available functionality
MODULE_AUTHOR       ("Aniruddha Kanhere");             ///< The author -- visible when you use modinfo
MODULE_VERSION      ("1.0");                           ///< A version number to inform users


/*..............................................................................
    @breif:      Read from the given register
    @parameters: base_addr: the base address of the register bank
                 reg:       the offset of the register from the base_addr
    @return:     void
..............................................................................*/
inline u32 MCSPI_read_reg(void __iomem *base_addr, u32 reg)
{
  return ioread32(base_addr + reg);
}


/*..............................................................................
    @breif:      Write to the given register
    @parameters: base_addr: the base address of the register bank
                 reg:       the offset of the register from the base_addr
                 val:       the value to be written to the register
    @return:     void
..............................................................................*/
inline void MCSPI_write_reg(void __iomem *base_addr,	u32 reg, u32 val)
{
	iowrite32(val, base_addr + reg);
}


/*
core function for setting the TRM value of SPI module. Directly calling this is
not advised. Call the MCSPI_mode_set() function instead
*/
static void __set_tx_rx(struct MCSPI *dev, u32 channel_conf)
{
  u32 val;
  val = MCSPI_read_reg(dev->base_addr, channel_conf);
  val &= ~MCSPI_CHCONF_TRM(0x03);
  switch(dev->tx_rx)
  {
    case MCSPI_CHCONF_TRM_RX:
         val |= MCSPI_CHCONF_TRM(MCSPI_CHCONF_TRM_RX);
         break;
    case MCSPI_CHCONF_TRM_TX:
         val |= MCSPI_CHCONF_TRM(MCSPI_CHCONF_TRM_TX);
         break;
    case MCSPI_CHCONF_TRM_TX_RX:
    default:
         val |= MCSPI_CHCONF_TRM(MCSPI_CHCONF_TRM_TX_RX);
         break;
  }
  MCSPI_write_reg(dev->base_addr, channel_conf, val);
}

/*
core function for enabling the clock to the SPI module. DO NOT directly call this.
Call the enable_clock() instead
*/
static inline void __set_clock(void __iomem *clock_base, u32 offset, bool enable)
{
  u32 val;

  val = MCSPI_read_reg(clock_base, offset);
  val &= CM_PER_SPI0_CLKCTRL_MODULEMODE(0x03);
  if(enable)
    val |= CM_PER_SPI0_CLKCTRL_MODULEMODE(CM_PER_SPI0_CLKCTRL_MODULEMODE_ENABLE);
  else
    val |= CM_PER_SPI0_CLKCTRL_MODULEMODE(CM_PER_SPI0_CLKCTRL_MODULEMODE_DISABLE);

  MCSPI_write_reg(clock_base, offset, val);
}


/*..............................................................................
    @breif:      enable SPI0 clock
    @parameters: base_addr: The base address of CM_PER registers
                 offset:    the offset of the CM_PER_SPI(0/1)_CLKCTRL register
    @return:     void
..............................................................................*/
inline void enable_clock(void __iomem *clock_base, u32 offset)
{
  __set_clock(clock_base, offset, 1);
}

/*..............................................................................
    @breif:      disable SPI0 clock
    @parameters: base_addr: The base address of CM_PER registers
                 offset:    the offset of the CM_PER_SPI(0/1)_CLKCTRL register
    @return:     void
..............................................................................*/
inline void disable_clock(void __iomem *clock_base, u32 offset)
{
  __set_clock(clock_base, offset, 0);
}


/*..............................................................................
    @breif:      set given bit(s) of given address. This is unsafe and user has
                 to make sure of correct memory address
    @parameters: addr: The address of register
                 bit:  the bit(s) to be set, indicated by 1s in the u32 value
    @return:     void
..............................................................................*/
void inline MCSPI_set_bit(void __iomem *addr, u32 bit)
{
  u32 val;
  val = ioread32(addr);
  iowrite32(val | bit, addr);
}

/*..............................................................................
    @breif:      reset given bit(s) of given address. This is unsafe and user has
                 to make sure of correct memory address
    @parameters: addr: The address of register
                 bit:  the bit(s) to be reset, indicated by 1s in the u32 value
    @return:     void
..............................................................................*/
void inline MCSPI_reset_bit(void __iomem *addr, u32 bit)
{
  u32 val;
  val = ioread32(addr);
  iowrite32(~bit & val, addr);
}

/*..............................................................................
    @breif:      waits for the given bit(s) to set befor the given timeout,
                 else returns -1
    @parameters: addr: The register address to look for
                 bit:  the bit for which we are supposed to wait
                 timeout: timeout in milliseconds
    @return:     0 on success; -1 on error;  -2 on NULL addr
..............................................................................*/
int MCSPI_wait_for_bit_set(void __iomem *addr, u32 bit, unsigned int timeout)
{
  unsigned long timeout_local = jiffies + msecs_to_jiffies(timeout);

  if(!addr)
    return -2;

  while(!(ioread32(addr) & bit))
  {
    if(time_after(jiffies, timeout_local))
    {
      return -1;
    }
    cpu_relax();
  }
  return 0;
}


/*..............................................................................
    @breif:      waits for the given bit(s) to reset befor the given timeout,
                 else returns -1
    @parameters: addr: The register address to look for
                 bit:  the bit for which we are supposed to wait
                 timeout: timeout in milliseconds
    @return:     0 on success; -1 on error
..............................................................................*/
int MCSPI_wait_for_bit_reset(void __iomem *addr, u32 bit, unsigned int timeout)
{
  unsigned long timeout_local = jiffies + msecs_to_jiffies(timeout);

  while((ioread32(addr) & bit))
  {
    if(time_after(jiffies, timeout_local))
    {
      return -1;
    }
    cpu_relax();
  }
  return 0;
}




/*..............................................................................
    @breif:      Sets the SPI mode MASTER or SLAVE
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_mode_set(struct MCSPI *dev)
{
  u32 val=0;
  //Check for validity of the mode. If invalid do nothing
  if(dev->role == MCSPI_MODULCTRL_MASTER  ||  dev->role == MCSPI_MODULCTRL_SLAVE)
  {
    val = MCSPI_read_reg(dev->base_addr, MCSPI_MODULCTRL);
    val &= ~MCSPI_MODULCTRL_MS(1);
    val |= MCSPI_MODULCTRL_MS(dev->role);
    MCSPI_write_reg(dev->base_addr, MCSPI_MODULCTRL, val);

    /*
    val = MCSPI_read_reg(dev->base_addr, MCSPI_MODULCTRL);
    if( ( (val & 1<<2)>>2) == MCSPI_MODULCTRL_MASTER)
      DEBUG_NORM("MASTER_MODE\n");
    else
     DEBUG_NORM("SLAVE_MODE\n");
     */
  }

  switch(dev->channel_number)
  {
    default:
    case 0: __set_tx_rx(dev, MCSPI_CH0CONF);  break;
    case 1: __set_tx_rx(dev, MCSPI_CH1CONF);  break;
    case 2: __set_tx_rx(dev, MCSPI_CH2CONF);  break;
    case 3: __set_tx_rx(dev, MCSPI_CH3CONF);  break;
  }


  return;
}


/*..............................................................................
    @breif:      Sets the SPI word length for transfer
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_wl_set(struct MCSPI *dev)
{
  u32 val;
  u32 channel_conf = 0;

  switch(dev->channel_number)
  {
    default:
    case 0: channel_conf = MCSPI_CH0CONF;   break;
    case 1: channel_conf = MCSPI_CH1CONF;   break;
    case 2: channel_conf = MCSPI_CH2CONF;   break;
    case 3: channel_conf = MCSPI_CH3CONF;   break;
  }

  val = MCSPI_read_reg(dev->base_addr, channel_conf);

  val &= ~MCSPI_CHCONF_WL(0x1F);
  val |= MCSPI_CHCONF_WL(dev->word_length);

  MCSPI_write_reg(dev->base_addr, channel_conf, val);
}


/*..............................................................................
    @breif:      Sets the SPI polarity
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_pol_pha_set(struct MCSPI *dev)
{
  u32 val=0;
  u32 channel_conf;

  u32 pin_dir = 0xFFFFFFFF;

  if (dev->pin_direction == MCSPI_D0_IN_D1_OUT) {
		pin_dir &= ~MCSPI_CHCONF_IS(1);
		pin_dir &= ~MCSPI_CHCONF_DPE1(1);
		pin_dir |= MCSPI_CHCONF_DPE0(1);
	} else {
		pin_dir |= MCSPI_CHCONF_IS(1);
		pin_dir |= MCSPI_CHCONF_DPE1(1);
		pin_dir &= ~MCSPI_CHCONF_DPE0(1);
  }

  switch(dev->channel_number)
  {
    default:
    case 0: channel_conf = MCSPI_CH0CONF;  break;
    case 1: channel_conf = MCSPI_CH1CONF;  break;
    case 2: channel_conf = MCSPI_CH2CONF;  break;
    case 3: channel_conf = MCSPI_CH3CONF;  break;
  }

  val = MCSPI_read_reg(dev->base_addr, channel_conf);
  val &= ~MCSPI_CHCONF_POL(1) & ~MCSPI_CHCONF_PHA(1);
  val |= MCSPI_CHCONF_POL((bool)dev->polarity) | MCSPI_CHCONF_PHA((bool)dev->phase);
  MCSPI_write_reg(dev->base_addr, channel_conf, val & pin_dir);

  return;
}


/*..............................................................................
    @breif:      Enables or disables the SPI module
    @parameters: dev: the device struct for the SPI module
                 enable: can be 0/1 for disable/enable
    @return:     void
..............................................................................*/
void MCSPI_enable(struct MCSPI *dev, u8 enable)
{
  u32 val;

  switch(dev->channel_number)
  {
    case 0: val = MCSPI_read_reg(dev->base_addr, MCSPI_CH0CTRL);
            MCSPI_write_reg(dev->base_addr, MCSPI_CH0CTRL, MCSPI_CHCTRL_EN(enable));
            break;
    case 1: val = MCSPI_read_reg(dev->base_addr, MCSPI_CH1CTRL);
            MCSPI_write_reg(dev->base_addr, MCSPI_CH1CTRL, MCSPI_CHCTRL_EN(enable));
            break;
    case 2: val = MCSPI_read_reg(dev->base_addr, MCSPI_CH2CTRL);
            MCSPI_write_reg(dev->base_addr, MCSPI_CH2CTRL, MCSPI_CHCTRL_EN(enable));
            break;
    case 3: val = MCSPI_read_reg(dev->base_addr, MCSPI_CH3CTRL);
            MCSPI_write_reg(dev->base_addr, MCSPI_CH3CTRL, MCSPI_CHCTRL_EN(enable));
            break;

    default:  DEBUG_ALERT("%s: Enable: Incorrect Channel Number\n",DRIVER_NAME);
              break;
  }
}



/*..............................................................................
    @breif:      Software reset the module
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_reset(struct MCSPI *dev)
{
  MCSPI_set_bit(dev->base_addr + MCSPI_SYSCONFIG, 0x02);
  if( MCSPI_wait_for_bit_set(dev->base_addr + MCSPI_SYSSTATUS, 0x01, 100) <0 )
    DEBUG_ALERT("%s: Reset: timout\n", DRIVER_NAME);
}


/*..............................................................................
    @breif:      Chip select polarity and pin_selection
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_Set_CS(struct MCSPI *dev)
{
	u32 val;
  u32 channel_conf;

  switch(dev->channel_number)
  {
    default:
    case 0: channel_conf = MCSPI_CH0CONF;  break;
    case 1: channel_conf = MCSPI_CH1CONF;  break;
    case 2: channel_conf = MCSPI_CH2CONF;  break;
    case 3: channel_conf = MCSPI_CH3CONF;  break;
  }

  val = MCSPI_read_reg(dev->base_addr, channel_conf);
  if (dev->CS_polarity == MCSPI_CS_ACTIVE_LOW)
    val |= MCSPI_CHCONF_EPOL(1);
  else
    val &= ~MCSPI_CHCONF_EPOL(1);
  MCSPI_write_reg(dev->base_addr, channel_conf, val);


	val = MCSPI_read_reg(dev->base_addr, MCSPI_MODULCTRL);
	if (dev->CS_sensitive == MCSPI_CS_SENSITIVE_ENABLED)
		val &= ~MCSPI_MODULCTRL_PIN34(1);
	else if(dev->CS_sensitive == MCSPI_CS_SENSITIVE_DISABLED)
		val |= MCSPI_MODULCTRL_PIN34(1);
	MCSPI_write_reg(dev->base_addr, MCSPI_MODULCTRL, val);
}


/*..............................................................................
    @breif:      Set Clock divider
    @parameters: dev: the device struct for the SPI module
    @return:     void
..............................................................................*/
void MCSPI_Set_CLKD(struct MCSPI *dev)
{
  u32 val;
  u32 restore;
  u32 channel_conf;

  switch(dev->channel_number)
  {
    default:
    case 0: channel_conf = MCSPI_CH0CONF;  break;
    case 1: channel_conf = MCSPI_CH1CONF;  break;
    case 2: channel_conf = MCSPI_CH2CONF;  break;
    case 3: channel_conf = MCSPI_CH3CONF;  break;
  }

  if(dev->role == MCSPI_MODULCTRL_MASTER)
  {
    if(dev->clock_div >= CLK_1  && dev->clock_div <=CLK_32768)
    {
      restore = MCSPI_read_reg(dev->base_addr, channel_conf);
      restore = restore & MCSPI_CHCTRL_EN(1);
      restore = restore ? 1 : 0;

      //Disable the device before changing the clock frequency
      MCSPI_enable(dev, 0);

      val = MCSPI_read_reg(dev->base_addr, channel_conf);
      val &= ~MCSPI_CHCONF_CLKD(dev->clock_div);
      val |= MCSPI_CHCONF_CLKD(dev->clock_div);

      MCSPI_write_reg(dev->base_addr, channel_conf, val);

      //restore the status of the SPI driver.
      MCSPI_enable(dev, restore);
    }
  }
}


/** @brief The IRQ handler for the MCSPI controller
 *  @param irq: the IRQ numer which called this handler
 *         dev_id: pointer to temp_struct
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
irq_handler_t MCSPI_irq_handler(unsigned int irq, void *dev_id)
{
  struct MCSPI_data *mcspi_data = (struct MCSPI_data *)dev_id;
  struct MCSPI *mcspi = (struct MCSPI *)mcspi_data->device;

  u32 val;

  switch(mcspi->channel_number)
  {
    case 0:  val = MCSPI_read_reg(mcspi->base_addr, MCSPI_CH0STAT);
             if(val & MCSPI_CHSTAT_EOT_MASK)
             {
               DEBUG_NORM("%s: IRQ: ch 0 EOT set\n", DRIVER_NAME);
             }
             break;

    case 1:  val = MCSPI_read_reg(mcspi->base_addr, MCSPI_CH1STAT);
             if(val & MCSPI_CHSTAT_EOT_MASK)
             {
               DEBUG_NORM("%s: IRQ: ch 1 EOT set\n", DRIVER_NAME);
             }
             break;

    case 2:  val = MCSPI_read_reg(mcspi->base_addr, MCSPI_CH2STAT);
             if(val & MCSPI_CHSTAT_EOT_MASK)
             {
               DEBUG_NORM("%s: IRQ: ch 2 EOT set\n", DRIVER_NAME);
             }
             break;

    case 3:  val = MCSPI_read_reg(mcspi->base_addr, MCSPI_CH3STAT);
             if(val & MCSPI_CHSTAT_EOT_MASK)
             {
               DEBUG_NORM("%s: IRQ: ch 3 EOT set\n", DRIVER_NAME);
             }
             break;

    default: DEBUG_ALERT("%s: IRQ: incorrect channel number\n", DRIVER_NAME);
             break;
  }

  return (irq_handler_t) IRQ_HANDLED;
}
