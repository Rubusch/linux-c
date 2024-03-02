// SPDX-License-Identifier: GPL-2.0+
/*
   The module interacts with the ioctl systemcall. This time the device node in
   the demo will be created by using **devtmpfs** instead of doing it
   manually.

   The main points that differ from the plain chardev demo are:
   - Include the <linux/device.h> header file to create the class
   - The driver will have a class name and a device name; this results in a
     device that appears on the file system at
       /sys/class/<class name>/<device name>
   - the init function is longer, since it automatically will allocate a major
     number

   As most of the "basics" code, contains still a lot of boiler-plate
   code, using the contemporary API frameworks this should not be
   implemented anymore in a modern driver.
*/

#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define DEVICE_NAME "lothars_device"
#define CLASS_NAME "lothars_class"

// static declarations
static struct cdev chardev_dev;   // used for character device operations
static struct class* chardev_class;   // used for /sys/class
dev_t dev;   // used for /sys/class entry

static int chardev_open(struct inode *inode, struct file *file)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static int chardev_close(struct inode *inode, struct file *file)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static long chardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("%s(): called, cmd = %d, arg = %ld\n", __func__, cmd, arg);
	return 0;
}

// fops - declare a file operations structure
static const struct file_operations chardev_fops = {
	.open = chardev_open,
	.release = chardev_close,
	.unlocked_ioctl = chardev_ioctl,
};

static int __init chardev_init(void)
{
	int ret;
	dev_t dev_no;
	int major;
	struct device *chardev_device;

	pr_info("%s(): hello chardev init\n", __func__);

	// allocate device numbers dynamically
	ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
	if (0 > ret) {
		pr_warn("%s(): unable to allocate dynamic major number\n", __func__);
		return ret;
	}

	// get device identifiers
	major = MAJOR(dev_no);
	dev = MKDEV(major, 0);
	pr_info("%s(): allocated correctly with major number %d\n",
			__func__, major);

	// initialize the cdev structure and add it to kernel space
	cdev_init(&chardev_dev, &chardev_fops);
	ret = cdev_add(&chardev_dev, dev, 1);
	if (0 > ret) {
			unregister_chrdev_region(dev, 1);
			pr_info("unable to add cdev\n");
			return ret;
	}

	// register the device class
	chardev_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(chardev_class)) {
		unregister_chrdev_region(dev, 1);
		cdev_del(&chardev_dev);
		pr_warn("%s(): failed to register device class\n", __func__);
		return PTR_ERR(chardev_class);
	}
	pr_info("%s(): device class registered correctly\n", __func__);

	// create a device node named DEVICE_NAME associated to dev
	chardev_device = device_create(chardev_class, NULL, dev, NULL, DEVICE_NAME);
	if (IS_ERR(chardev_device)) {
		class_destroy(chardev_class);
		cdev_del(&chardev_dev);
		unregister_chrdev_region(dev, 1);
		pr_warn("%s(): failed to create the device\n", __func__);
		return PTR_ERR(chardev_device);
	}
	pr_info("%s(): the device is created correctly\n", __func__);

	return 0;
}

static void __exit chardev_exit(void)
{
	pr_info("%s(): hello chardev exit\n", __func__);

	device_destroy(chardev_class, dev);   // remove the device
	class_destroy(chardev_class);   // remove the device class

	cdev_del(&chardev_dev);
	unregister_chrdev_region(dev, 1);
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with character device classes");
