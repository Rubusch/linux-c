KERNELDIR ?= /usr/src/linux
obj-m += adxl345_core.o adxl345_i2c.o adxl345_spi.o
dtb-y += input_demo_overlay.dtbo
## issue: insmod: ERROR: could not insert module adxl345_spi.ko: Invalid module format
## fix: append EXTRAVERSION to $(uname -r), remove version before '-'
EXTRAVERSION = "-lothar09+"

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
