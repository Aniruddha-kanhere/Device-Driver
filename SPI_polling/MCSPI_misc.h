/*
* @file    MCSPI_misc.h
* @author  Aniruddha Kanhere
* @date    13 July 2019
* @version 1
* @brief   Prototypes of Interrupts, other structs, functions for the MCSPI
*          device driver
*/

#ifndef _MCSPI_MISC_H_
#define _MCSPI_MISC_H_

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/ioport.h>         // Required for request_mem_region
#include <asm/io.h> 		          // Required for ioremap/ unmap etc.

#include "MCSPI_reg.h"
#include "control_module.h"

#define CONFIGURE_SUCCESS         0
#define CONFIGURE_FAIL            1
#define MAX_BUFFER_LENGTH         50

struct MCSPI_msg{
  char *msg;
  int buffer_length;
};

struct MCSPI_data {
  dev_t  device_id;
  int numberOpens;
  struct MCSPI *device;
  struct MCSPI_msg *msg;
};

/*..............................................................................
    @breif:      Configure the whole module with settings
    @parameters: mcspi: struct containing all the parameters to be passed on
    @return:     CONFIGURE_SUCCESS/CONFIGURE_FAIL
..............................................................................*/
int MCSPI_configure(struct MCSPI *mcspi);


int MCSPI_send_data_poll(struct MCSPI *dev, char* msg, int len);

#endif
