/*
* @file    MCSPI_misc.c
* @author  Aniruddha Kanhere
* @date    13 July 2019
* @version 1
* @brief   definitions of other structs, miscellaneous functions for the MCSPI
*          device driver
*/

#include "MCSPI_misc.h"
#include <stdarg.h>

MODULE_LICENSE      ("GPL v2");                           ///< The license type -- this affects available functionality
MODULE_AUTHOR       ("Aniruddha Kanhere");              ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION  ("A MCSPI LKM misc files for the BBB");  ///< The description -- see modinfo
MODULE_VERSION      ("1.0");                           ///< A version number to inform users


/*..............................................................................
    @breif:      Send the data using polling
    @parameters: dev: struct defining device
                 msg: the char* msg which you want to send
                 len: the length of the message you'll be 
                      sending
    @return:     CONFIGURE_SUCCESS/CONFIGURE_FAIL
..............................................................................*/
int MCSPI_send_data_poll(struct MCSPI *dev, char* msg, int len)
{
  struct MCSPI_data data_var;
  struct MCSPI_data *mcspi_data = &data_var;
  long int timeout = ( ((u32)(dev->clock_div+1)) * 2 * 1000 * (dev->word_length+1) )/48000000;
  //clock_dividor * factor_of_safety * convertsion_to_ms * number_of_bits /Clock_speed

  int bytes;
  void __iomem *channel_stat = NULL;
  u32 channel_tx = 0, channel_rx = 0;

  mcspi_data->device = dev;

  if(timeout == 0)
    timeout = 1;

  switch(dev->channel_number)
  {
    default:
    case 0: channel_stat = dev->base_addr + MCSPI_CH0STAT;
            channel_tx = MCSPI_TX0;
            channel_rx = MCSPI_RX0;
            break;
    case 1: channel_stat = dev->base_addr + MCSPI_CH1STAT;
            channel_tx = MCSPI_TX1;
            channel_rx = MCSPI_RX1;
            break;
    case 2: channel_stat = dev->base_addr + MCSPI_CH2STAT;
            channel_tx = MCSPI_TX2;
            channel_rx = MCSPI_RX2;
            break;
    case 3: channel_stat = dev->base_addr + MCSPI_CH3STAT;
            channel_tx = MCSPI_TX3;
            channel_rx = MCSPI_RX3;
            break;
  }

  DEBUG_NORM("%s: Send: sending %d characters\n", DRIVER_NAME, len);

  for(bytes = 0 ; bytes < len ; bytes++)
  {
    MCSPI_write_reg(dev->base_addr, channel_tx, (u32)msg[bytes]);
    mb();

    DEBUG_NORM("Send: sent %c -- \n\n", msg[bytes]);

    if(MCSPI_wait_for_bit_set(channel_stat, MCSPI_CHSTAT_TXS_MASK, timeout) < 0)
      return -ETIME;


    if(dev->tx_rx == MCSPI_CHCONF_TRM_RX || dev->tx_rx == MCSPI_CHCONF_TRM_TX_RX)
    {
      if(MCSPI_wait_for_bit_set(channel_stat, MCSPI_CHSTAT_RXS_MASK, timeout) < 0)
        return -ETIME;

      msg[bytes] = MCSPI_read_reg(dev->base_addr, channel_rx);
    }
  }

  if(MCSPI_wait_for_bit_set(channel_stat, MCSPI_CHSTAT_EOT_MASK, timeout) < 0)
    return -ETIME;

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
    DEBUG_ALERT("%s: Config: Wrong mode selected (MASTER/SLAVE)\n", DRIVER_NAME);
    return CONFIGURE_FAIL;
  }

  if(mcspi->clock_div >= CLK_1   &&  mcspi->clock_div <= CLK_32768)
    MCSPI_Set_CLKD(mcspi);
  else
    DEBUG_ALERT("%s: Config: wrong clock parameter\n", DRIVER_NAME);

  if((mcspi->word_length==MCSPI_CHCONF_WL_8BIT) ||
     (mcspi->word_length==MCSPI_CHCONF_WL_16BIT) ||
     (mcspi->word_length==MCSPI_CHCONF_WL_32BIT))
  {
    MCSPI_wl_set(mcspi);
  }
  else
    DEBUG_ALERT("%s: Config: wrong wl parameter\n", DRIVER_NAME);

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
          DEBUG_ALERT("%s: Config: Incorrect phase\n", DRIVER_NAME);
          return CONFIGURE_FAIL;
        }
      }
      else
      {
        DEBUG_ALERT("%s: Config: Incorrect polarity\n", DRIVER_NAME);
        return CONFIGURE_FAIL;
      }
    }
    else
    {
      DEBUG_ALERT("%s: Config: Pin direction incorrect\n", DRIVER_NAME);
      return CONFIGURE_FAIL;
    }
  }
  else{
    DEBUG_ALERT("%s: Config: Incorrect channel number (0-3)\n", DRIVER_NAME);
    return CONFIGURE_FAIL;
  }
}
