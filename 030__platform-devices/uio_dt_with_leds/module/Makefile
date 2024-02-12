DTOVERLAY = uioled_overlay
KERNELDIR ?= /usr/src/linux
obj-m += uioled.o

all: dt
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

dt: $(DTOVERLAY).dts
	dtc -@ -I dts -O dtb -o $(DTOVERLAY).dtbo $(DTOVERLAY).dts

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all dt clean
