obj-m += lab1_pdrv.o
obj-m += lab2_pdrv.o
obj-m += lab2_pdrv_2.o
obj-m += lab2_pdrv_3.o

all:
	make -C/lib/modules/$(shell uname -r)/build SUBDIRS=$(shell pwd) modules
