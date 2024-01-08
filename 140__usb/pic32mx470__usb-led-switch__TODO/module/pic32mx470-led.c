// SPDX-License-Identifier: GPL-2.0+
/*
  USB led demo for the PIC32MX470 with switch

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018
*/

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#define USBLED_VENDOR_ID 0x04D8
#define USBLED_PRODUCT_ID 0x003F

static void led_urb_out_callback(struct urb *urb);
static void led_urb_in_callback(struct urb *urb);

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
	struct usb_interface *intf;
	struct urb *interrupt_out_urb;
	struct urb *interrupt_in_urb;
	struct usb_endpoint_descriptor *interrupt_out_endpoint;
	struct usb_endpoint_descriptor *interrupt_in_endpoint;
	u8 irq_data;
	u8 led_number;
	u8 ibuf;
	int interrupt_out_interval;
	int ep_in;
	int ep_out;
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
	led->irq_data = val;

	if (val == 0) {
		dev_info(dev, "%s() - read status", __func__);
	} else if (val == 1 || val == 2 || val == 3) {
		dev_info(dev, "%s() - led = %d\n",
			 __func__, led->led_number);
	} else {
		dev_info(dev, "%s() - unknown led %d\n",
			 __func__, led->led_number);
		ret = -EINVAL;
		return ret;
	}

	// send out
	ret = usb_submit_urb(led->interrupt_out_urb, GFP_KERNEL);
	if (ret) {
		dev_err(dev, "%s() - interrupt_out_urb() failed",
			__func__);
		return ret;
	}
	return count;
}
static DEVICE_ATTR_RW(led);

static void
led_urb_out_callback(struct urb *urb)
{
	
}

static void
led_urb_in_callback(struct urb* urb)
{
	
}

static int
led_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *usbdev = interface_to_usbdev(interface);
	struct usb_host_interface *altsetting = interface->cur_altsetting;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_led *leddev = NULL;
	struct device *dev = &interface->dev;
	int ep, ep_in, ep_out;
	int ret = -ENOMEM, size, res;

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
	usb_free_urb(dev->interrupt_out_urb);
	usb_free_urb(dev->interrupt_in_urb);
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
	usb_free_urb(dev->interrupt_out_urb);
	usb_free_urb(dev->interrupt_in_urb);
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
