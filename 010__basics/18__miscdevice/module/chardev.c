// SPDX-License-Identifier: GPL-2.0+
/*
   The miscdevice simplifies all steps needed for char device.

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
*/

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

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

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = chardev_open,
	.release = chardev_close,
	.unlocked_ioctl = chardev_ioctl,
};

static struct miscdevice chardev_miscdevice = {
	.name = DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init chardev_init(void)
{
	int ret;

	pr_info("%s(): hello chardev init\n", __func__);

	/*
	  register the device at the kernel

	  miscdevice registration performs:
	  1. alloc_chrdev_region(); – used for the major and minor number
	  2. cdev_init();           – used to initialize cdev
	  3. cdev_add();            – used to  add the cdev structure to the device
	  4. class_create();        – used to create a class
	  5. device_create();       – used to create a device
	*/
	ret = misc_register(&chardev_miscdevice);

	if (0 != ret) {
		pr_err("%s(): could not register the misc device mydev\n",
		       __func__);
		return ret;
	}

	pr_info("%s(): got minor %i\n", __func__, chardev_miscdevice.minor);
	return 0;
}

static void __exit chardev_exit(void)
{
	pr_info("%s(): hello chardev exit\n", __func__);

	/*
	  unregister the miscdevice instance

	  unregister miscdevice performs:
	  1. cdev_del();        – used to delete cdev
	  2. unregister_chrdev_region();  – used to remove the
	                          major and minor number
	  3. device_destroy();  – used to delete the device
	  4. class_destroy();   – used to delete the class
	 */
	misc_deregister(&chardev_miscdevice);
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with character device classes");
