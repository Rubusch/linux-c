# USB Demo Module

Basic USB driver matching Demo

For verification I used my PIC32MX470 board, configured to the below
USB vendor id as an HID device.  

NB: USB vendor and product id are not enough in recent
kernels. Without UDEV rule, the inplace kernel modules such as usbhid
will catch the device before our probe().  

In this setup, make sure the following is _NOT_ set (or at least as
module, then unload the module). Alternatively a udev rule has to be
in place additionally to allow for USB matching:  

- `CONFIG_USB_HID`


The source was tested compiled and running on v6.3 on RPI.  

## Usage

First, adjust the vendorid and deviceid in the source to the specific device.  

```
$ make
$ sudo insmod hello_usb.ko
```

Load the module, then connec the USB cable to the PIC32MX - watch the log:
```
  ...
    Jan 28 06:07:09 ctrl001 kernel: [19669.591966] usb 1-1.5: new full-speed USB device number 11 using dwc_otg
    Jan 28 06:07:09 ctrl001 kernel: [19669.695039] usb 1-1.5: New USB device found, idVendor=04d8, idProduct=003f, bcdDevice= 1.00
    Jan 28 06:07:09 ctrl001 kernel: [19669.695060] usb 1-1.5: New USB device strings: Mfr=1, Product=2, SerialNumber=0
    Jan 28 06:07:09 ctrl001 kernel: [19669.695074] usb 1-1.5: Product: Simple HID Device Demo
    Jan 28 06:07:09 ctrl001 kernel: [19669.695087] usb 1-1.5: Manufacturer: Lothars USB HID Demo
    Jan 28 06:07:09 ctrl001 kernel: [19669.696226] pen_probe() - XXX probed usb device (04D8:003F) XXX
    Jan 28 06:07:10 ctrl001 mtp-probe: checking bus 1, device 11: "/sys/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.5"
    Jan 28 06:07:10 ctrl001 mtp-probe: bus: 1, device: 11 was not an MTP device
    Jan 28 06:07:10 ctrl001 mtp-probe: checking bus 1, device 11: "/sys/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.5"
    Jan 28 06:07:10 ctrl001 mtp-probe: bus: 1, device: 11 was not an MTP device
```

Unload the driver as follows.  

```
$ sudo rmmod usb
```

## Notes

This is a basic instance of the usb driver skeleton. Find some more elaborated usb device driver example e.g. here:  
https://sysplay.github.io/books/LinuxDrivers/book/Content/Part11.html

---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * https://kernel.readthedocs.io/en/sphinx-samples/writing_usb_driver.html
 * https://sysplay.github.io/books/LinuxDrivers/book/Content/Part11.html
