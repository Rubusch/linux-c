# PIC32MX470: usb led demo with switch

Kernel Config: make sure the following is set:  
- `CONFIG_HID_SUPPORT`
- `CONFIG_HID_GENERIC`

The communication between the host and the devie here is done asynchronously by using USB request blocks (urbs).  

## Hardware: Microchip Curiosity PIC32MX470 (PIC32MX470512H)

PIC32 Board:  
https://www.microchip.com/DevelopmentTools/ProductDetails/dm320103

Connection:  
Connect the USB Micro-B port (J12) of the PIC32MX470 Curiosity
Development Board to the J19 USB-B type C connector. In case this will
need a USB type-C male to micro-B male cable.  

TODO    


## Software: MPLAB X IDE

MPLAB X IDE: https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs

```
$ sudo apt install -y libusb-1.0-0

$ tar xf ./MPLABX-v6.15-linux-installer.tar
$ sudo ./MPLABX-v6.15-linux-installer.sh
-> only select MPLAB X IDE, and
-> PIC32 support
```

## Setup the HID Application in the MPLAB IDE

TODO       

# Build

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
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 617ff, p. 628  
