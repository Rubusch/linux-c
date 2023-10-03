KERNELDIR ?= /usr/src/linux
obj-m += hellochardev.o
ccflags-y += -include $(kbuild-dir)/hellochardev.h

APPLICATION ?= tester.elf
HEADERS += hellochardev.h
CC ?= gcc
HOSTCC ?= gcc

all: default

default: module $(APPLICATION)

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

$(APPLICATION): application.c
	$(CROSS_COMPILE)$(HOSTCC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete
	rm -f *.elf

.PHONY: all default module clean
