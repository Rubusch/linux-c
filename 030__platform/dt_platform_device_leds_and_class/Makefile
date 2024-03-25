KERNELDIR ?= /usr/src/linux
obj-m += leddriver.o

## dtbo-y seems to be broken (again...?)
#dtbo-y += leddriver_overlay.dtbo
DTSFILES += leddriver_overlay.dts
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
