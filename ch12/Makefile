obj-m += lab1_pdrv.o
obj-m += lab1_drv_c1.o
obj-m += lab1_drv_c2.o
obj-m += integrity_test.o

all:
	make -C/lib/modules/$(shell uname -r)/build SUBDIRS=$(shell pwd) modules

