// SPDX-License-Identifier: GPL-2.0+
/*
   The chardev module interacts with the ioctl systemcall.
*/

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#include "ioctl_dev.h"

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
	.open = chardev_open,
	.release = chardev_close,
	.unlocked_ioctl = chardev_ioctl,
};

static struct miscdevice ioctl_dev = {
	.name = IOCTL_DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void)
{
	int ret;
	pr_info("%s(): called\n", __func__);
	ret = misc_register(&ioctl_dev);
	if (0 != ret) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	misc_deregister(&ioctl_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with character devices");
