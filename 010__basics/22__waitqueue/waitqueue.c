// SPDX-License-Identifier: GPL-2.0+
/*
  wait on events using a waitqueue
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/wait.h>

#define HELLO_CDEV_NAME "lothars_hello_cdev"
#define HELLO_CLASS_NAME "lothars_hello_class"
#define HELLO_DEVICE_NAME "lothars_hello_device"
#define HELLO_CHARDEV_MINOR 123

dev_t dev = 0;
static struct class *dev_class;
static struct cdev hello_chardev_cdev;

static struct task_struct *kthread1, *kthread2;
static long int waitqueue_flag = 0;
DECLARE_WAIT_QUEUE_HEAD(wq1); // static creation
static wait_queue_head_t wq2; // prepares dynamic creation

/*
  thread routine
*/
static int thread_routine(void *arg)
{
	int status = -1;
	int sel = *(int *) arg;
	switch (sel) {
	case 1:
		wait_event(wq1, waitqueue_flag == 11);
		pr_info("%s(): waitqueue_flag is %ld", __func__, waitqueue_flag);
		break;
	case 2:
		do {
			status = wait_event_timeout(wq2, waitqueue_flag == 22,
						    msecs_to_jiffies(5000));
			pr_info("%s(): waitqueue_flag is pending.. timeout",
				__func__);
		} while (0 == status);
		break;
	default:
		pr_warn("%s(): should not reach here", __func__);
		break;
	}
	return 0;
}

/*
  Write() - we can `echo 1 > /dev/...`, but not Read() from the device
*/
static ssize_t
hello_write(struct file *file, const char __user *ubuf, size_t len, loff_t *poff)
{
	int size, not_copied, ret;
	char buf[32];

	pr_info("%s(): called", __func__);
	memset(buf, '\0', sizeof(buf));
	size = min(len, sizeof(buf));
	not_copied = copy_from_user(buf, ubuf, size);
	if (0 > not_copied) {
		pr_err("%s(): faild to read from userspace", __func__);
		return -ENOMEM;
	}
	ret = kstrtol(buf, 10, &waitqueue_flag);
	if (-EINVAL == ret) {
		pr_err("%s(): failed to convert value read from userspace",
		       __func__);
		return -EINVAL;
	}
	pr_info("%s(): waitqueue_flag is %ld", __func__, waitqueue_flag);

	wake_up(&wq1);
	wake_up(&wq2);

	return len;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.write = hello_write,
};

int init_hello(void)
{
	int thr1 = 1, thr2 = 2, ret;

	pr_info("%s() called", __func__);
	init_waitqueue_head(&wq2); // dynamic creation

	ret = alloc_chrdev_region(&dev, HELLO_CHARDEV_MINOR, 1, HELLO_CDEV_NAME);
	if (0 > ret) {
		pr_err("%s(): alloc_chrdev_region() failed", __func__);
		return -ENOMEM;
	}
	pr_info("%s(): major = %d, minor = %d\n",
		__func__, MAJOR(dev),MINOR(dev));
	cdev_init(&hello_chardev_cdev, &fops);
	ret = cdev_add(&hello_chardev_cdev, dev, 1);
	if (0 > ret) {
		pr_err("%s(): cdev_add() failed", __func__);
		goto err_cdev;
	}
	dev_class = class_create(THIS_MODULE, HELLO_CLASS_NAME);
	if (NULL == dev_class) {
		pr_err("%s(): class_create() failed", __func__);
		goto err_class;
	}
	if (NULL ==
	    device_create(dev_class, NULL, dev, NULL, HELLO_DEVICE_NAME)) {
		pr_err("%s(): device_create() failed", __func__);
		goto err_device;
	}

	kthread1 = kthread_run(thread_routine, &thr1, "kthread1");
	if (NULL == kthread1) {
		pr_err("%s(): failed to create kthread1", __func__);
		goto err_device;
	}

	kthread2 = kthread_run(thread_routine, &thr2, "kthread2");
	if (NULL == kthread2) {
		pr_err("%s(): failed to create kthread2", __func__);
		waitqueue_flag = 11;
		wake_up(&wq1);
		mdelay(10);
		goto err_device;
	}

	pr_info("%s(): both threads up and running", __func__);

	return 0;

err_device:
	class_destroy(dev_class);

err_class:
	cdev_del(&hello_chardev_cdev);

err_cdev:
	unregister_chrdev_region(dev, 1);

	return -ENOMEM;
}

void cleanup_hello(void)
{
	pr_info("%s(): called", __func__);
	waitqueue_flag = 11;
	wake_up(&wq1);
	mdelay(10);
	waitqueue_flag = 22;
	wake_up(&wq2);
	mdelay(10);

	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&hello_chardev_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("%s(): READY.\n", __func__);
}

static int __init mod_init(void)
{
	return init_hello();
}

static void __exit mod_exit(void)
{
	cleanup_hello();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Character device driver with event wait queue.");
