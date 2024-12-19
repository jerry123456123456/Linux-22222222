#!/bin/bash

ccflags_y += -O2

ifneq ($(KERNELRELEASE),)
obj-m := ntychannel.o
else
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 
endif

clean:
	rm -rf *.o *.ko *.mod.c

depend .depend dep:
	$(CC)  -M *.c > .depend 

