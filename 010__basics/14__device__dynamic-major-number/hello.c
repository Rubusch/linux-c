// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#define LOTHARS_DEVICE_NAME "lothars_device"
#define DEVICE_MINOR_NUMBER 123

// device node
dev_t dev = 0;

int init_devnum(void)
{
	pr_info("%s(): initializing...!\n", __func__);
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
	if (0 > alloc_chrdev_region(&dev, DEVICE_MINOR_NUMBER, 1,
				    LOTHARS_DEVICE_NAME)) {
		pr_err("%s(): alloc_chrdev_region() failed", __func__);
		return -ENOMEM;
	}
	pr_info("%s(): major = %d, minor = %d\n", __func__, MAJOR(dev),
	       MINOR(dev));
	return 0;
}

void cleanup_devnum(void)
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
	pr_info("%s(): READY.\n", __func__);
}

static int __init mod_init(void)
{
	return init_devnum();
}

static void __exit mod_exit(void)
{
	cleanup_devnum();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates major and minor device numbers!");
