// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_chardev(void);
void cleanup_hello_chardev(void);

#define HELLO_CHARDEV_MAJOR 244 /* any number */
#define HELLO_CDEV_NAME "lothars_hello_cdev"
#define HELLO_CLASS_NAME "lothars_hello_class"
#define HELLO_DEVICE_NAME "lothars_hello_device"

// device setup
dev_t dev = 0;

int init_hello_chardev(void)
{
	int ret = -1;
	printk(KERN_INFO "%s(): called\n", __func__);

	// allocate major number
	ret = alloc_chrdev_region(&dev, HELLO_CHARDEV_MAJOR, 1, HELLO_CDEV_NAME);
	if (0 > ret) {
		printk(KERN_ERR "%s(): alloc_chrdev_region() failed\n", __func__);
		return -ENOMEM;
	}
	printk(KERN_INFO "%s(): major = %d, minor = %d\n",
	       __func__, MAJOR(dev), MINOR(dev));

	printk(KERN_INFO "%s(): done.\n", __func__);
	return 0;
}

void cleanup_hello_chardev(void)
{
	unregister_chrdev_region(dev, 1);

	printk(KERN_INFO "%s(): READY.\n", __func__);
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
