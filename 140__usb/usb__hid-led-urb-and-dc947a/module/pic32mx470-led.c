// SPDX-License-Identifier: GPL-2.0+
/*
  USB led demo for the PIC32MX470 with ltc3206 multidisplay led
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/i2c.h>

#define DRIVER_NAME "lothars-usb-ltc3206"
#define USBLED_VENDOR_ID 0x04D8
#define USBLED_PRODUCT_ID 0x003F
#define LTC3206_OUTBUF_LEN 3  /* usb write packet length */
#define LTC3206_I2C_DATA_LEN 3

/*
  struct to hold all device specific stuff
 */
struct i2c_ltc3206 {
	u8 obuffer[LTC3206_OUTBUF_LEN];   /* usb write buffer */

	// i2c/smbus data buffer
	u8 user_data_buffer[LTC3206_I2C_DATA_LEN];
	int ep_out;                       /* out endpoint */
	struct usb_device *usb_dev;       /* usb device for this device */
	struct usb_interface *interface;  /* usb interface for this device */
	struct i2c_adapter adapter;       /* i2c related things */

	// wq to wait for an ongoing write
	wait_queue_head_t usb_urb_completion_wait;
	bool ongoing_usb_ll_op;           /* all is in progress */
	struct urb* interrupt_out_urb;
};

/*
  return list of supported functionality
 */
static u32
ltc3206_usb_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C
		| I2C_FUNC_SMBUS_EMUL
		| I2C_FUNC_SMBUS_READ_BLOCK_DATA
		| I2C_FUNC_SMBUS_BLOCK_PROC_CALL;
}

/*
  usb out urb callback

  this callback is registered by the ltc3206_init()
 */
static void
ltc3206_usb_cmpl_cb(struct urb *urb)
{
	struct i2c_ltc3206 *i2cdev = urb->context;
	struct device  *dev = &i2cdev->interface->dev;
	int status = urb->status;
	int ret;

	switch (status) {
	case 0:             // success
		break;
	case -ECONNRESET:   // unlink
	case -ENOENT:
	case -ESHUTDOWN:
/*	case -EPIPE:        // should clear the halt */
		return;
	default:            // error
		goto resubmit;
	}

	// wake up the waiting function,
	// modify the flag indicating the ll status
	i2cdev->ongoing_usb_ll_op = false;  // 0
	// TODO verify        
	wake_up_interruptible(&i2cdev->usb_urb_completion_wait);
	return;

resubmit:
	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret) {
		dev_err(dev, "%s(): can't resubmit interrupt urb, ret = %d", __func__, ret);
	}
}

/*
  called by ltc3206_i2c_write(), from off ltc3206_usb_i2c_xfer()
 */
static int
ltc3206_ll_cmd(struct i2c_ltc3206 *i2cdev)
{
	int ret;
	struct device *dev = &i2cdev->interface->dev;

	// tell everybody to leave the URB alone,
	// we are going to write to the LTC3206 device
	i2cdev->ongoing_usb_ll_op = true; // 1

	// submit the interrutp out URB packet
	if (usb_submit_urb(i2cdev->interrupt_out_urb, GFP_KERNEL)) {
		dev_err(dev, "%s(): usb_submit_urb intr out failed", __func__);
		i2cdev->ongoing_usb_ll_op = false; // 0
		return -EIO;
	}

	// wait for the transmit completion,
	// the USB URB callback will signal it
	ret = wait_event_interruptible(i2cdev->usb_urb_completion_wait, (!i2cdev->ongoing_usb_ll_op));
	if (0 > ret) {
		dev_err(dev, "%s(): wait interrupted", __func__);
		goto ll_exit_clear_flag;
	}

	return 0;

ll_exit_clear_flag:
	i2cdev->ongoing_usb_ll_op = false; // 0
	return ret;
}

/*
  init function

  this init function is called by the ltc3206_probe()
 */
static int
ltc3206_init(struct i2c_ltc3206 *i2cdev)
{
	int ret;
	struct device *dev = &i2cdev->interface->dev;

	// initialize the ltc3206
	dev_info(dev, "%s(): called", __func__);
	dev_info(dev, "%s(): ltc3206 at USB bus 0x%08x address 0x%08x",
		 __func__, i2cdev->usb_dev->bus->busnum, i2cdev->usb_dev->devnum);

	// allocate the int out URB
	i2cdev->interrupt_out_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!i2cdev->interrupt_out_urb) {
		ret = -ENODEV;
		goto err_init;
	}

	// init the int out URB
	usb_fill_int_urb(i2cdev->interrupt_out_urb,
			 i2cdev->usb_dev,
			 usb_sndintpipe(i2cdev->usb_dev, i2cdev->ep_out),
			 (void*) &i2cdev->obuffer,
			 LTC3206_OUTBUF_LEN,
			 ltc3206_usb_cmpl_cb,
			 i2cdev,
			 1);
	ret = 0;

	goto no_err_init;

err_init:
	dev_err(dev, "%s(): ltc3206 init failed to allocate the int out URB, ret = %d", __func__, ret);
	return ret;

no_err_init:
	dev_err(dev, "%s(): ltc3206 init ok, ret = %d", __func__, ret);
	return ret;
}

/*
  called by ltc3206_usb_i2c_xfer()
 */
