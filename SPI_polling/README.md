# **SPI_polling**
Sending data over SPI using polling. One byte at a time...

**********************************************************************
  Copyright (C) 2019 Aniruddha Kanhere
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation.
**********************************************************************
  
This section of the repository has the polling version of SPI data transfer from the MCSPI0 module of the beaglebone black. The user can (currently) configure the module by directly modifying the (struct MCSPI) mcspi present in the MCSPI_mod.c file. For available configuration options one can look into the MCSPI_reg.h file which has definitions of the registers and the values it can possibly take.

The ioctl commands are defined in the MCSPI_ioctl.h file which has to be included in userspace programs as well as the kernel code. The commands and arguments are defined using the existing definition in MCSPI_reg.h. (USER_SPACE stops compilation of non-user space libraries while the program is being compiled for the userland program(s).)

Also (in near) future, I will be uploading the Interrupt driven versions of the SPI device driver (in another directory within the same repo).

If you just want to transmit some message with the given (default) configuration, just type "make" after traversing to the directory of this project. If all goes well, you'll have an executable file called testSPI (Huzzah!!). Just execute it using "./testSPI" and follow the commands.