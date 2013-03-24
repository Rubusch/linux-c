## Makefile for kernelmodule, kernel 2.6.18
## needs kernel headers
##
## in case needs to be done:
## cd /usr/src/linux
## make-kpkg clean && make-kpkg oldconfig 
## make scripts/mod/modpost
##

#obj-m += other module.o
obj-m += hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
