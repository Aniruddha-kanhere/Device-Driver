#ifndef __MCSPI_IOCTL_H__
#define __MCSPI_IOCTL_H__

#include <linux/types.h>

#define MCSPI_MAGIC_NUMBER 'k'

#define MCSPI_MODE_SET           _IOW(MCSPI_MAGIC_NUMBER, 1, __u8)
#define MCSPI_MODE_GET           _IOR(MCSPI_MAGIC_NUMBER, 2, __u8)

#define MCSPI_POL_SET            _IOW(MCSPI_MAGIC_NUMBER, 3, __u8)
#define MCSPI_POL_GET            _IOR(MCSPI_MAGIC_NUMBER, 4, __u8)

#define MCSPI_PHA_SET            _IOW(MCSPI_MAGIC_NUMBER, 5, __u8)
#define MCSPI_PHA_GET            _IOR(MCSPI_MAGIC_NUMBER, 6, __u8)

#define MCSPI_PIN_CONFIG_SET     _IOW(MCSPI_MAGIC_NUMBER, 7, __u8)
#define MCSPI_PIN_CONFIG_GET     _IOR(MCSPI_MAGIC_NUMBER, 8, __u8)

#define MCSPI_CLKD_SET           _IOW(MCSPI_MAGIC_NUMBER, 9, __u8)
#define MCSPI_CLKD_GET           _IOR(MCSPI_MAGIC_NUMBER, 10, __u8)

#define MCSPI_CS_SET             _IOW(MCSPI_MAGIC_NUMBER, 11, __u8)
#define MCSPI_CS_GET             _IOR(MCSPI_MAGIC_NUMBER, 12, __u8)

#define MCSPI_TRM_SET            _IOW(MCSPI_MAGIC_NUMBER, 13, __u8)
#define MCSPI_TRM_GET            _IOR(MCSPI_MAGIC_NUMBER, 14, __u8)

#define MAX_IOCTL_NUMBER         15


 /*
 *   One can use the below defined macros for ioctl command arguments (the arg
 *   value). These are defined in the header file MCSPI_reg.h.
 */
#define USER_SPACE       //this stops definition of various libraries and functions
                         //included in MCSPI_reg.h when compiling for userland
                         //programs. (Compiler complains if you do define it)

#include "MCSPI_reg.h"

#define MCSPI_MODE_MASTER                     MCSPI_MODULCTRL_MASTER
#define MCSPI_MODE_SLAVE                      MCSPI_MODULCTRL_SLAVE

#define MCSPI_PHA_ODD                         MCSPI_CHCONF_PHA_ODD
#define MCSPI_PHA_EVEN                        MCSPI_CHCONF_PHA_EVEN

#define MCSPI_POL_ACTIVE_HIGH                 MCSPI_CHCONF_POL_ACTIVE_HIGH
#define MCSPI_POL_ACTIVE_LOW                  MCSPI_CHCONF_POL_ACTIVE_LOW

#define CLK_DIV_1                             CLK_1
#define CLK_DIV_2                             CLK_2
#define CLK_DIV_4                             CLK_4
#define CLK_DIV_8                             CLK_8
#define CLK_DIV_16                            CLK_16
#define CLK_DIV_32                            CLK_32
#define CLK_DIV_64                            CLK_64
#define CLK_DIV_128                           CLK_128
#define CLK_DIV_256                           CLK_256
#define CLK_DIV_512                           CLK_512
#define CLK_DIV_1024                          CLK_1024
#define CLK_DIV_2048                          CLK_2048
#define CLK_DIV_4096                          CLK_4096
#define CLK_DIV_8192                          CLK_8192
#define CLK_DIV_16384                         CLK_16384
#define CLK_DIV_32768                         CLK_32768

#define MCSPI_PIN_CONFIG_D0_IN_D1_OUT         MCSPI_D0_IN_D1_OUT
#define MCSPI_PIN_CONFIG_D1_IN_D0_OUT         MCSPI_D1_IN_D0_OUT

#define MCSPI_CS_DISABLED                     MCSPI_CS_SENSITIVE_DISABLED
#define MCSPI_CS_ENABLED                      MCSPI_CS_SENSITIVE_ENABLED

#define MCSPI_TRM_TX                          MCSPI_CHCONF_TRM_TX
#define MCSPI_TRM_RX                          MCSPI_CHCONF_TRM_RX
#define MCSPI_TRM_TX_RX                       MCSPI_CHCONF_TRM_TX_RX

#undef  USER_SPACE

#endif
