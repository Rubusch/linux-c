## Makefile
##
## for a kernel module (2.6.18) for multiple files
##

obj-m += hellomultiple.o
multiple-objs := hellostart.o hellostop.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


