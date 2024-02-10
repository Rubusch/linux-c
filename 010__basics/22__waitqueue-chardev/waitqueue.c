// SPDX-License-Identifier: GPL-2.0+
/*
  wait on events using a waitqueue

  NB: make sure not to print function arguments of the device
  functions if they are not set, this can make the entire PC hang!
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>

#define HELLO_CLASS_NAME "lothars_hello_class"

// threading
uint32_t read_count = 0;
static struct task_struct *wait_thread;

// wait queue
//*
DECLARE_WAIT_QUEUE_HEAD(chardev_waitqueue);
/*/ // alternatively create a global instance directly
wait_queue_head_t chardev_waitqueue;
// */
int chardev_waitqueue_flag = 0;

static int wait_routine(void *unused)
{
	while (1) {
		printk(KERN_INFO "waiting for event...\n");
		/**
		 * wait_event_interruptible - sleep until a condition gets true
		 * @wq_head: the waitqueue to wait on
		 * @condition: a C expression for the event to wait for
		 *
		 * The process is put to sleep (TASK_INTERRUPTIBLE)
		 * until the @condition evaluates to true or a signal
		 * is received.  The @condition is checked each time
		 * the waitqueue @wq_head is woken up.
		 *
		 * wake_up() has to be called after changing any
		 * variable that could change the result of the wait
		 * condition.
		 *
		 * The function will return -ERESTARTSYS if it was
		 * interrupted by a signal and 0 if @condition
		 * evaluated to true.
		 */
		wait_event_interruptible(chardev_waitqueue,
					 chardev_waitqueue_flag != 0);
		if (2 == chardev_waitqueue_flag) {
			printk(KERN_INFO "event came from EXIT\n");
			return 0;
		}
		printk(KERN_INFO "event came from READ - read count: %d\n",
		       ++read_count);
		chardev_waitqueue_flag = 0;
	}
	do_exit(0);
	return 0;
}

static ssize_t hello_read(struct file *file, char __user *buf, size_t len,
			  loff_t *poff)
{
	printk(KERN_INFO "%s()\n", __func__);
	chardev_waitqueue_flag = 1;
	/**
	 * Wakeup macros to be used to report events to the targets.
	 */
	wake_up_interruptible(&chardev_waitqueue);
	return 0;
}

static ssize_t hello_write(struct file *file, const char __user *buf,
			   size_t len, loff_t *poff)
{
	printk(KERN_INFO "%s()\n", __func__);
	return len; // if this is 0, it will spin around the "write"
}

static struct file_operations fops = {
	.read = hello_read,
	.write = hello_write,
};

static struct miscdevice waitqueue_dev = {
	.name = HELLO_CLASS_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void)
{
	int ret;

	printk(KERN_INFO "%s() initializing\n", __func__);
	ret = misc_register(&waitqueue_dev);
	if (0 != ret) {
		pr_err("%s(): could not register miscdevice\n", __func__);
		return -EFAULT;
	}

	// initialize wait queue
	init_waitqueue_head(&chardev_waitqueue);

	// start kernel thread
	wait_thread = kthread_create(wait_routine, NULL, "WaitThread");
	if (wait_thread) {
		printk(KERN_INFO "thread created\n");
		wake_up_process(wait_thread);
	} else {
		printk(KERN_ERR "thread creation failed\n");
		return -EFAULT;
	}

	printk(KERN_INFO "%s() done.\n", __func__);
	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);

	chardev_waitqueue_flag = 2;
	/**
	 * Wakeup macros to be used to report events to the targets.
	 */
	wake_up_interruptible(&chardev_waitqueue);

	misc_deregister(&waitqueue_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Character device driver with event wait queue.");
