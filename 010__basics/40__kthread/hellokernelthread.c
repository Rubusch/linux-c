// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h> /* kthread_run(), kthread_create() */
#include <linux/delay.h> /* msleep() */

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_kernelthread(void);
void cleanup_hello_kernelthread(void);

int kernelthread_routine(void *);

/*
  globals
*/

#define KERNELTHREAD_NAME "lothars_kernelthread"

// pointer to kernelthread
static struct task_struct *hello_kernelthread;

/*
  implementation
*/

/*
  Some thread worker routine.
*/
int kernelthread_routine(void *pv)
{
	int idx = 0;

	/**
	 * kthread_should_stop() - should this kthread return now?
	 *
	 * When someone calls kthread_stop() on your kthread, it will be woken
	 * and this will return true.  You should then return, and your return
	 * value will be passed through to kthread_stop().
	 */
	while (!kthread_should_stop()) {
		printk(KERN_INFO "%s() counting: %d\n", __func__, idx++);
		msleep(1000);
	}
	return 0;
}

int init_hello_kernelthread(void)
{
	printk(KERN_INFO "%s() started\n", __func__);

#if 0 /* either, first create kernelthread */
	/**
	 * kthread_create() - create a kthread on the current node
	 * @threadfn: the function to run in the thread
	 * @data: data pointer for @threadfn()
	 * @namefmt: printf-style format string for the thread name
	 * @arg...: arguments for @namefmt.
	 *
	 * This macro will create a kthread on the current node, leaving it in
	 * the stopped state.  This is just a helper for kthread_create_on_node();
	 * see the documentation there for more details.
	 */
	hello_kernelthread = kthread_create(kernelthread_routine, NULL, KERNELTHREAD_NAME);
	if (NULL == hello_kernelthread) {
		printk(KERN_ALERT "kthread_create() failed.");
		return -ENOMEM;
	}

	// and start kernelthread later
	/**
	 * wake_up_process - Wake up a specific process
	 * @p: The process to be woken up.
	 *
	 * Attempt to wake up the nominated process and move it to the set of runnable
	 * processes.
	 *
	 * Return: 1 if the process was woken up, 0 if it was already running.
	 *
	 * This function executes a full memory barrier before accessing the task state.
	 */
	wake_up_process(hello_kernelthread);

#else /* alternatively, create and start kernelthread at once */

	/**
	 * kthread_run - create and wake a thread.
	 * @threadfn: the function to run until signal_pending(current).
	 * @data: data ptr for @threadfn.
	 * @namefmt: printf-style name for the thread.
	 *
	 * Description: Convenient wrapper for kthread_create() followed by
	 * wake_up_process().  Returns the kthread or ERR_PTR(-ENOMEM).
	 */
	hello_kernelthread =
		kthread_run(kernelthread_routine, NULL, KERNELTHREAD_NAME);
	if (NULL == hello_kernelthread) {
		printk(KERN_ALERT "kthread_create() failed.");
		return -ENOMEM;
	}
#endif

	printk(KERN_INFO "%s() kernelthread initialized\n", __func__);

	return 0;
}

void cleanup_hello_kernelthread(void)
{
	/**
	 * kthread_stop - stop a thread created by kthread_create().
	 * @k: thread created by kthread_create().
	 *
	 * Sets kthread_should_stop() for @k to return true, wakes it, and
	 * waits for it to exit. This can also be called after kthread_create()
	 * instead of calling wake_up_process(): the thread will exit without
	 * calling threadfn().
	 *
	 * If threadfn() may call do_exit() itself, the caller must ensure
	 * task_struct can't go away.
	 *
	 * Returns the result of threadfn(), or %-EINTR if wake_up_process()
	 * was never called.
	 */
	kthread_stop(hello_kernelthread);
	printk("%s() READY.\n", __func__);
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_hello_kernelthread();
}

static void __exit mod_exit(void)
{
	cleanup_hello_kernelthread();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates the start and stop of a kernelthread.");
