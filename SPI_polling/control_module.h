/*
* @file    control_module.h
* @author  Aniruddha Kanhere
* @date    13 July 2019
* @version 1
* @brief   This CONTROL_MODULE registers and functions are just for the use of
*          MCSPI module. This is NOT a full library
*/

#ifndef __CONTROL_MODULE_H__
#define __CONTROL_MODULE_H__


#define CONTROL_MODULE_START               0x44E10000
#define CONTROL_MODULE_END                0x44E11FFF
#define CONTROL_MODULE_SIZE               CONTROL_MODULE_END - CONTROL_MODULE_START
#define CONF_SPI0_SCLK_OFFSET             0x950
#define CONF_SPI0_D0_OFFSET               0x954
#define CONF_SPI0_D1_OFFSET               0x958
#define CONF_SPI0_CS0_OFFSET              0x95C
#define CONF_SPI0_CS1_OFFSET              0x960

#define CONF_MODULE_PIN_SLEWCTRL(val)     (((u32)val)<<6)
#define CONF_MODULE_PIN_RXACTIVE(val)     (((u32)val)<<5)
#define CONF_MODULE_PIN_PUTYPESEL(val)    (((u32)val)<<4)
#define CONF_MODULE_PIN_PUDEN(val)        (((u32)val)<<3)
#define CONF_MODULE_PIN_MMODE(val)        (((u32)val)<<0)

#endif
