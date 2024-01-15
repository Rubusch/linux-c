# PIC32MX470: led demos

A Linux USB driver that is controlled from userspace by using the I2C
tools for Linux.  

The PIC32 driver model is recursive. This involves an I2C device
through a PCI board that integrates a USB to I2C converter. The main
steps in click-programming the PIC32 setup will create this recursive
driver model.  

- PIC32: Develop a PCI device driver that will reate a USB
  adapter. The PCI device driver is the parent of the USB adapter
  driver.
- PIC32: Develop a USB device driver that will send USB data to the
  USB adapter driver through the USB core. This USB devie driver will
  also create an I2C adapter driver. The USB device driver is the
  parent of the I2C adapter driver.
- PIC32: Create an I2C device driver that will send data to the I2C
  adapter driver through the I2C core and will create a
  `file_operations` structure to define the driver functions that are
  called when the Linux user space reads and writes to character
  devices.

Kernel Config: make sure the following is set:  
- `CONFIG_HID_SUPPORT`
- `CONFIG_HID_GENERIC`

## Hardware: Microchip Curiosity PIC32MX470 (PIC32MX470512H)

#### PIC32 Board
https://www.microchip.com/DevelopmentTools/ProductDetails/dm320103

Connection:  
Connect the USB Micro-B port (J12) of the PIC32MX470 Curiosity
Development Board to the J19 USB-B type C connector. In case this will
need a USB type-C male to micro-B male cable.  


#### DC749A Board
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/dc749a.html

Connection: connect the following MikroBUS 1 pins  
- SDA -> pin 7 (SDA) of the DC749a J1
- SCL -> pin 4 (SCL)
- DC749a pin 6 (ENRGB/S) -> DC749a Vin J2 pin
- 3.3V -> J20 DVCC pin
Do *not* conect GND between the two boards!  

NB: Verify on the PIC32MX470 Dev Board that the series resistors
mounted on the SCL and SDA lines of the **MikroBUS 1 socket J5** are
set to **0 Ohms**, if not replace them with 0 Ohms. Alternatively take
the signals directly from the J6 connector.  

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
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 629ff, p. 648  
