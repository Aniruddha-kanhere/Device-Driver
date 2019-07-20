/*
* @file    cm_per.h
* @author  Aniruddha Kanhere
* @date    13 July 2019
* @version 1
* @brief   This CM_PER_MODULE (clocking) registers and functions are just for
*          the use of MCSPI module. This is NOT a full library
*/
#ifndef __CM_PER_H__
#define __CM_PER_H__

#define CM_PER_START 0x44E00000
#define CM_PER_END   0x44E003FF
#define CM_PER_SIZE  CM_PER_END - CM_PER_START


//Much more registers but we just need this one for now
#define CM_PER_SPI0_CLKCTRL   0x4C


#define CM_PER_SPI0_CLKCTRL_MODULEMODE(val)      ((u32)val<<0)
#define CM_PER_SPI0_CLKCTRL_MODULEMODE_DISABLE   0x00
#define CM_PER_SPI0_CLKCTRL_MODULEMODE_ENABLE    0x02

#endif
