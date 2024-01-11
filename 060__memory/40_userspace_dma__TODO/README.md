# DMA from user space

TODO       

# Build

## Devicetree

Copy it to the specified location in the linux sources (6.3), then build it  
```
$ cd linux
$ cp -arf <SOURCES>/devicetree/arch ./

$ make dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module

Having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace

Having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./userspace
$ make
```
Copy the `*.elf` file(s) over to the target  

## Usage

```
pi@ctrl001:~ $ ls -l
    total 36
    -rw-r--r-- 1 pi pi 17280 Dec  3 19:01 dma_demo.ko
    -rwxr-xr-x 1 pi pi 12528 Dec  3 19:01 sdma.elf
pi@ctrl001:~ $ sudo su
# insmod ./dma_demo.ko
# ./sdma.elf
    enter phrase:
    7
# rmmod dma_demo.ko
```

Following the logs   
```
TODO   
```

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 409  
