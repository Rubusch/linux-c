/*
  module_b
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_module_b(void);
void cleanup_module_b(void);

// function defined in module_a
void shared_func(void);

/*
  globals
*/

// global, defined in module_a
extern int shared_counter;

/*
  implementation
*/

/*
  start / stop module
*/

int init_module_b(void)
{
	printk(KERN_INFO "%s() initializing..\n", __func__);
	shared_func();
	return 0;
}

void cleanup_module_b(void)
{
	printk(KERN_INFO "%s() READY.\n", __func__);
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_module_b();
}

static void __exit mod_exit(void)
{
	cleanup_module_b();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION(
	"Demonstrates sharing functions and variables through symbols.");
