KERNELDIR ?= /usr/src/linux
obj-m += iio-ltc2607-dac.o
dtbo-y += iio-ltc2607-dac_overlay.dtbo

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
