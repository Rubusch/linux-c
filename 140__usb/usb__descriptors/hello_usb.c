/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/usb.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

// usb
static int lothars_usb_probe(struct usb_interface *,
			     const struct usb_device_id *);
static void lothars_usb_disconnect(struct usb_interface *);

/*
  globals
*/

// e.g. using my SONY usbstick
#define USB_DRIVER_NAME "lothars_usb_driver"

// XXX adjust here XXX
#define USB_VENDORID 0x346d
#define USB_PRODUCTID 0x5678


#define PRINT_USB_IFACE_DESCRIPTOR(i)                                          \
	printk(KERN_INFO "USB INTERFACE DESCRIPTOR:\n");                       \
	printk(KERN_INFO "-------------------------\n");                       \
	printk(KERN_INFO "bLength: 0x%x\n", i.bLength);                        \
	printk(KERN_INFO "bDescriptorType: 0x%x\n", i.bDescriptorType);        \
	printk(KERN_INFO "bInterfaceNumber: 0x%x\n", i.bInterfaceNumber);      \
	printk(KERN_INFO "bAlternateSetting: 0x%x\n", i.bAlternateSetting);    \
	printk(KERN_INFO "bNumEndpoints: 0x%x\n", i.bNumEndpoints);            \
	printk(KERN_INFO "bInterfaceClass: 0x%x\n", i.bInterfaceClass);        \
	printk(KERN_INFO "bInterfaceSubClass: 0x%x\n", i.bInterfaceSubClass);  \
	printk(KERN_INFO "bInterfaceProtocol: 0x%x\n", i.bInterfaceProtocol);  \
	printk(KERN_INFO "iInterface: 0x%x\n", i.iInterface);                  \
	printk(KERN_INFO "\n");

#define PRINT_USB_ENDPOINT_DESCRIPTOR(e)                                       \
	printk(KERN_INFO "USB ENDPOINT DESCRIPTOR:\n");                        \
	printk(KERN_INFO "-------------------------\n");                       \
	printk(KERN_INFO "bLength: 0x%x\n", e.bLength);                        \
	printk(KERN_INFO "bDescriptorType: 0x%x\n", e.bDescriptorType);        \
	printk(KERN_INFO "bEndpointAddress: 0x%x\n", e.bEndpointAddress);      \
	printk(KERN_INFO "bmAttributes: 0x%x\n", e.bmAttributes);              \
	printk(KERN_INFO "wMaxPacketSize: 0x%x\n", e.wMaxPacketSize);          \
	printk(KERN_INFO "bInterval: 0x%x\n", e.bInterval);                    \
	printk(KERN_INFO "\n");

// usb
const struct usb_device_id lothars_usb_table[] = {
	{ USB_DEVICE(USB_VENDORID, USB_PRODUCTID) },
	{} /* terminating endtry */
};
MODULE_DEVICE_TABLE(hello_usb, lothars_usb_table); // enables usb hotplug system, to load the driver automatically

static struct usb_driver lothars_usb_driver = {
	.name = USB_DRIVER_NAME,
	.id_table = lothars_usb_table,
	.probe = lothars_usb_probe,
	.disconnect = lothars_usb_disconnect,
};

/*
  implementation
*/

static int lothars_usb_probe(struct usb_interface *interface,
			     const struct usb_device_id *id)
{
	unsigned int idx;
	unsigned int nendpoints;
	struct usb_host_interface *iface_desc = interface->cur_altsetting;

	dev_info(&interface->dev,
		 "usb driver probed - vendor id: 0x%02x, product id: 0x%02x\n",
		 id->idVendor, id->idProduct);
	nendpoints = iface_desc->desc.bNumEndpoints;
	PRINT_USB_IFACE_DESCRIPTOR(iface_desc->desc);

	for (idx = 0; idx < nendpoints; ++idx) {
		PRINT_USB_ENDPOINT_DESCRIPTOR(iface_desc->endpoint[idx].desc);
	}

	return 0;
}

static void lothars_usb_disconnect(struct usb_interface *interface)
{
	dev_info(&interface->dev, "usb driver disconnected\n");
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	int res;
	res = usb_register(&lothars_usb_driver);
	if (0 > res) {
		printk(KERN_ERR "usb_register() failed\n");
		return -ENOMEM;
	}
	return 0;
}

static void __exit mod_exit(void)
{
	usb_deregister(&lothars_usb_driver);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates a Hello USB Module!");
