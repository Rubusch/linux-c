// SPDX-License-Identifier: GPL-2.0+
/*
  USB led demo for the PIC32MX470 with switch

  The communication between the host and the device is done
  asynchronously by using USB Request Blocks (URBs)

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
  The table of devices that work with this driver
 */
static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(USBLED_VENDOR_ID, USBLED_PRODUCT_ID) },
	{ },
};
MODULE_DEVICE_TABLE(usb, id_table);

/*
  Create a private structure that will store the driver's data.
 */
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

	return sprintf(buf, "%d\n", led->led_number);
}

static ssize_t
led_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usb_led *led = usb_get_intfdata(intf);
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
	struct usb_led *led = urb->context;
	struct device *dev = &led->usbdev->dev;

	dev_info(dev, "%s() - called", __func__);

	// sync/async unlink faults aren't errors
	if (urb->status) {
		if (!(urb->status == -ENOENT ||
		    urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN)) {
			dev_err(dev, "%s() - nonzero write status received: %d",
				__func__, urb->status);
		}
	}
}

static void
led_urb_in_callback(struct urb* urb)
{
	struct usb_led *led = urb->context;
	struct device *dev = &led->usbdev->dev;
	int ret;

	if (urb->status) {
		if (!(urb->status == -ENOENT ||
		      urb->status == -ECONNRESET ||
		      urb->status == -ESHUTDOWN)) {
			dev_err(dev, "%s() - nonzero write status received: %d",
				__func__, urb->status);
		}
	}

	if (0x00 == led->ibuf) {
		pr_info("switch is ON.");
	} else if (0x01 == led->ibuf) {
		pr_info("switch is OFF.");
	} else {
		pr_info("bad value received.");
	}

	ret = usb_submit_urb(led->interrupt_in_urb, GFP_KERNEL);
	if (ret) {
		dev_err(dev, "%s() - failed to submit interrupt_in_urb %d",
			__func__, ret);
	}
}

static int
led_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	// obtain the struct usb_device from the usb_interface
	struct usb_device *usbdev = interface_to_usbdev(interface);
	struct usb_host_interface *altsetting = interface->cur_altsetting;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_led *led = NULL;
	struct device *dev = &interface->dev;
	int ep, ep_in, ep_out;
	int ret = -ENOMEM, size, res;

	dev_info(dev, "%s() - called", __func__);
	res = usb_find_last_int_out_endpoint(altsetting, &endpoint);
	if (res) {
		dev_info(dev, "%s() - no endpoint found", __func__);
		return res;
	}

	ep = usb_endpoint_num(endpoint); // value from 0 to 15
	dev_info(dev, "%s() - usb_endpoint_num() = %d [1] - endpoint number",
		 __func__, ep);

	size = usb_endpoint_maxp(endpoint);
	dev_info(dev, "%s() - usb_endpoint_maxp() = %ld [?] - endpoint size",
		 __func__, (long) size);

	// validate endpoint and size
	if (0 >= size) {
		dev_info(dev, "%s() - invalid size (%d)",
			 __func__, size);
		return -ENODEV;
	}

	ep_in = altsetting->endpoint[0].desc.bEndpointAddress;
	ep_out = altsetting->endpoint[1].desc.bEndpointAddress;

	dev_info(dev, "%s() - endpoint IN address is 0x%02x",
		 __func__, ep_in);
	dev_info(dev, "%s() - endpoint OUT address is 0x%02x",
		 __func__, ep_out);

	// allocate the private data structure
	led = kzalloc(sizeof(*led), GFP_KERNEL);
	if (!led) {
		dev_err(dev, "%s() - out of memory", __func__);
		ret = -ENOMEM;
		goto err;
	}

	// init usb_led instance
	led->ep_in = ep_in;
	led->ep_out = ep_out;

	led->usbdev = usb_get_dev(usbdev);
	led->intf = interface;

	// alloate int_out_urb instance
	led->interrupt_out_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!led->interrupt_out_urb) {
		goto err;
	}

	// initialize int_out_urb
	usb_fill_int_urb(led->interrupt_out_urb,
			 led->usbdev,
			 usb_sndintpipe(led->usbdev, ep_out),
			 (void*) &led->irq_data,
			 1,
			 led_urb_out_callback, led, 1);

	// allocate int_in_urb instance
	led->interrupt_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!led->interrupt_in_urb) {
		goto err;
	}

	// initialize int_in_urb
	usb_fill_int_urb(led->interrupt_in_urb,
			 led->usbdev,
			 usb_rcvintpipe(led->usbdev, ep_in),
			 (void*) &led->ibuf,
			 1,
			 led_urb_in_callback, led, 1);

	usb_set_intfdata(interface, led);

	// create a led sysfs entry to interact with user space
	ret = device_create_file(&interface->dev, &dev_attr_led);
	if (ret)
		goto err_create_file;

	ret = usb_submit_urb(led->interrupt_in_urb, GFP_KERNEL);
	if (ret) {
		dev_err(dev, "%s() - failed to submit interrupt_in_urb %d",
			__func__, ret);
		device_remove_file(dev, &dev_attr_led);
		goto err_create_file;
	}

	dev_info(dev, "%s() - interrupt_in_urb submitted", __func__);

	return 0;

err_create_file:
	usb_free_urb(led->interrupt_out_urb);
	usb_free_urb(led->interrupt_in_urb);
	usb_put_dev(usbdev);
	usb_set_intfdata(interface, NULL);

err:
	kfree(dev);
	return ret;
}

static void
led_disconnect(struct usb_interface *interface)
{
	struct usb_led *led = usb_get_intfdata(interface);
	struct device* dev = &interface->dev;
	dev_info(dev, "%s() - called", __func__);

	device_remove_file(&interface->dev, &dev_attr_led);
	usb_free_urb(led->interrupt_out_urb);
	usb_free_urb(led->interrupt_in_urb);
	usb_set_intfdata(interface, NULL);
	usb_put_dev(led->usbdev);
	kfree(led);

	dev_info(dev, "%s() - usb led is now disconnected", __func__);
}

/*
  prepare the struct usb_driver to be registered at the USB core
 */
static struct usb_driver led_driver = {
	.name = "usbled",
	.probe = led_probe,
	.disconnect = led_disconnect,
	.id_table = id_table,
};
module_usb_driver(led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("pic32 usb demo: led/switch usb controlled");
