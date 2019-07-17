# @file    Makefile
# @author  Aniruddha Kanhere
# @date    13 July 2019
# @version 1
# @brief   This Makefile makes the module and compiles the dependencies listed
#          in SPI-objs. It also compiles the test program meant to test the
#          working of SPI module by sending data

DEPS = MCSPI_reg.h
TESTOBJ = testSPI
%.o: %.c $(DEPS)
	$(CC) -o $@ $<

obj-m+=SPI.o
SPI-objs := MCSPI_mod.o MCSPI_reg.o MCSPI_misc.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) $(TESTOBJ).c -o $(TESTOBJ)

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm $(TESTOBJ)
