// SPDX-License-Identifier: GPL-2.0+
/*
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include <linux/miscdevice.h>

// a "special" device derrived from another, here, miscdevice
#define DEV_NAME "lothars_device"
#define DEV_MEMSIZE 256
struct special_dev
{
	struct miscdevice *misc_dev;
	char content[DEV_MEMSIZE];
};

static struct special_dev sp_dev;

static int
demo_open(struct inode *inode, struct file *file)
{
	file->private_data = &sp_dev;
	return 0;
}

static ssize_t
demo_read(struct file *file, char __user *ubuf, size_t count, loff_t *off)
{
	struct special_dev *dev;

	pr_info("%s(): called\n", __func__);
	dev = container_of(file->private_data, struct special_dev, misc_dev);
	if (0 == *off) {
		if (copy_to_user(ubuf, &dev->content, sizeof(dev->content))) {
			pr_err("%s(): failed to return content to userspace\n",
			       __func__);
			return -EFAULT;
		}

		return 0;
	}

	return count;
}

static ssize_t
demo_write(struct file *file, const char __user *ubuf,
	   size_t count, loff_t *off)
{
	struct special_dev *dev;

	pr_info("%s(): called\n", __func__);
	dev = container_of(file->private_data, struct special_dev, misc_dev);
	if (copy_from_user(dev->content, ubuf, count)) {
		pr_err("%s(): failed to copy content\n", __func__);
		return -EFAULT;
	}
	dev->content[count-1] = '\0';
	pr_info("%s(): received from userspace '%s'\n",
		__func__, dev->content);

	return count;
}

static struct file_operations fops = {
	.open = demo_open,
	.read = demo_read,
	.write = demo_write,
};

static struct miscdevice misc_dev = {
	.name = DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void)
{
	int ret = 0;
	struct special_dev *dev;

	pr_info("%s(): called\n", __func__);
	dev = &sp_dev;

	dev->misc_dev = &misc_dev;
	ret = misc_register(dev->misc_dev);
	if (ret) {
		return -EINVAL;
	}
	return ret;
}

static void __exit mod_exit(void)
{
	struct special_dev *dev;

	pr_info("%s(): called\n", __func__);
	dev = &sp_dev;
	misc_deregister(dev->misc_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Show usage of private data of a character device.");
