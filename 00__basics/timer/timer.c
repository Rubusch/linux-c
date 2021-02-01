/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello(void);
void cleanup_hello(void);

// timer callback
void timer_callback(struct timer_list *);


/*
  globals
*/

// timer (in milli secs)
#define TIMEOUT 500
static struct timer_list lothars_timer;

// data (demo)
static unsigned int counter = 0;


/*
  implementation
*/

void timer_callback(struct timer_list *data)
{
	// timer work routine
	printk(KERN_INFO "%s() - %d\n", __func__, counter++);

	/*
	  re-enable timer

	  Working as a periodic timer when reenabled here.
	 */
	mod_timer(&lothars_timer, jiffies + msecs_to_jiffies(TIMEOUT));
}


/*
  start / stop module
*/

int init_hello(void)
{
	printk(KERN_INFO "%s() - initializing...\n", __func__);

	// timer setup
	timer_setup(&lothars_timer, timer_callback, 0);

	// setup timer interval
	mod_timer(&lothars_timer, jiffies + msecs_to_jiffies(TIMEOUT));

	return 0;
}

void cleanup_hello(void)
{
	del_timer(&lothars_timer);

	printk(KERN_INFO "%s() - READY.\n", __func__);
}


/*
  init / exit
*/

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
MODULE_DESCRIPTION("Demonstrates kernel timers.");
