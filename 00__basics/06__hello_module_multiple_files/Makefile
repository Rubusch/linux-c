KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#RM = rm -f

all: module

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete

.PHONY: all module clean
