/*
* @file    MCSPI_misc.c
* @author  Aniruddha Kanhere
* @date    13 July 2019
* @version 1
* @brief   definitions of other structs, miscellaneous functions for the MCSPI
*          device driver
*/

#include "MCSPI_misc.h"

MODULE_LICENSE      ("GPL v2");                           ///< The license type -- this affects available functionality
MODULE_AUTHOR       ("Aniruddha Kanhere");              ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION  ("A MCSPI LKM misc files for the BBB");  ///< The description -- see modinfo
MODULE_VERSION      ("1.0");                           ///< A version number to inform users


int MCSPI_send_data_poll(struct MCSPI *dev, char* msg, int len)
{
  struct MCSPI_data data_var;
  struct MCSPI_data *mcspi_data = &data_var;

  int bytes = 0;
  void __iomem *channel_stat = NULL;

  mcspi_data->device = dev;

  switch(dev->channel_number)
  {
    default:
    case 0: channel_stat = dev->base_addr + MCSPI_CH0STAT;   break;
    case 1: channel_stat = dev->base_addr + MCSPI_CH1STAT;   break;
    case 2: channel_stat = dev->base_addr + MCSPI_CH2STAT;   break;
    case 3: channel_stat = dev->base_addr + MCSPI_CH3STAT;   break;
  }

  pr_info("%s: Send: sending %d characters\n", DRIVER_NAME, len);

  for( ; bytes < len ; bytes++)
  {
    //c = (u8)MCSPI_read_reg(dev->base_addr, MCSPI_RX0);
    MCSPI_write_reg(dev->base_addr, MCSPI_TX0, (u32)msg[bytes]);
    mb();
    //msg[bytes] = c;
    pr_info("Send: sent %c -- \n\n", msg[bytes]);


    //mbw();



    //while(i--);
    //i = 30000;

    /*if(MCSPI_wait_for_bit_reset(channel_conf, MCSPI_CHSTAT_TXS_MASK, 200) < 0)
    {
      pr_alert("%s: send: not even resetting\n", DRIVER_NAME);
      return -1;
    }*/

    if(MCSPI_wait_for_bit_set(channel_stat, MCSPI_CHSTAT_TXS_MASK, 1000) < 0)
    {
      pr_info("timeout\n");
    }

    //MCSPI_irq_handler(1, mcspi_data);

  }

  return 0;
}



/*..............................................................................
    @breif:      Configure the whole module with settings
    @parameters: mcspi: struct containing all the parameters to be passed on
    @return:     CONFIGURE_SUCCESS/CONFIGURE_FAIL
..............................................................................*/
int MCSPI_configure(struct MCSPI *mcspi)
{
  MCSPI_reset(mcspi);

  //MCSPI_set_bit(mcspi->base_addr+MCSPI_CH0CONF, (0x03UL<<19));

  if(mcspi->role == MCSPI_MODULCTRL_MASTER || mcspi->role == MCSPI_MODULCTRL_SLAVE)
    MCSPI_mode_set(mcspi);
  else
  {
    pr_alert("%s: Config: Wrong mode selected (MASTER/SLAVE)\n", DRIVER_NAME);
    return CONFIGURE_FAIL;
  }

  if(mcspi->clock_div >= CLK_1   &&  mcspi->clock_div <= CLK_32768)
    MCSPI_Set_CLKD(mcspi);
  else
    pr_alert("%s: Config: wrong clock parameter\n", DRIVER_NAME);

  if((mcspi->word_length==MCSPI_CHCONF_WL_8BIT) ||
     (mcspi->word_length==MCSPI_CHCONF_WL_16BIT) ||
     (mcspi->word_length==MCSPI_CHCONF_WL_32BIT))
  {
    MCSPI_wl_set(mcspi);
  }
  else
    pr_alert("%s: Config: wrong wl parameter\n", DRIVER_NAME);

  if(mcspi->CS_polarity == MCSPI_CS_ACTIVE_LOW || mcspi->CS_polarity == MCSPI_CS_ACTIVE_HIGH)
    MCSPI_Set_CS(mcspi);

  if(mcspi->channel_number>=0 && mcspi->channel_number<=4)
  {
    if(mcspi->pin_direction == MCSPI_D0_IN_D1_OUT || mcspi->pin_direction == MCSPI_D1_IN_D0_OUT)
    {
      if(mcspi->phase == MCSPI_CHCONF_PHA_ODD || mcspi->phase == MCSPI_CHCONF_PHA_EVEN)
      {
        if(mcspi->phase == MCSPI_CHCONF_POL_ACTIVE_HIGH || mcspi->phase == MCSPI_CHCONF_POL_ACTIVE_LOW)
        {
          MCSPI_pol_pha_set(mcspi);
          return CONFIGURE_SUCCESS;
        }
        else
        {
          pr_alert("%s: Config: Incorrect phase\n", DRIVER_NAME);
          return CONFIGURE_FAIL;
        }
      }
      else
      {
        pr_alert("%s: Config: Incorrect polarity\n", DRIVER_NAME);
        return CONFIGURE_FAIL;
      }
    }
    else
    {
      pr_alert("%s: Config: Pin direction incorrect\n", DRIVER_NAME);
      return CONFIGURE_FAIL;
    }
  }
  else{
    pr_alert("%s: Config: Incorrect channel number (0-3)\n", DRIVER_NAME);
    return CONFIGURE_FAIL;
  }
}
