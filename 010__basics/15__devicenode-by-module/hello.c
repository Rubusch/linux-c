// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

int init_hello_chardev(void);
void cleanup_hello_chardev(void);

#define HELLO_CHARDEV_MINOR 123 /* any number */
#define HELLO_CDEV_NAME "lothars_hello_cdev"
#define HELLO_CLASS_NAME "lothars_hello_class"
#define HELLO_DEVICE_NAME "lothars_hello_device"

dev_t dev = 0;
static struct class *dev_class;

int init_hello_chardev(void)
{
	printk(KERN_INFO "%s(): initializing\n", __func__);

	// allocate major number
	if (0 > alloc_chrdev_region(&dev, HELLO_CHARDEV_MINOR, 1,
				    HELLO_CDEV_NAME)) {
		printk(KERN_ERR "%s(): alloc_chrdev_region() failed\n", __func__);
		return -ENOMEM;
	}
	printk(KERN_INFO "%s(): major = %d, minor = %d\n", __func__, MAJOR(dev),
	       MINOR(dev));

	// create class instance
	/**
	 * class_create - create a struct class structure
	 * @owner: pointer to the module that is to "own" this struct class
	 * @name: pointer to a string for the name of this class.
	 * @key: the lock_class_key for this class; used by mutex lock debugging
	 *
	 * This is used to create a struct class pointer that can then be used
	 * in calls to device_create().
	 *
	 * Returns &struct class pointer on success, or ERR_PTR() on error.
	 *
	 * Note, the pointer created here is to be destroyed when finished by
	 * making a call to class_destroy().
	 */
	dev_class = class_create(THIS_MODULE, HELLO_CLASS_NAME);
	if (NULL == dev_class) {
		printk(KERN_ERR "%s(): class_create() failed\n", __func__);
		goto err_class;
	}

	// create device instance
	/**
	 * device_create - creates a device and registers it with sysfs
	 * @class: pointer to the struct class that this device should be
	 *         registered to
	 * @parent: pointer to the parent struct device of this new device, if any
	 * @devt: the dev_t for the char device to be added
	 * @drvdata: the data to be added to the device for callbacks
	 * @fmt: string for the device's name
	 *
	 * This function can be used by char device classes.  A struct device
	 * will be created in sysfs, registered to the specified class.
	 *
	 * A "dev" file will be created, showing the dev_t for the
	 * device, if the dev_t is not 0,0.  If a pointer to a parent
	 * struct device is passed in, the newly created struct device
	 * will be a child of that device in sysfs.  The pointer to
	 * the struct device will be returned from the call.  Any
	 * further sysfs files that might be required can be created
	 * using this pointer.
	 *
	 * Returns &struct device pointer on success, or ERR_PTR() on error.
	 *
	 * Note: the struct class passed to this function must have previously
	 * been created with a call to class_create().
	 */
	if (NULL == device_create(dev_class, NULL, dev,
				  NULL, HELLO_DEVICE_NAME)) {
		printk(KERN_ERR "%s(): device_create() failed\n", __func__);
		goto err_device;
	}

	printk(KERN_INFO "%s() done.\n", __func__);
	return 0;

err_device:
	class_destroy(dev_class);

err_class:
	unregister_chrdev_region(dev, 1);

	return -ENOMEM;
}

void cleanup_hello_chardev(void)
{
	/**
	 * device_destroy - removes a device that was created with device_create()
	 * @class: pointer to the struct class that this device was registered
	 *         with
	 * @devt: the dev_t of the device that was previously registered
	 *
	 * This call unregisters and cleans up a device that was created with a
	 * call to device_create().
	 */
	device_destroy(dev_class, dev);

	/**
	 * class_destroy - destroys a struct class structure
	 * @cls: pointer to the struct class that is to be destroyed
	 *
	 * Note, the pointer to be destroyed must have been created with a call
	 * to class_create().
	 */
	class_destroy(dev_class);

	/**
	 * unregister_chrdev_region() - unregister a range of device numbers
	 * @from: the first in the range of numbers to unregister
	 * @count: the number of device numbers to unregister
	 *
	 * This function will unregister a range of @count device numbers,
	 * starting with @from.  The caller should normally be the one who
	 * allocated those numbers in the first place...
	 */
	unregister_chrdev_region(dev, 1);

	printk(KERN_INFO "%s() READY.\n", __func__);
}

static int __init mod_init(void)
{
	return init_hello_chardev();
}

static void __exit mod_exit(void)
{
	cleanup_hello_chardev();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates a character device driver and device file.");
