// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_chardev(void);
void cleanup_hello_chardev(void);

/*
  globals
*/

#define HELLO_CHARDEV_MAJOR 244 /* any number */
#define HELLO_CDEV_NAME "lothars_hello_cdev"
#define HELLO_CLASS_NAME "lothars_hello_class"
#define HELLO_DEVICE_NAME "lothars_hello_device"

// device setup
dev_t dev = 0;

/*
  start / stop module
*/

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
	if (0 > alloc_chrdev_region(&dev, HELLO_CHARDEV_MAJOR, 1,
				    HELLO_CDEV_NAME)) {
		printk(KERN_ERR "alloc_chrdev_region() failed\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "%s() major = %d, minor = %d\n", __func__, MAJOR(dev),
	       MINOR(dev));

	printk(KERN_INFO "%s() done.\n", __func__);
	return 0;
}

void cleanup_hello_chardev(void)
{
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

/*
  init / exit
*/

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
