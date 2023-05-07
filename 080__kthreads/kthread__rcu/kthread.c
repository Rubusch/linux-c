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

// kthread routines
int kthread1(void *);
int kthread2(void *);

/*
  globals
*/

#define THREAD1_NAME "thread1"
#define THREAD2_NAME "thread2"

// kthread tasks
static struct task_struct *kthread1_task;
static struct task_struct *kthread2_task;

// define a spinlock instead of DEFINE_RWLOCK() here, for the write_lock/unlock()
DEFINE_SPINLOCK(lothars_lock);

// data
unsigned long global_counter = 0;

/*
  implementation
*/

/*
  thread #1
*/
int kthread1(void *pv)
{
	while (!kthread_should_stop()) {
		/**
		 * using rcu use
		 *    spin_lock(&lock) and spin_unlock(&lock)
		 * instead of
		 *    write_lock(&lock) and write_unlock(&lock)
		 *
		 * working with lists and rcu, use list_del_rcu()
		 * instead of list_del()
		 */
		spin_lock(&lothars_lock);
		global_counter++;
		printk(KERN_INFO "%s: counter++\n", THREAD1_NAME);

		// use spin_unlock() and synchronize_rcu() instead of write_unlock()
		spin_unlock(&lothars_lock);
		synchronize_rcu();

		msleep(1000);
	}
	return 0;
}

/*
  thread #2
*/
int kthread2(void *pv)
{
	while (!kthread_should_stop()) {
		/**
		 * in case of working with list, with rcu use
		 *     list_for_each_entry_rcu(...);
		 * instead of
		 *     list_for_each_entry(...);
		 *
		 * rcu_read_lock() and rcu_read_unlock() don't need
		 * arguments, being a lock-free threading mechanism
		 * for syncing
		 */
		rcu_read_lock();
		printk(KERN_INFO "%s: read counter = %lu\n", THREAD2_NAME,
		       global_counter);

		rcu_read_unlock();
		msleep(1000);
	}
	return 0;
}

int init_hello_kernelthread(void)
{
	printk(KERN_INFO "%s() started\n", __func__);

	/**
	 * kthread_run - create and wake a thread.
	 * @threadfn: the function to run until signal_pending(current).
	 * @data: data ptr for @threadfn.
	 * @namefmt: printf-style name for the thread.
	 *
	 * Description: Convenient wrapper for kthread_create() followed by
	 * wake_up_process().  Returns the kthread or ERR_PTR(-ENOMEM).
	 */
	kthread1_task = kthread_run(kthread1, NULL, THREAD1_NAME);
	if (!kthread1_task) {
		printk(KERN_ERR "%s creation failed\n", THREAD1_NAME);
		return -1;
	}
	kthread2_task = kthread_run(kthread2, NULL, THREAD2_NAME);
	if (!kthread2_task) {
		printk(KERN_ERR "%s creation failed\n", THREAD2_NAME);
		goto err_thread1;
	}

	printk(KERN_INFO "%s() kernelthread initialized\n", __func__);

	return 0;

err_thread1:
	kthread_stop(kthread1_task);

	return -1;
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
	kthread_stop(kthread1_task);
	kthread_stop(kthread2_task);

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
MODULE_DESCRIPTION("Demonstrates elementary usage of rcu.");
