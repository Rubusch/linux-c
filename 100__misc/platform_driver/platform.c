// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#define PLATFORM_DEVNAME "lothars_device"
#define LOTHARS_PLATFORM_DRIVER "lothars-platform-dummy" /* must match with ins */

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

static struct miscdevice pf_device = {
	.name = PLATFORM_DEVNAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int pf_probe (struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);
	if (misc_register(&pf_device)) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static int pf_remove(struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);
	misc_deregister(&pf_device);
	return 0;
}

static struct platform_driver pf_drv = {
	.probe      = pf_probe,
	.remove     = pf_remove,
	.driver     = {
		.name     = LOTHARS_PLATFORM_DRIVER, /* can be called from ins */
		.owner    = THIS_MODULE,
	},
};
module_platform_driver(pf_drv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with platform drivers.");