static int ltc3206_i2c_write(struct i2c_ltc3206 *i2cdev, struct i2c_msg *pmsg)
{
	u8 ucXferLen;
	u8 *pSrc, *pDst;
	int ret, idx;

	// NB: use pr_debug() and turn on output with dynamic debuggin
	// TODO use pr_debug()    
	pr_debug("%s(): called", __func__);
	if (LTC3206_I2C_DATA_LEN < pmsg->len) {
		pr_err("%s(): problem with the length", __func__);
		return -EINVAL;
	}

	// i2c write length
	ucXferLen = (u8) pmsg->len;

	pSrc = &pmsg->buf[0];
	pDst = &i2cdev->obuffer[0];
	memcpy(pDst, pSrc, ucXferLen);

	for (idx=0; idx<sizeof(i2cdev->obuffer); idx++) {
		pr_debug("%s(): obuffer[%d] = 0x%08x", __func__, idx, i2cdev->obuffer[idx]);
	}

	ret = ltc3206_ll_cmd(i2cdev);
	if (0 > ret) {
		return -EFAULT;
	}

	return 0;
}

/*
  device layer, called from the I2C user app
 */
static int
ltc3206_usb_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct i2c_ltc3206 *i2cdev = i2c_get_adapdata(adap);
	struct i2c_msg *pmsg;
	int ret, count;

	// NB: as an alternative to dev_info(&i2cdev->interface->dev, "...");
	pr_info("%s(): called", __func__);

	pr_info("%s(): number of i2c msgs = %d", __func__, num);
	for (count = 0; count < num; count++) {
		pmsg = &msgs[count];
		ret = ltc3206_i2c_write(i2cdev, pmsg);
		if (0 > ret) {
			pr_err("%s(): failed to write to i2c", __func__);
			goto err;
		}
	}

	// return number of written bytes,
	// if all the messages were transferred ok
	ret = num;

err:
	return ret;
}

static const struct i2c_algorithm ltc3206_usb_algorithm = {
	.master_xfer = ltc3206_usb_i2c_xfer,
	.functionality = ltc3206_usb_func,
};

static const struct usb_device_id ltc3206_table[] = {
	{ USB_DEVICE(USBLED_VENDOR_ID, USBLED_PRODUCT_ID) },
	{ },
};
MODULE_DEVICE_TABLE(usb, ltc3206_table);

/*
  clean up i2c and usb dev
 */
static void
ltc3206_free(struct i2c_ltc3206 *i2cdev)
{
	usb_put_dev(i2cdev->usb_dev);
	usb_set_intfdata(i2cdev->interface, NULL);
	kfree(i2cdev);
}

/*
 */
static int
ltc3206_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_host_interface *hostif = interface->cur_altsetting;
	struct i2c_ltc3206 *i2cdev;
	struct device *dev = &interface->dev;
	int ret;

	dev_info(dev, "%s(): called", __func__);

	// allocate memory for our device and initialize it
	i2cdev = kzalloc(sizeof(*i2cdev), GFP_KERNEL);
	if (NULL == i2cdev) {
		dev_err(dev, "%s(): allocation of i2c device memory failed", __func__);
		ret = -ENOMEM;
		goto err;
	}

	// get endpoint out, ep_out, address
	i2cdev->ep_out = hostif->endpoint[1].desc.bEndpointAddress;
	i2cdev->usb_dev = usb_get_dev(interface_to_usbdev(interface));
	i2cdev->interface = interface;

	init_waitqueue_head(&i2cdev->usb_urb_completion_wait);

	// save data pointer in this interface device
	usb_set_intfdata(interface, i2cdev);

	// setup i2c adapter description
	i2cdev->adapter.owner = THIS_MODULE;
	i2cdev->adapter.class = I2C_CLASS_HWMON;
	i2c_set_adapdata(&i2cdev->adapter, i2cdev);

	snprintf(i2cdev->adapter.name, sizeof(i2cdev->adapter.name),
		 DRIVER_NAME " at bus 0x%08x device 0x%08x",
		 i2cdev->usb_dev->bus->busnum,
		 i2cdev->usb_dev->devnum);

	i2cdev->adapter.dev.parent = &i2cdev->interface->dev;

	// initialize the ltc3206 device
	ret = ltc3206_init(i2cdev);
	if (0 > ret) {
		dev_err(dev, "%s(): failed to init i2c adapter", __func__);
		goto err_init;
	}

	// attach to i2c layer
	ret = i2c_add_adapter(&i2cdev->adapter);
	if (0 > ret) {
		dev_err(dev, "%s(): failed to add i2c adapter", __func__);
//		goto err_i2c; // TODO rm   
		goto err_init;
	}

	dev_info(dev, "%s(): connected", __func__);
	return 0;

err_init:
	usb_free_urb(i2cdev->interrupt_out_urb);
//err_i2c: // TODO move up?     
	usb_set_intfdata(interface, NULL);  // set interface to NULL
	ltc3206_free(i2cdev);  // set i2cdev->interface to NULL

err:
	return ret;
}

/*
  disconnect from i2c and usb
 */
static void
ltc3206_disconnect(struct usb_interface *interface)
{
	struct i2c_ltc3206 *i2cdev = usb_get_intfdata(interface);
	struct device *dev = &interface->dev;

	dev_info(dev, "%s(): called", __func__);

	i2c_del_adapter(&i2cdev->adapter);
	usb_kill_urb(i2cdev->interrupt_out_urb);
	usb_free_urb(i2cdev->interrupt_out_urb);
	usb_set_intfdata(interface, NULL);
	ltc3206_free(i2cdev);
}

static struct usb_driver ltc3206_driver = {
	.name = DRIVER_NAME,
	.probe = ltc3206_probe,
	.disconnect = ltc3206_disconnect,
	.id_table = ltc3206_table,
};
module_usb_driver(ltc3206_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("pic32 usb demo with LTC3206 and DC749a");
