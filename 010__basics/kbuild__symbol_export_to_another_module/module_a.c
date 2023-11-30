// SPDX-License-Identifier: GPL-2.0+

/*
  module_a
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_module_a(void);
void cleanup_module_a(void);

// this function will be exported to other module
void shared_func(void);
EXPORT_SYMBOL(shared_func);

/*
  globals
*/

// this global will be exported to other module
int shared_counter = 0;
EXPORT_SYMBOL(shared_counter);

/*
  implementation
*/

void shared_func(void)
{
	printk(KERN_INFO "%s() has been called!\n", __func__);
	shared_counter++;
}

/*
  start / stop module
*/

int init_module_a(void)
{
	printk(KERN_INFO "%s() initializing..\n", __func__);
	return 0;
}

void cleanup_module_a(void)
{
	printk(KERN_INFO "%s() READY.\n", __func__);
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_module_a();
}

static void __exit mod_exit(void)
{
	cleanup_module_a();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION(
	"Demonstrates sharing functions and variables through symbols.");
