// SPDX-License-Identifier: GPL-2.0+
/*
  USB led demo for the PIC32MX470

  The driver requires the PIC32MX470 hardware and thus the MPLAB IDE
  with a USB HID application project.
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#define USBLED_VENDOR_ID   0x04D8
#define USBLED_PRODUCT_ID  0x003F


/*
  NB: The vendor and product id will only match, if the usbhid module
   is not present!
 */
static const struct usb_device_id usbled_id_table[] = {
	{ USB_DEVICE(USBLED_VENDOR_ID, USBLED_PRODUCT_ID) },
	{ },
};
MODULE_DEVICE_TABLE(usb, usbled_id_table);

/*
  Create a private structure that will store the driver's data.
 */
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
	dev_info(dev, "%s() - called\n", __func__);

	return sprintf(buf, "%d\n", led->led_number);
}

/*
  Everytime someone writes to the sysfs entry,
  e.g. /sys/bus/usb/devices/.../led, the driver's led_store() is
  called. The struct usb_led associated to the USB device is recovered
  by using usb_get_intfdata(). Any command written to the led sysfs
  entry is stored in the variable val. Finally, the command value will
  be sent via USB by using usb_bulk_msg().

  The kernel provides usb_bulk_msg() and usb_control_msg() as helper
  functions that make it possible to transfer simple bulk and control
  messages without having to create an URB; initialize it; submit it;
  and wait for its completion handler. These functions are synchronous
  and will make your code sleep. They must not be called from
  interrupt context or with a spinlock held i.e. not in ATOMIC context!
 */
static ssize_t
led_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usb_led *led = usb_get_intfdata(intf);
	u8 val;
	int error, ret;

	dev_info(dev, "%s() - called\n", __func__);

	// transform char array to u8 value
	error = kstrtou8(buf, 10, &val);
	if (error)
		return error;

	led->led_number = val;
	if (val == 1 || val == 2 || val == 3 || val == 0) {
		dev_info(dev, "%s() - led = %d\n",
			 __func__, led->led_number);
	} else {
		dev_info(dev, "%s() - unknown led %d\n",
			 __func__, led->led_number);
		ret = -EINVAL;
		return ret;
	}

	/*
	  toggle led using usb_bulk_msg()

	  arguments:
	  - usb_dev: pointer to the usb device to send the message to
	  - pipe: endpoint "pipe" to send the message to
	  - data: pointer to the data to send
	  - len: length in bytes of the data to send
	  - actual_length: pointer to a location to put the actual
  	        length transferred in bytes
	  - timeout: time in msec to wait for the message to complete

	 */
	ret = usb_bulk_msg(led->usbdev, usb_sndctrlpipe(led->usbdev, 1),
			   &led->led_number,
			   1, NULL, 0);
	if (ret) {
		ret = -EFAULT;
		return ret;
	}
	return count;
}
static DEVICE_ATTR_RW(led);

/*
  The probe() function is called by the USB core into the driver to
  see if the driver is willing to manage a particular interface on a
  device. If yes, probe() returns 0 and uses usb_set_intfdata() to
  associate driver specific data with the interface.

  The struct usb_driver defines some power management (pm) callbacks:
  - suspend()
  - resume()
  - reset_resume()  // the suspended device has been reset instead
                    // of being resumed

  ..and device level operations, such as
  - pre_reset()     // the device is about to be reset
  - post_reset()    // the device has been reset
 */
static int
usbled_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	// obtain the struct usb_device from the usb_interface
	struct usb_device *usbdev = interface_to_usbdev(interface);
	struct usb_led *led = NULL;
	struct device *dev = &interface->dev;
	int ret = -ENOMEM;

	dev_info(dev, "%s() - called\n", __func__);

	// allocate the private data structure
	led = kzalloc(sizeof(*led), GFP_KERNEL);
	if (!led) {
		dev_err(dev, "%s() - out of memory\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	// store the usb device in our data structure
	led->usbdev = usb_get_dev(usbdev);

	// attach the USB device data to the USB interface
	usb_set_intfdata(interface, led);

	// create a led sysfs entry to interact with user space
	ret = device_create_file(&interface->dev, &dev_attr_led);
	if (ret)
		goto err_create_file;

	return 0;

err_create_file:
	usb_put_dev(usbdev);
	usb_set_intfdata(interface, NULL);

err:
	kfree(dev);
	return ret;
}

static void
usbled_disconnect(struct usb_interface *interface)
{
	struct usb_led *led = usb_get_intfdata(interface);
	struct device* dev = &interface->dev;

	dev_info(dev, "%s() - called\n", __func__);

	device_remove_file(&interface->dev, &dev_attr_led);
	usb_set_intfdata(interface, NULL);
	usb_put_dev(led->usbdev);
	kfree(led);

	dev_info(dev, "%s() - usb led is now disconnected\n", __func__);
}

/*
  prepare the struct usb_driver to be registered at the USB core
 */
static struct usb_driver led_driver = {
	.name = "lothars_usbled",
	.id_table = usbled_id_table,
	.probe = usbled_probe,
	.disconnect = usbled_disconnect,
};
module_usb_driver(led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("pic32mx470 usb demo");
