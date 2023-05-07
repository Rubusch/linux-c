/*
   Char Device with Devicetree Binding

   Demonstrates how to connect the chardev driver to the devicetree.

   - Builds on a miscdevice instance
   - Includes <linux/platform_device.h>
   - Declares a list of devices supported by the driver
   - Initializes a struct of_device_id
   - Initializes a struct platform_driver

   ---
   REFERENCES:
   - Linux Driver Development for Embedded Processors, A. L. Rios, 2018

   VERIFIED:
   linux v6.3/aarch64
*/

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

// define major number
#define DEVICE_NAME "lothars_device"

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
	.owner = THIS_MODULE,
	.open = chardev_open,
	.release = chardev_close,
	.unlocked_ioctl = chardev_ioctl,
};

// declare and initialize the struct miscdevice
static struct miscdevice chardev_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &chardev_fops,
};


// of - add probe() function becomes the new __init function
static int __init chardev_probe(struct platform_device *pdev)
{
	int ret;

	pr_info("%s(): chardev init was called!\n", __func__);

	// register the device at the kernel
	ret = misc_register(&chardev_miscdevice);
	if (0 != ret) {
		pr_err("%s(): could not register the misc device mydev\n", __func__);
		return ret;
	}

	pr_info("%s(): got minor %i\n", __func__, chardev_miscdevice.minor);
	return 0;
}

// of - the remove() function becomes the new __exit function, NB: returns int
static int __exit chardev_remove(struct platform_device *pdev)
{
	pr_info("%s(): the chardev exit was called!\n", __func__);

	// unregister the miscdevice instance
	misc_deregister(&chardev_miscdevice);
	return 0;
}

// of - declare a list of devices supported by the driver
static const struct of_device_id chardev_of_ids[] = {
	{ .compatible = "lothars,chardev_keys"},
	{},
};
MODULE_DEVICE_TABLE(of, chardev_of_ids);

// of - define the platform driver structure
static struct platform_driver chardev_platform_driver = {
	.probe = chardev_probe,
	.remove = chardev_remove,
	.driver = {
		.name = "lotharskeys",
		.of_match_table = chardev_of_ids,
		.owner = THIS_MODULE,
	}
};

// of - finally, register the platform driver
module_platform_driver(chardev_platform_driver);

// NB: there are no module_init() / module_exit() macros anymore!!! 
//module_init(chardev_init);
//module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with character device classes");
