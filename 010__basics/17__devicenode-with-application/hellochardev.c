// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/slab.h> /* kmalloc(), kfree() */
#include <linux/uaccess.h>

#include "hellochardev.h"

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_chardev(void);
void cleanup_hello_chardev(void);

static int hello_open(struct inode *, struct file *);
static int hello_release(struct inode *, struct file *);
static ssize_t hello_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t hello_write(struct file *, const char __user *, size_t,
			   loff_t *);
// device setup
dev_t dev = 0;
static struct class *dev_class;
static struct cdev hello_chardev_cdev;

// buffer to hold data
uint8_t *kernel_buf;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.release = hello_release,
	.read = hello_read,
	.write = hello_write,
};

/**
  Called when opening the device file.
 */
static int hello_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "%s()\n", __func__);
	kernel_buf = kmalloc(KERNEL_BUF_SIZE, GFP_KERNEL);
	if (0 == kernel_buf) {
		printk(KERN_ERR "kmalloc() failed\n");
		return -ENOMEM;
	}
	return 0;
}

/**
  Called when closing the device file.
 */
static int hello_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "%s()\n", __func__);
	kfree(kernel_buf);
	return 0;
}

/**
  Called when someone (user/kernel) reads from the device file.
 */
static ssize_t hello_read(struct file *file, char __user *buf, size_t len,
			  loff_t *poff)
{
	printk(KERN_INFO "%s()\n", __func__);
	if (copy_to_user(buf, kernel_buf, KERNEL_BUF_SIZE)) {
		printk("%s() failed\n", __func__);
		return -EFAULT;
	}
	return 0;
}

/**
  Called when someone (user/kernel) writes into the device file.
 */
static ssize_t hello_write(struct file *file, const char __user *buf,
			   size_t len, loff_t *poff)
{
	printk(KERN_INFO "%s()\n", __func__);
	if (copy_from_user(kernel_buf, buf, len)) {
		printk("%s() failed\n", __func__);
		return -EFAULT;
	}
	return len; // if this is 0, it will spin around the "write"
}

int init_hello_chardev(void)
{
	printk(KERN_INFO "%s() initializing\n", __func__);

	// allocate major number
	/**
	 * alloc_chrdev_region() - register a range of char device numbers
	 * @dev: output parameter for first assigned number
	 * @baseminor: first of the requested range of minor numbers
	 * @count: the number of minor numbers required
	 * @name: the name of the associated device or driver
	 *
	 * Allocates a range of char device numbers.  The major number will be
	 * chosen dynamically, and returned (along with the first minor number)
	 * in @dev.  Returns zero or a negative error code.
	 */
	if (0 > alloc_chrdev_region(&dev, HELLO_CHARDEV_MINOR, 1,
				    HELLO_CDEV_NAME)) {
		printk(KERN_ERR "alloc_chrdev_region() failed\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "%s() major = %d, minor = %d\n", __func__, MAJOR(dev),
	       MINOR(dev));

	// create cdev structure (character device)
	/**
	 * cdev_init() - initialize a cdev structure
	 * @cdev: the structure to initialize
	 * @fops: the file_operations for this device
	 *
	 * Initializes @cdev, remembering @fops, making it ready to add to the
	 * system with cdev_add().
	 */
	cdev_init(&hello_chardev_cdev, &fops);

	// add cdev to system
	/**
	 * cdev_add() - add a char device to the system
	 * @p: the cdev structure for the device
	 * @dev: the first device number for which this device is responsible
	 * @count: the number of consecutive minor numbers corresponding to this
	 *         device
	 *
	 * cdev_add() adds the device represented by @p to the system, making it
	 * live immediately.  A negative error code is returned on failure.
	 */
	if (0 > cdev_add(&hello_chardev_cdev, dev, 1)) {
		printk(KERN_ERR "cdev_add() faied\n");
		goto err_cdev;
	}

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
		printk(KERN_ERR "class_create() failed\n");
		goto err_class;
	}

	// create device instance
	/**
	 * device_create - creates a device and registers it with sysfs
	 * @class: pointer to the struct class that this device should be registered to
	 * @parent: pointer to the parent struct device of this new device, if any
	 * @devt: the dev_t for the char device to be added
	 * @drvdata: the data to be added to the device for callbacks
	 * @fmt: string for the device's name
	 *
	 * This function can be used by char device classes.  A struct device
	 * will be created in sysfs, registered to the specified class.
	 *
	 * A "dev" file will be created, showing the dev_t for the device, if
	 * the dev_t is not 0,0.
	 * If a pointer to a parent struct device is passed in, the newly created
	 * struct device will be a child of that device in sysfs.
	 * The pointer to the struct device will be returned from the call.
	 * Any further sysfs files that might be required can be created using this
	 * pointer.
	 *
	 * Returns &struct device pointer on success, or ERR_PTR() on error.
	 *
	 * Note: the struct class passed to this function must have previously
	 * been created with a call to class_create().
	 */
	if (NULL ==
	    device_create(dev_class, NULL, dev, NULL, HELLO_DEVICE_NAME)) {
		printk(KERN_ERR "device_create() failed\n");
		goto err_device;
	}

	printk(KERN_INFO "%s() done.\n", __func__);
	return 0;

err_device:
	class_destroy(dev_class);

err_class:
	cdev_del(&hello_chardev_cdev);

err_cdev:
	unregister_chrdev_region(dev, 1);

	return -ENOMEM;
}

void cleanup_hello_chardev(void)
{
	/**
	 * device_destroy - removes a device that was created with device_create()
	 * @class: pointer to the struct class that this device was registered with
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
	 * cdev_del() - remove a cdev from the system
	 * @p: the cdev structure to be removed
	 *
	 * cdev_del() removes @p from the system, possibly freeing the structure
	 * itself.
	 *
	 * NOTE: This guarantees that cdev device will no longer be able to be
	 * opened, however any cdevs already open will remain and their fops will
	 * still be callable even after cdev_del returns.
	 */
	cdev_del(&hello_chardev_cdev);

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
