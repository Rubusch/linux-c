// SPDX-License-Identifier: GPL-2.0+
/*
  USB led demo for the PIC32MX470

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018
*/

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#define USBLED_VENDOR_ID 0x04D8
#define USBLED_PRODUCT_ID 0x003F

/*
  table of devices that work with this driver
*/
static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(USBLED_VENDOR_ID, USBLED_PRODUCT_ID) },
	{ },
};
MODULE_DEVICE_TABLE(usb, id_table);

struct usb_led {
	struct usb_device *usbdev;
	u8 led_number;
};

/*
 */
static ssize_t
led_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usb_led *led = usb_get_intfdata(intf);
	dev_info(dev, "%s() - called", __func__);

	return sprintf(buf, "%d\n", led->led-number);
}

static ssize_t
led_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usb_led *led = usb_get_intfdata(intf);
	struct device *dev = &intf->dev;
	u8 val;
	int error, ret;

	dev_info(dev, "%s() - called", __func__);

	// transform char array to u8 value
	error = kstrtou8(buf, 10, &val);
	if (error)
		return error;

	led->led_number = val;
	if (val == 1 || val == 2 || val == 3) {
		dev_info(dev, "%s() - led = %d\n",
			 __func__, led->led_number);
	} else {
		dev_info(dev, "%s() - unknown led %d\n",
			 __func__, led->led_number);
		ret = -EINVAL;
		return ret;
	}

	// toggle led
	ret = usb_bulk_msg(led->usbdev, usb_sndctrlpipe(led->usbdev, 1),
			   &led->led_number,
			   1, NULL, 0);
	if (ret) {
		ret = -EFAULT;
		return retval;
	}
	return count;
}
static DEVICE_ATTR_RW(led);

static int
led_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *usbdev = interface_to_usbdev(interface);
	struct usb_led *leddev = NULL;
	struct device *dev = &interface->dev;
	int ret = -ENOMEM;

	dev_info(dev, "%s() - called", __func__);

	leddev = kzalloc(sizeof(struct usb_led), GFP_KERNEL);
	if (!leddev) {
		dev_err(dev, "out of memory", __func__);
		ret = -ENOMEM;
		goto error;
	}

	dev->usbdev = usb_get_dev(usbdev);
	usb_set_intfdata(interface, leddev);

	ret = device_create_file(&interface->dev, &dev_attr_led);
	if (ret)
		goto error_create_file;

	return 0;

error_create_file:
	usb_put_dev(usbdev);
	usb_set_intfdata(interface, NULL);

error:
	kfree(dev);
	return ret;
}

static void
led_disconnect(struct usb_interface *interface)
{
	struct usb_led *leddev;
	struct device* dev = &interface->dev;

	leddev = usb_get_intfdata(interface);
	device_remove_file(&interface->dev, &dev_attr_led);
	usb_set_intfdata(interface, NULL);
	usb_put_dev(dev->usbdev);
	kfree(dev);

	dev_info(dev, "%s() - usb led is now disconnected", __func__);
}

static struct usb_driver led_driver = {
	.name = "usbled",
	.probe = led_probe,
	.disconnect = led_disconnect,
	.id_table = id_table,
};
module_usb_driver(led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("pic32 usb demo");
