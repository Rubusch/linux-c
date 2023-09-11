KERNELDIR ?= /lib/modules/$(shell uname -r)/build

## userspace application
APPLICATION ?= ioctl.exe
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
	rm -f *.exe

.PHONY: all default module clean
