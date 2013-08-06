NAME = lab1a_pdrv

obj-m += $(NAME).o

all:
	make -C/lib/modules/$(shell uname -r)/build SUBDIRS=$(shell pwd) modules

install:
	sudo insmod $(NAME).ko

uninstall:
	sudo rmmod $(NAME)

