# Keyless Class Module

The demo creates several led devices under the "Keyled" class and also several sysfs entries under each led device. Each led device can be controlled by writing from user space to the sysfs entries under each led device. There is no cdev structure initialized for each device (adding as an argument a file_operations structure) and thus there won't be a led device under /dev which could be controlled by a syscall.  

Control the led devices by writing to the sysfs entries under `/sys/class/Keyled/<led device>/ directory.  

## Hardware: MikroE Button-R-Click

https://www.mikroe.com/button-r-click


# Build

## Devicetree

copy it to the specified location in the linux sources (6.3), then build it  
```
$ cd linux
$ cp -arf <SOURCES>/devicetree/arch ./

$ make dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module

Should crosscompile - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Usage

TODO           


## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 313  
