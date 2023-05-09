## out of soure build
KERNELDIR ?= $(HOME)/workspace/linux
ARCH ?= aarch64
CROSS_COMPILE ?= aarch64-linux-gnu-

obj-m += print_my_types.o

all:
	make -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
		M=$(PWD) modules

clean:
	make -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
		M=$(PWD) clean

.PHONY: all clean
