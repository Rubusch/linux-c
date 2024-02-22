KERNELDIR ?= /usr/src/linux
obj-m += leddriver.o
dtb-y += leddriver_overlay.dtbo

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
