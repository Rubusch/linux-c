/*
  Hello Module

  A basic linux kernel module, 2.6.18.

  A "static" declaration for variables and functions is necessary to
  avoid namespace conflicts with other functions by the same name (in
  same "common" namespace!).

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init init_hello(void)
{
	printk(KERN_INFO "Hello World!\n");
	return 0;
}

static void __exit exit_hello(void)
{
	printk(KERN_INFO "Goodbye World!\n");
}

module_init(init_hello);
module_exit(exit_hello);
