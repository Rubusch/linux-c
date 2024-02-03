# USB Demo Module

For verification I used my PIC32MX470 board, configured to the below
USB vendor id as an HID device.  

NB: USB vendor and product id are not enough in recent
kernels. Without UDEV rule, the inplace kernel modules such as usbhid
will catch the device before our probe().  

In this setup, make sure the following is _NOT_ set (or at least as
module, then unload the module). Alternatively a udev rule has to be
in place additionally to allow for USB matching:  

- `CONFIG_USB_HID`


## Usage

```
$ make
$ sudo insmod hello_usb.ko
```
Copy the module to the target device. Load it. The USB device can be already connected, or connect it, the module will print the descriptor data to the logs.  
```
Jan 28 06:17:09 ctrl001 kernel: [20269.482711] usbcore: deregistering interface driver lothars_usb
Jan 28 06:17:09 ctrl001 kernel: [20269.482875] pen_disconnect() - XXX usb device disconnected XXX
Jan 28 06:24:46 ctrl001 kernel: [20726.581210] lothars_usb_driver 1-1.5:1.0: usb driver probed - vendor id: 0x4d8, product id: 0x3f
Jan 28 06:24:46 ctrl001 kernel: [20726.581255] USB INTERFACE DESCRIPTOR:
Jan 28 06:24:46 ctrl001 kernel: [20726.581271] -------------------------
Jan 28 06:24:46 ctrl001 kernel: [20726.581286] bLength: 0x9
Jan 28 06:24:46 ctrl001 kernel: [20726.581302] bDescriptorType: 0x4
Jan 28 06:24:46 ctrl001 kernel: [20726.581319] bInterfaceNumber: 0x0
Jan 28 06:24:46 ctrl001 kernel: [20726.581335] bAlternateSetting: 0x0
Jan 28 06:24:46 ctrl001 kernel: [20726.581351] bNumEndpoints: 0x2
Jan 28 06:24:46 ctrl001 kernel: [20726.581368] bInterfaceClass: 0x3
Jan 28 06:24:46 ctrl001 kernel: [20726.581383] bInterfaceSubClass: 0x0
Jan 28 06:24:46 ctrl001 kernel: [20726.581399] bInterfaceProtocol: 0x0
Jan 28 06:24:46 ctrl001 kernel: [20726.581416] iInterface: 0x0
Jan 28 06:24:46 ctrl001 kernel: [20726.581432]
Jan 28 06:24:46 ctrl001 kernel: [20726.581446] USB ENDPOINT DESCRIPTOR:
Jan 28 06:24:46 ctrl001 kernel: [20726.581461] -------------------------
Jan 28 06:24:46 ctrl001 kernel: [20726.581475] bLength: 0x7
Jan 28 06:24:46 ctrl001 kernel: [20726.581491] bDescriptorType: 0x5
Jan 28 06:24:46 ctrl001 kernel: [20726.581508] bEndpointAddress: 0x81
Jan 28 06:24:46 ctrl001 kernel: [20726.581524] bmAttributes: 0x3
Jan 28 06:24:46 ctrl001 kernel: [20726.581540] wMaxPacketSize: 0x40
Jan 28 06:24:46 ctrl001 kernel: [20726.581557] bInterval: 0x1
Jan 28 06:24:46 ctrl001 kernel: [20726.581573]
Jan 28 06:24:46 ctrl001 kernel: [20726.581587] USB ENDPOINT DESCRIPTOR:
Jan 28 06:24:46 ctrl001 kernel: [20726.581601] -------------------------
Jan 28 06:24:46 ctrl001 kernel: [20726.581624] bLength: 0x7
Jan 28 06:24:46 ctrl001 kernel: [20726.581640] bDescriptorType: 0x5
Jan 28 06:24:46 ctrl001 kernel: [20726.581656] bEndpointAddress: 0x1
Jan 28 06:24:46 ctrl001 kernel: [20726.581673] bmAttributes: 0x3
Jan 28 06:24:46 ctrl001 kernel: [20726.581689] wMaxPacketSize: 0x40
Jan 28 06:24:46 ctrl001 kernel: [20726.581705] bInterval: 0x1
Jan 28 06:24:46 ctrl001 kernel: [20726.581721]
Jan 28 06:24:46 ctrl001 kernel: [20726.583014] usbcore: registered new interface driver lothars_usb_driver
```

Unload the driver as follows.  
```
$ sudo rmmod hello_usb
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
