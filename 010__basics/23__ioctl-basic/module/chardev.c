// SPDX-License-Identifier: GPL-2.0+
/*
   The chardev module interacts with the ioctl systemcall.
*/

#include <linux/cdev.h>
#include <linux/fs.h>

#define DEV_MAJOR_NUM 202

static struct cdev chardev;

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

/*
  fops - declare a file operations structure
*/
static const struct file_operations chardev_fops = {
	.owner = THIS_MODULE,
	.open = chardev_open,
	.release = chardev_close,
	.unlocked_ioctl = chardev_ioctl,
};

static int __init chrdev_init(void)
{
	int ret;
	dev_t dev = MKDEV(DEV_MAJOR_NUM, 0); // get first device identifier

	pr_info("%s(): called", __func__);

	// allocate device numbers
	ret = register_chrdev_region(dev, 1, "lothars_cdev");
	if (0 > ret) {
		pr_err("%s(): failed to allocate major number %d",
			__func__, DEV_MAJOR_NUM);
		return ret;
	}

	// initialize the cdev structure and add it to kernel space
	cdev_init(&chardev, &chardev_fops);
	ret = cdev_add(&chardev, dev, 1);
	if (0 > ret) {
		pr_err("%s(): unable to add cdev", __func__);
		unregister_chrdev_region(dev, 1);
		return ret;
	}

	return 0;
}

static void __exit chrdev_exit(void)
{
	pr_info("%s(): called", __func__);
	cdev_del(&chardev);
	unregister_chrdev_region(MKDEV(DEV_MAJOR_NUM, 0), 1);
}

module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with character devices");
