KERNELDIR ?= /usr/src/linux

## userspace application
APPLICATION ?= ioctl.elf
HEADERS += ioctl.h
CC ?= gcc
HOSTCC ?= gcc

all: default

default: module $(APPLICATION)

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

## userspace application
$(APPLICATION): application.c
	$(CROSS_COMPILE)$(HOSTCC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete
	rm -f $(APPLICATION)

.PHONY: all default module clean
