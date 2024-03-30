KERNELDIR ?= /usr/src/linux
obj-m += hello_gpio.o

## dtbo-y seems to be broken (again...?)
#dtbo-y += gpio-led__rpi4.dtbo
DTSFILES += gpio-led__rpi4.dts
DTBOFILES := $(DTSFILES:.dts=.dtbo)
DTC ?= dtc

%.dtbo: %.dts
	$(DTC) -i $(KERNELDIR)/include -O dtb -o $@ $<

all: $(DTBOFILES)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
