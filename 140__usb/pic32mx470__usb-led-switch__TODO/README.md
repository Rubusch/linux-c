# PIC32MX470: usb led demo with switch

The communication between the host and the devie here is done asynchronously by using USB request blocks (urbs).  

## Hardware: Microchip Curiosity PIC32MX470 (PIC32MX470512H)

PIC32 Board: https://www.microchip.com/DevelopmentTools/ProductDetails/dm320103


## Software: MPLAB X IDE

MPLAB X IDE: https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs

```
$ sudo apt install -y libusb-1.0-0

$ tar xf ./MPLABX-v6.15-linux-installer.tar
$ sudo ./MPLABX-v6.15-linux-installer.sh
-> only select MPLAB X IDE, and
-> PIC32 support
```
TODO       

# Build

## Devicetree

Copy it to the specified location in the linux sources, then build it  
```
$ cd linux
$ cp -arf <SOURCES>/devicetree/arch ./
$ find . -name \*.dtb -delete
$ make dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module

Having crossbuild-essentials-arm64 installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module__ltc2607-dual-dac
$ make
```
Copy the module over to the target  

## Usage

```
TODO       
```

Logs   
```
TODO      
```

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 594ff, p. 616  
