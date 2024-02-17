// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#define PLATFORM_DEVICE_NAME "lothars_device"
#define PLATFORM_DRIVER_NAME "lothars-platform-dummy" /* must match with ins */

int dummy_open(struct inode * inode, struct file * filp)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

int dummy_release(struct inode * inode, struct file * filp)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

ssize_t dummy_read (struct file *filp, char __user * buf, size_t count,
		    loff_t * offset)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

ssize_t dummy_write(struct file * filp, const char __user * buf, size_t count,
		    loff_t * offset)
{
	pr_info("%s(): called\n", __func__);
	return count;
}

static struct file_operations fops = {
	.open = dummy_open,
	.release = dummy_release,
	.read = dummy_read,
	.write = dummy_write,
};

static struct miscdevice pdrv_device = {
	.name = PLATFORM_DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int pdrv_probe (struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);
	if (misc_register(&pdrv_device)) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static int pdrv_remove(struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);
	misc_deregister(&pdrv_device);
	return 0;
}

static struct platform_driver pdrv_driver = {
	.probe      = pdrv_probe,
	.remove     = pdrv_remove,
	.driver     = {
		.name     = PLATFORM_DRIVER_NAME, /* can be called from ins */
		.owner    = THIS_MODULE,
	},
};
module_platform_driver(pdrv_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with platform drivers.");
