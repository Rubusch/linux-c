KERNELDIR ?= /usr/src/linux
obj-m += input_demo.o

## dtbo-y seems to be broken (again...?)
#dtbo-y += input_demo_overlay.dtbo
DTSFILES += input_demo_overlay.dts
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
