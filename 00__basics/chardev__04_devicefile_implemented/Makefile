KERNELDIR ?= /lib/modules/$(shell uname -r)/build
obj-m += hellochardev.o
ccflags-y += -include $(kbuild-dir)/hellochardev.h

APPLICATION ?= tester.exe
HEADERS += hellochardev.h
CC ?= gcc

all: default

default: module $(APPLICATION)

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

$(APPLICATION): application.c
	$(CC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete
	rm -f *.exe

.PHONY: all default module clean
