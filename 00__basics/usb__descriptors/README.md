# USB Demo Module

The source was tested compiled and running on 5.4.75.  


## Usage

First, adjust the vendorid and deviceid in the source to the specific device.  

```
$ make
$ sudo insmod hello_usb.ko

$ sudo rmmod hello_usb


$ sudo dmesg | tail
    Feb  3 15:33:54 debian kernel: lothars_usb_driver 1-4.3.2:1.4: usb driver probed - vendor id: 0x1366, product id: 0x1051
    Feb  3 15:33:54 debian kernel: USB INTERFACE DESCRIPTOR:
    Feb  3 15:33:54 debian kernel: -------------------------
    Feb  3 15:33:54 debian kernel: bLength: 0x9
    Feb  3 15:33:54 debian kernel: bDescriptorType: 0x4
    Feb  3 15:33:54 debian kernel: bInterfaceNumber: 0x4
    Feb  3 15:33:54 debian kernel: bAlternateSetting: 0x0
    Feb  3 15:33:54 debian kernel: bNumEndpoints: 0x2
    Feb  3 15:33:54 debian kernel: bInterfaceClass: 0xff
    Feb  3 15:33:54 debian kernel: bInterfaceSubClass: 0xff
    Feb  3 15:33:54 debian kernel: bInterfaceProtocol: 0xff
    Feb  3 15:33:54 debian kernel: iInterface: 0x9
    Feb  3 15:33:54 debian kernel: 
    Feb  3 15:33:54 debian kernel: USB ENDPOINT DESCRIPTOR:
    Feb  3 15:33:54 debian kernel: -------------------------
    Feb  3 15:33:54 debian kernel: bLength: 0x7
    Feb  3 15:33:54 debian kernel: bDescriptorType: 0x5
    Feb  3 15:33:54 debian kernel: bEndpointAddress: 0x89
    Feb  3 15:33:54 debian kernel: bmAttributes: 0x2
    Feb  3 15:33:54 debian kernel: wMaxPacketSize: 0x40
    Feb  3 15:33:54 debian kernel: bInterval: 0x1
    Feb  3 15:33:54 debian kernel: 
    Feb  3 15:33:54 debian kernel: USB ENDPOINT DESCRIPTOR:
    Feb  3 15:33:54 debian kernel: -------------------------
    Feb  3 15:33:54 debian kernel: bLength: 0x7
    Feb  3 15:33:54 debian kernel: bDescriptorType: 0x5
    Feb  3 15:33:54 debian kernel: bEndpointAddress: 0x6
    Feb  3 15:33:54 debian kernel: bmAttributes: 0x2
    Feb  3 15:33:54 debian kernel: wMaxPacketSize: 0x40
    Feb  3 15:33:54 debian kernel: bInterval: 0x1
    Feb  3 15:33:54 debian kernel: 
```

Now plug the specified USB device and have a look into the ``/var/log/syslog``.  

```
$ dmesg | tail
```

Unload the driver as follows.  

```
$ sudo rmmod usb
```


## Notes

**USB core** - USB core is a codebase consisting of routines and structures available to HCDs (Host Controller Driver) and USB drivers.  

**USB Driver** - This is the USB driver which we are going to write for the USB devices.  


The USB device contains a number of descriptors that help to define what the device is capable of. We will discuss about the below descriptors.  

 * Device descriptor: USB devices can only have one device descriptor. The device descriptor includes information such as what USB revision the device complies with, the Product and Vendor IDs used to load the appropriate drivers, and the number of possible configuration descriptors the device can have.  

 * Configuration descriptor: The configuration descriptor specifies values such as the amount of power this particular configuration uses if the device is self or bus-powered and the number of interfaces it has. When a device is enumerated, the host reads the device descriptors and can make a decision of which configuration to enable. A device can have more than one configuration, though it can enable only one configuration at a time.  

 * Interface descriptor: A device can have one or more interfaces. Each interface descriptor can have a number of endpoints and represents a functional unit belonging to a particular class. For example you could have a multi-function fax/scanner/printer device. Interface descriptor one could describe the endpoints of the fax function, Interface descriptor two the scanner function, and Interface descriptor three the printer function. Unlike the configuration descriptor, there is no limitation as to having only one interface enabled at a time. A device could have 1 or many interface descriptors enabled at once.  

 * Endpoint descriptor: Each endpoint descriptor is used to specify the type of transfer, direction, polling interval, and maximum packet size for each endpoint. In other words, each endpoint is a source or sink of data.  


There are four different ways to transfer data on a USB bus. These data transfer types are set in the endpoint descriptor.

 * Control Transfers: Control transfers are typically used for command and status operations. They are essential to set up a USB device with all enumeration functions being performed using control transfers. This is a bi-directional transfer which uses both an IN and an OUT endpoint.  

 * Interrupt Transfers: Interrupt transfers have nothing to do with interrupts. The name is chosen because they are used for the sort of purpose where an interrupt would have been used in earlier connection types. Interrupt transfers are regularly scheduled IN or OUT transactions, although the IN direction is the more common usage.  

 * Bulk Transfers: Bulk transfers are designed to transfer large amounts of data with error-free delivery, but with no guarantee of bandwidth. The host will schedule bulk transfers after the other transfer types have been allocated. If an OUT endpoint is defined as using Bulk transfers, then the host will transfer data to it using OUT transactions. If an IN endpoint is defined as using Bulk transfers, then the host will transfer data from it using IN transactions.  

 * Isochronous Transfers: Isochronous transfers have a guaranteed bandwidth, but error-free delivery is not guaranteed. The main purpose of isochronous transfers is applications such as audio data transfer, where it is important to maintain the data flow, but not so important if some data gets missed or corrupted. An isochronous transfer uses either an IN transaction or an OUT transaction depending on the type of endpoint.  

[taken from www.embetronicx.com]  



---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * www.embetronicx.com (2020/2021)
 * https://kernel.readthedocs.io/en/sphinx-samples/writing_usb_driver.html
