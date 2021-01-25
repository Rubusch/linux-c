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
int kernelthread_routine(void* pv)
{
	int idx=0;
	while (!kthread_should_stop()) {
		printk(KERN_INFO "%s() counting: %d\n", __func__, idx++);
		msleep(1000);
	}
	return 0;
}


int init_hello_kernelthread(void)
{
	printk(KERN_INFO "%s() started\n", __func__);

//*
	// first, create kernelthread
	hello_kernelthread = kthread_create(kernelthread_routine, NULL, KERNELTHREAD_NAME);
	if (NULL == hello_kernelthread) {
		printk(KERN_ALERT "kthread_create() failed.");
		return -ENOMEM;
	}

	// and start kernelthread later
	wake_up_process(hello_kernelthread);
/*/
// alternatively
	// create and start kernelthread at once
	hello_kernelthread = kthread_run(kernelthread_routine, NULL, KERNELTHREAD_NAME);
	if (NULL == hello_kernelthread) {
		printk(KERN_ALERT "kthread_create() failed.");
		return -ENOMEM;
	}
// */
	printk(KERN_INFO "%s() kernelthread initialized\n", __func__);

	return 0;
}


void cleanup_hello_kernelthread(void)
{
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
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@gmx.ch>");
MODULE_DESCRIPTION("Demonstrates the start and stop of a kernelthread.");
