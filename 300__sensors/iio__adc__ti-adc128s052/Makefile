KERNELDIR ?= /usr/src/linux/
obj-m += ti-adc128s052.o

## dtbo-y seems to be broken (again...?)
DTSFILES += adc121s_spi_overlay.dts
DTBOFILES := $(DTSFILES:.dts=.dtbo)

CFLAGS += -Wall
#CFLAGS += -pedantic
## issue: insmod: ERROR: could not insert module adxl345_spi.ko: Invalid module format
## fix: append EXTRAVERSION to $(uname -r), remove version before '-'
#EXTRAVERSION = "-lothar02"

%.dtbo: %.dts
	$(DTC) dtc -O dtb -o $@ $<

all: $(DTBOFILES)
	$(MAKE) $(CFLAGS) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
