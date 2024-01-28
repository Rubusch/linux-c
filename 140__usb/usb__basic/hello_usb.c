/*
  Basic USB driver matching Demo

  For verification I used my PIC32MX470 board, configured to the
  below USB vendor id as an HID device.

  NB: USB vendor and product id are not enough in recent
  kernels. Without UDEV rule, the inplace kernel modules such as
  usbhid will catch the device before our probe().

  In this setup, make sure the following is _NOT_ set (or at least as
  module, then unload the module). Alternatively a udev rule has to be
  in place additionally to allow for USB matching:
   - `CONFIG_USB_HID`

  Load the module, then connec the USB cable to the PIC32MX - watch the log:
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

    GPLv2 || MIT licensed, author: Lothar Rubusch
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

/*
  forwards
*/

static int pen_probe(struct usb_interface *, const struct usb_device_id *);
static void pen_disconnect(struct usb_interface *);

/*
  globals
*/

#define USB_DEVICE_NAME "lothars_usb"

#define USB_VENDOR_ID   0x04D8
#define USB_PRODUCT_ID  0x003f

static struct usb_device_id pen_table[] = {
	{ USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
	{} /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, pen_table);

static struct usb_driver pen_driver = {
	.name = USB_DEVICE_NAME,
	.id_table = pen_table,
	.probe = pen_probe,
	.disconnect = pen_disconnect,
};

/*
  implementations
*/

static int pen_probe(struct usb_interface *interface,
		     const struct usb_device_id *id)
{
	printk(KERN_INFO "%s() - XXX probed usb device (%04X:%04X) XXX\n",
	       __func__, id->idVendor, id->idProduct);
	return 0;
}

static void pen_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "%s() - XXX usb device disconnected XXX\n", __func__);
}

/*
  init / exit
*/

module_usb_driver(pen_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates the basic usb driver skeleton.");
