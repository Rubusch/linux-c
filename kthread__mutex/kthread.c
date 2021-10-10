/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h> /* kthread_run(), kthread_create() */
#include <linux/delay.h> /* msleep() */
#include <linux/mutex.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_kernelthread(void);
void cleanup_hello_kernelthread(void);

// kthread routines
int kthread_routine(const char *);
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

// mutex
struct mutex lothars_mutex;

// data
unsigned long global_counter = 0;

/*
  implementation
*/

int kthread_routine(const char *thread_name)
{
	/**
	 * kthread_should_stop() - should this kthread return now?
	 *
	 * When someone calls kthread_stop() on your kthread, it will be woken
	 * and this will return true.  You should then return, and your return
	 * value will be passed through to kthread_stop().
	 */
	while (!kthread_should_stop()) {
		/**
		 * mutex_lock - acquire the mutex
		 * @lock: the mutex to be acquired
		 *
		 * Lock the mutex exclusively for this task. If the mutex is not
		 * available right now, it will sleep until it can get it.
		 *
		 * The mutex must later on be released by the same task that
		 * acquired it. Recursive locking is not allowed. The task
		 * may not exit without first unlocking the mutex. Also, kernel
		 * memory where the mutex resides must not be freed with
		 * the mutex still locked. The mutex must first be initialized
		 * (or statically defined) before it can be locked. memset()-ing
		 * the mutex to 0 is not allowed.
		 *
		 * (The CONFIG_DEBUG_MUTEXES .config option turns on debugging
		 * checks that will enforce the restrictions and will also do
		 * deadlock debugging)
		 *
		 * This function is similar to (but not equivalent to) down().
		 */
		mutex_lock(&lothars_mutex);
		global_counter++;
		printk(KERN_INFO "%s - counter = %lu\n", thread_name,
		       global_counter);
		/**
		 * mutex_unlock - release the mutex
		 * @lock: the mutex to be released
		 *
		 * Unlock a mutex that has been locked by this task previously.
		 *
		 * This function must not be used in interrupt context. Unlocking
		 * of a not locked mutex is not allowed.
		 *
		 * This function is similar to (but not equivalent to) up().
		 */
		mutex_unlock(&lothars_mutex);
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

int init_hello_kernelthread(void)
{
	printk(KERN_INFO "%s() started\n", __func__);

	/**
	 * mutex_init - initialize the mutex
	 * @mutex: the mutex to be initialized
	 *
	 * Initialize the mutex to unlocked state.
	 *
	 * It is not allowed to initialize an already locked mutex.
	 */
	mutex_init(&lothars_mutex);

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
MODULE_DESCRIPTION("Demonstrates elementary usage of mutex.");
