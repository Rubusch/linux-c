KERNELDIR ?= /usr/src/linux
obj-m += iio-ltc2422-adc.o

## dtbo-y seems to be broken (again...?)
#dtbo-y += iio-ltc2422-adc_overlay.dtbo
DTSFILES += iio-ltc2422-adc_overlay.dts
DTBOFILES := $(DTSFILES:.dts=.dtbo)
DTC ?= dtc

%.dtbo: %.dts
	$(DTC) -O dtb -o $@ $<

all: $(DTBOFILES)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
