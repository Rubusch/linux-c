DTOVERLAY = i2c_pcf8574_overlay
KERNELDIR ?= /usr/src/linux
obj-m += i2c_pcf8574.o

all: dt
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

dt: $(DTOVERLAY).dts
	dtc -@ -I dts -O dtb -o $(DTOVERLAY).dtbo $(DTOVERLAY).dts

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all dt clean
