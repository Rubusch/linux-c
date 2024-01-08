KERNELDIR ?= /usr/src/linux
obj-m += adxl345_core.o adxl345_i2c.o adxl345_spi.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
