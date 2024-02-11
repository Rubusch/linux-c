KERNELDIR ?= /usr/src/linux

## userspace application
APPLICATION ?= signalee.elf
HEADERS += gpio_irq_signaler.h
CC ?= gcc
HOSTCC ?= gcc

all: default

default: module $(APPLICATION)

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

$(APPLICATION): signalee.c
	$(CROSS_COMPILE)$(HOSTCC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete
	rm -f $(APPLICATION)

.PHONY: all default module clean
