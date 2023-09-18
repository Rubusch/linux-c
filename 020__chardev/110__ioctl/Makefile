KERNELDIR ?= /usr/src/linux

## userspace application
APPLICATION ?= ioctl.elf
HEADERS += ioctl.h
CC ?= gcc

all: default

default: module $(APPLICATION)

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

## userspace application
$(APPLICATION): application.c
	$(CC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete
	rm -f *.elf

.PHONY: all default module clean
