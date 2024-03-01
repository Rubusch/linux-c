// SPDX-License-Identifier: GPL-2.0+
/*
  wait on events using a waitqueue
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>

#define HELLO_DEVICE_NAME "lothars_hello_device"

static struct task_struct *kthread1, *kthread2;
static long int waitqueue_flag = 0;
DECLARE_WAIT_QUEUE_HEAD(wq1); // static creation
static wait_queue_head_t wq2; // prepares dynamic creation

static int thread_routine(void *arg)
{
	int status = -1;
	int sel = *(int *) arg;
	switch (sel) {
	case 1:
		wait_event(wq1, waitqueue_flag == 11);
		pr_info("%s(): waitqueue_flag is %ld\n",
			__func__, waitqueue_flag);
		break;
	case 2:
		do {
			status = wait_event_timeout(wq2, waitqueue_flag == 22,
						    msecs_to_jiffies(5000));
			pr_info("%s(): waitqueue_flag is pending.. timeout\n",
				__func__);
		} while (0 == status);
		break;
	default:
		pr_warn("%s(): should not reach here\n",
			__func__);
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
		pr_err("%s(): faild to read from userspace\n",
		       __func__);
		return -ENOMEM;
	}
	ret = kstrtol(buf, 10, &waitqueue_flag);
	if (-EINVAL == ret) {
		pr_err("%s(): failed to convert value read from userspace\n",
		       __func__);
		return -EINVAL;
	}
	pr_info("%s(): waitqueue_flag is %ld\n", __func__, waitqueue_flag);

	wake_up(&wq1);
	wake_up(&wq2);

	return len;
}

static struct file_operations fops = {
	.write = hello_write,
};

static struct miscdevice waitqueue_dev = {
	.name = HELLO_DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void)
{
	int thr1 = 1, thr2 = 2, ret;

	pr_info("%s() called", __func__);
	init_waitqueue_head(&wq2); // dynamic creation
	ret = misc_register(&waitqueue_dev);
	if (0 != ret) {
		pr_err("%s(): could not register the misc device\n", __func__);
		return ret;
	}

	kthread1 = kthread_run(thread_routine, &thr1, "kthread1");
	if (NULL == kthread1) {
		pr_err("%s(): failed to create kthread1\n", __func__);
		return -EFAULT;
	}

	kthread2 = kthread_run(thread_routine, &thr2, "kthread2");
	if (NULL == kthread2) {
		pr_err("%s(): failed to create kthread2\n", __func__);
		waitqueue_flag = 11;
		wake_up(&wq1);
		mdelay(10);
		return -EFAULT;
	}

	pr_info("%s(): both threads up and running\n", __func__);

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	waitqueue_flag = 11;
	wake_up(&wq1);
	mdelay(10);
	waitqueue_flag = 22;
	wake_up(&wq2);
	mdelay(10);

	misc_deregister(&waitqueue_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Character device driver with event wait queue.");
