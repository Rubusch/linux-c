/*
   char device

   Demonstrated the usage of the miscdevice framework, and will result in
   fewer lines of code.

   struct miscdevice {
     int minor;
	 const char *name;
	 const struct file_operations *fops;
	 struct list_head list;
	 struct device *this_device;
	 const char *nodename;
	 umode_t mode;
   };

   The main points in this demo are:
   - Add the header <linux/miscdevice.h>
   - Initialize the miscdevice structure
   - Register and unregister the device with the kernel, using
     misc_register()/misc_deregister()

   ---
   REFERENCES:
   - Linux Driver Development for Embedded Processors, A. L. Rios, 2018

   VERIFIED:
   linux v6.3/aarch64
*/

// support char dev
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

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

static int __init chardev_init(void)
{
	int ret;

	pr_info("%s(): hello chardev init\n", __func__);

	// register the device at the kernel
	ret = misc_register(&chardev_miscdevice);

	if (0 != ret) {
		pr_err("%s(): could not register the misc device mydev\n", __func__);
		return ret;
	}

	pr_info("%s(): got minor %i\n", __func__, chardev_miscdevice.minor);
	return 0;
}

static void __exit chardev_exit(void)
{
	pr_info("%s(): hello chardev exit\n", __func__);

	// unregister the miscdevice instance
	misc_deregister(&chardev_miscdevice);
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with character device classes");
