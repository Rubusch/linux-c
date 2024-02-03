# PIC32MX470: USB HID demo encapsuling a I2C Device

The setup will connect a DC749a multi led board over the mikorbus I
connectors, for the corresponding linux device driver.  

## Setup

The setup is build on a similar base as [the HID demo](../pic32mx470__01-hid-demo).  

Main modification is using the i2c driver and i2c library, link them as here in the image.  
![MCC setup](./pics/pic32mx470.jpg)  

Configure the i2c as synchronous driver, leave the rest as is.  

## Modifications

Enable the i2c setup with the pin configurator, for the `MikroBus I` interface.  

![setup0010](./pics/0010.jpg)  
![setup0020](./pics/0020.jpg)  
![setup0030](./pics/0030.jpg)  
![setup0040](./pics/0040.jpg)  
![setup0050](./pics/0050.jpg)  

