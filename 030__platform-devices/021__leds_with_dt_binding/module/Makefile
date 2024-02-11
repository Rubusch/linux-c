DTFRAG = leds_with_dt_binding_overlay
KERNELDIR ?= /usr/src/linux
obj-m += leddriver.o

all: dt
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

dt: $(DTFRAG).dts
	dtc -@ -I dts -O dtb -o $(DTFRAG).dtbo $(DTFRAG).dts

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all dt clean
