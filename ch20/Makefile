NAME = lab1_pdrv

obj-m += $(NAME).o

all:
	make -C/lib/modules/$(shell uname -r)/build SUBDIRS=$(shell pwd) modules

