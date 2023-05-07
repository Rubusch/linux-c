KERNELDIR ?= /lib/modules/$(shell uname -r)/build
obj-m += module_a.o
obj-m += module_b.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
