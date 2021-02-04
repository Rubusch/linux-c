/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h> /* kthread_run(), kthread_stop(),... */
#include <linux/delay.h> /* msleep() */


/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_atomic(void);
void cleanup_hello_atomic(void);

// thread implementation
int kthread_routine(const char *);
int kthread1(void *);
int kthread2(void *);


/*
  globals
*/

// variable for atomic increment operation
atomic_t hello_atomic_global = ATOMIC_INIT(0);

// bitfield for atomic bit operation
unsigned int hello_atomic_bit_check = 0;


// kernelthread objects
#define THREAD1_NAME "thread1"
static struct task_struct *kthread1_task;

#define THREAD2_NAME "thread2"
static struct task_struct *kthread2_task;


/*
  implementation
*/

int kthread_routine(const char *thread_name)
{
	unsigned int prev = 0;

	/**
	 * kthread_should_stop - should this kthread return now?
	 *
	 * When someone calls kthread_stop() on your kthread, it will be woken
	 * and this will return true.  You should then return, and your return
	 * value will be passed through to kthread_stop().
	 */
	while (!kthread_should_stop()) {

		/**
		 * atomic_inc - increment atomic variable
		 * @v: pointer of type atomic_t
		 *
		 * Atomically increments @v by 1.
		 */
		atomic_inc(&hello_atomic_global);

		/**
		 * test_and_change_bit - toggle a bit and return its old value
		 * @nr:  bit number to set
		 * @addr:  pointer to memory
		 */
		prev = test_and_change_bit(1, (void*) &hello_atomic_bit_check);

		printk(KERN_INFO "%s [value: %u] [bit: %u]\n", thread_name, atomic_read(&hello_atomic_global), prev);
		msleep(1000);
	}
	return 0;
}


int kthread1(void *pv)
{
	return kthread_routine(THREAD1_NAME);
}


int kthread2(void *pv)
{
	return kthread_routine(THREAD2_NAME);
}


/*
  start / stop hello module
*/

int init_hello_atomic(void)
{
	printk(KERN_INFO "initializing..\n");

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
		printk(KERN_ERR "first kthread creation failed\n");
		goto err_thread1;
	}
	printk(KERN_INFO "%s created\n", THREAD1_NAME);

	kthread2_task = kthread_run(kthread2, NULL, THREAD2_NAME);
	if (!kthread2_task) {
		printk(KERN_ERR "second kthread creation failed\n");
		goto err_thread2;
	}
	printk(KERN_INFO "%s created\n", THREAD2_NAME);

	return 0;

err_thread2:
	kthread_stop(kthread1_task);

err_thread1:
	return -ENOMEM;
}


void cleanup_hello_atomic(void)
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
	printk(KERN_INFO "READY.\n");
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_hello_atomic();
}

static void __exit mod_exit(void)
{
	cleanup_hello_atomic();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates thread operations on atomic variables.");
