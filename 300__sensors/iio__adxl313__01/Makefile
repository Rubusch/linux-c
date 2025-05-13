KERNELDIR ?= /usr/src/linux/
obj-m += adxl313_core.o
obj-m += adxl313_i2c.o
obj-m += adxl313_spi.o

## dtbo-y seems to be broken (again...?)
DTSFILES += adxl313_spi_overlay.dts
DTSFILES += adxl313_i2c_overlay.dts
DTBOFILES := $(DTSFILES:.dts=.dtbo)

CFLAGS += -Wall

%.dtbo: %.dts
	$(DTC) dtc -O dtb -o $@ $<

all: $(DTBOFILES)
	$(MAKE) $(CFLAGS) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
