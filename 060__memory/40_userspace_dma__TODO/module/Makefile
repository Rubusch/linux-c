KERNELDIR ?= /usr/src/linux
obj-m += dma_demo.o
dtb-y += dma_demo_overlay.dtbo

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
