/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h> /* kthread_run(), kthread_stop(),... */
#include <linux/delay.h> /* msleep() */
#include <linux/seqlock.h>



/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_atomic(void);
void cleanup_hello_atomic(void);

// thread implementation
int kthread1(void *);
int kthread2(void *);



/*
  globals
*/

// seqlock
seqlock_t lothars_seqlock;

// bitfield for atomic bit operation
unsigned int hello_atomic_bit_check = 0;


// kernelthread objects
#define THREAD1_NAME "thread1"
static struct task_struct *kthread1_task;

#define THREAD2_NAME "thread2"
static struct task_struct *kthread2_task;

// data
unsigned long global_counter = 0;



/*
  implementation
*/

int kthread1(void *pv)
{
	while (!kthread_should_stop()) {
		/**
		 * write_seqlock() - start a seqlock_t write side critical section
		 * @sl: Pointer to seqlock_t
		 *
		 * write_seqlock opens a write side critical section for the given
		 * seqlock_t.  It also implicitly acquires the spinlock_t embedded inside
		 * that sequential lock. All seqlock_t write side sections are thus
		 * automatically serialized and non-preemptible.
		 *
		 * Context: if the seqlock_t read section, or other write side critical
		 * sections, can be invoked from hardirq or softirq contexts, use the
		 * _irqsave or _bh variants of this function instead.
		 */
		write_seqlock(&lothars_seqlock);

		global_counter++;

		/**
		 * write_sequnlock() - end a seqlock_t write side critical section
		 * @sl: Pointer to seqlock_t
		 *
		 * write_sequnlock closes the (serialized and non-preemptible) write side
		 * critical section of given seqlock_t.
		 */
		write_sequnlock(&lothars_seqlock);
		msleep(1000);
	}
	return 0;
}


int kthread2(void *pv)
{
	unsigned int seq_no;
	unsigned long read_value;
	while (!kthread_should_stop()) {
		do {
			/*
			 * For all seqlock_t write side functions, use write_seqcount_*t*_begin()
			 * instead of the generic write_seqcount_begin(). This way, no redundant
			 * lockdep_assert_held() checks are added.
			 */

			/**
			 * read_seqbegin() - start a seqlock_t read side critical section
			 * @sl: Pointer to seqlock_t
			 *
			 * Return: count, to be passed to read_seqretry()
			 */
			seq_no = read_seqbegin(&lothars_seqlock);
			read_value = global_counter;

			/**
			 * read_seqretry() - end a seqlock_t read side section
			 * @sl: Pointer to seqlock_t
			 * @start: count, from read_seqbegin()
			 *
			 * read_seqretry closes the read side critical section of given seqlock_t.
			 * If the critical section was invalid, it must be ignored (and typically
			 * retried).
			 *
			 * Return: true if a read section retry is required, else false
			 */
		} while (read_seqretry(&lothars_seqlock, seq_no));
		printk(KERN_INFO "%s() - read value %lu", __func__, read_value);
		msleep(1000);
	}
	return 0;
}


/*
  start / stop hello module
*/

int init_hello_atomic(void)
{
	printk(KERN_INFO "initializing..\n");

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
MODULE_DESCRIPTION("Demonstrates the usage of seqlock.");
