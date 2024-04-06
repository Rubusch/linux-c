KERNELDIR ?= /usr/src/linux
obj-m += irq_click.o

## dtbo-y seems to be broken (again...?)
#dtbo-y += irq_click_overlay.dtbo
DTSFILES += irq_click_overlay.dts
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
