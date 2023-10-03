/*
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

#define USB_VENDOR_ID 0x346d
#define USB_PRODUCT_ID 0x5678

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
