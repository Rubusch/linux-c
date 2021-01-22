KERNELDIR ?= /lib/modules/$(shell uname -r)/build
obj-m += hellochardev.o

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
