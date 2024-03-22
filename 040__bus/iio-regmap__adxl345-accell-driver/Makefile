KERNELDIR ?= /usr/src/linux/
obj-m += adxl345_core.o
obj-m += adxl345_i2c.o
obj-m += adxl345_spi.o

## dtbo-y seems to be broken (again...?)
DTSFILES += adxl345_spi_overlay.dts
DTSFILES += adxl345_i2c_overlay.dts
DTSFILES += adxl345_spi-3wire_overlay.dts
DTBOFILES := $(DTSFILES:.dts=.dtbo)

## issue: insmod: ERROR: could not insert module adxl345_spi.ko: Invalid module format
## fix: append EXTRAVERSION to $(uname -r), remove version before '-'
EXTRAVERSION = "-lothar01"

%.dtbo: %.dts
	$(DTC) dtc -O dtb -o $@ $<

all: $(DTBOFILES)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
