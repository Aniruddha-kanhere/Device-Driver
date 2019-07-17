/**
**********************************************************************
  Copyright (C) 2019 Aniruddha Kanhere
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation.
**********************************************************************

 * @file   testSPI.c
 * @author Aniruddha Kanhere
 * @date   17 July 2019
 * @version 1
 * @brief  A Linux user space program that communicates with the SPI LKM. It passes a
 * string to the LKM which is sent using the SPI protocol. For this test to work the device
 * must be called /dev/MCSPI. 
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(){
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   printf("Starting device test code example...\n");
   fd = open("/dev/MCSPI", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   printf("Type in a short string to send to the kernel module:\n");
   scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)
   printf("Writing message to the device [%s].\n", stringToSend);
   ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
   if (ret < 0){
      perror("Failed to write the message to the device.");
      return errno;
   }
   
   close(fd);
   return 0;
}
