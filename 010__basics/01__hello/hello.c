// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello(void)
{
	// NB: Always use '\n' to flush the message, w/o the CR/LF the string
	// might be displayed later!
	pr_info("Hello World!\n");
	return 0;
}

void cleanup_hello(void)
{
	pr_info("Goodbye World!\n");
}

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
MODULE_DESCRIPTION("messing with modules");
