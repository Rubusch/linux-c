KERNELDIR ?= /usr/src/linux/
obj-m += adxl345_core.o
obj-m += adxl345_i2c.o
obj-m += adxl345_spi.o

## dtbo-y seems to be broken (again...)
DTFILE = input_demo_overlay

## issue: insmod: ERROR: could not insert module adxl345_spi.ko: Invalid module format
## fix: append EXTRAVERSION to $(uname -r), remove version before '-'
EXTRAVERSION = "-lothar01"

all: dt
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

dt:
	$(DTC) dtc -O dtb -o $(DTFILE).dtbo $(DTFILE).dts

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all dt clean
