## Makefile, kernel 2.6.18
##
## kernel module
##

obj-m += hellochardev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
