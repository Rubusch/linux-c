KERNELDIR ?= /usr/src/linux
obj-m += i2c_pcf8574.o
dtb-y += i2c_pcf8574_overlay.dtbo

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
