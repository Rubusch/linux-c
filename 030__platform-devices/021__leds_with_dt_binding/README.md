# LED Module with Devicetree Binding (open firmware)

Shows a mere copy for personal notes of A. Rios LED driver for the 5.4 Kernel (originally), tested on 6.1.  

#### Notes on `container_of()`

```
    container_of(ptr, type, member)
```

* ptr – the pointer to an instance
* type – the type of the container struct
* member – the name of the member within the struct

The `container_of` macro then returns the address of the member for the specified instance.  


[further details on IOMUX on NPX's iMX7D p125ff -> references]  

## Linux

```
$ cd linux
$ make dtbs
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the /boot/bcm2710-rpi-3-b.dtb. In case make a safety backup first. A copy of a modified DTS is attached.  

## Module
Should compile cross - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace
Easiest is to copy the folder `userspace`  to the target  
```
rpi$ cd ./userspace
rpi$ make
```

## Usage
On the target perform the following to verify the functionality  
```
pi@raspberrypi:~$ sudo insmod chardev.ko

pi@raspberrypi:~$ sudo find /sys -name "*lothar*"
    /sys/bus/platform/drivers/lotharskeys
    /sys/module/chardev/drivers/platform:lotharskeys

pi@raspberrypi:~$ rmmod chardev
```
The module could be load, the devicetree binding would match.  

## Verified
* Verified against a RPI3 w/ aarch64

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 161
