// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/module.h>
#include <linux/kernel.h>

int init_module(void)
{
	// NB: printk() is possible, prefer pr_info(), though
	printk(KERN_INFO "Hello World!\n");
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye World!\n");
}

// NB: required declaration in v6.x
MODULE_LICENSE("GPL");
