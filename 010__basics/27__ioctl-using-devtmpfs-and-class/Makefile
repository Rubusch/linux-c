KERNELDIR ?= /usr/src/linux
obj-m += happy_ioctl.o

## application
SOURCE = ioctl_test.c
OBJECTS = $(SOURCE:.c=.o)
APPLICATION = $(SOURCE:.c=.elf)
.PREFIXES = .c .o

CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall

all: default

default: module $(APPLICATION)

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

%.c%.o:
	$(CC) $(CFLAGS) -c $<

$(APPLICATION): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f $(APPLICATION) $(OBJECTS) *~

.PHONY: all default module clean
