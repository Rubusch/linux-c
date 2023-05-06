/*
   char device

   The module interacts with the ioctl systemcall.

   REFERENCE:
   - Linux Driver Development for Embedded Processors, A. L. Rios, 2018
*/

// support char dev
#include <linux/cdev.h>
#include <linux/fs.h>

// define major number
#define MY_MAJOR_NUM 202

static struct cdev my_dev;

static int my_dev_open(struct inode *inode, struct file *file)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("%s(): called, cmd = %d, arg = %ld\n", __func__, cmd, arg);
	return 0;
}

// fops - declare a file operations structure
static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

static int __init chrdev_init(void)
{
	int ret;

	// get first device identifier
	dev_t dev = MKDEV(MY_MAJOR_NUM, 0);
	pr_info("hello chardev init\n");

	// allocate device numbers
	ret = register_chrdev_region(dev, 1, "my_char_device");
	if (0 > ret) {
		pr_info("unable to allocate major number %d\n", MY_MAJOR_NUM);
		return ret;
	}

	// initialize the cdev structure and add it to kernel space
	cdev_init(&my_dev, &my_dev_fops);
	ret = cdev_add(&my_dev, dev, 1);
	if (0 > ret) {
			unregister_chrdev_region(dev, 1);
			pr_info("unable to add cdev\n");
			return ret;
	}

	return 0;
}

static void __exit chrdev_exit(void)
{
	pr_info("hello chardev exit\n");
	cdev_del(&my_dev);
	unregister_chrdev_region(MKDEV(MY_MAJOR_NUM, 0), 1);
}

module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with character devices");
