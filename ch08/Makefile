obj-m += lab1_pdrv.o

all:
	make -C/lib/modules/$(shell uname -r)/build SUBDIRS=$(shell pwd) modules

