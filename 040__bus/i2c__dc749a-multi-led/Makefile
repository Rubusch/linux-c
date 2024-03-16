KERNELDIR ?= /usr/src/linux
obj-m += i2c_ltc3206euf.o
dtbo-y += i2c_ltc3206euf_overlay.dtbo

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all clean
