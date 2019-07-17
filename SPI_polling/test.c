#include "./my_gpio.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include<stdio.h>

Offset_define(GPIO);

#define CONTROL_MODULE_START_ADDR 0x44E10000
#define CONTROL_MODULE_END_ADDR   0x44E11FFF
#define CONTROL_MODULE_SIZE       (CONTROL_MODULE_END_ADDR - CONTROL_MODULE_START_ADDR)

#define  CONF_GPMC_AD7		0x81C

#define PIN 		(5)
#define GPIO_NUMBER	(0)

#define SIZE     	GPIO0_SIZE
#define STR_ADDR 	GPIO0_START_ADDR


int main()
{

	volatile void *gpio_addr, *control_addr;

	int fd = open("/dev/mem", O_RDWR);
	gpio_addr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, STR_ADDR);

	volatile unsigned int *REG;

	REG = (volatile unsigned int *)(gpio_addr + GPIO_OE);
        *REG &= ~BIT(PIN);

	printf("0x%08x", *REG);
	printf("  .... Printing OE \n");

/*	int fd1 = open("/dev/mem", O_RDWR);
	control_addr = mmap(0, CONTROL_MODULE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, CONTROL_MODULE_START_ADDR);

	volatile unsigned int *Control;
	Control = (volatile unsigned int *)(control_addr + CONF_GPMC_AD7);

	printf("0x%08x", *Control);
	printf("  .... Printing \n");

	*Control &= ~(BIT(5) | 0x07);
	*Control = 0x00;

	printf("0x%08x", *Control);
	printf("  .... Printing \n");

*/

	REG = (volatile unsigned int *)(gpio_addr + GPIO_DATAOUT);

	volatile unsigned long int i;

	while(1)
	{
		//i=100000000;
		//while(i--);
		*REG &= ~BIT(PIN);
		//printf("Set:   0x%08x\n", *REG);

		//i=100000000;
		//while(i--);	
		*REG |= BIT(PIN);
		//printf("ReSet: 0x%08x\n", *REG);	
	}

	return 0;
}
