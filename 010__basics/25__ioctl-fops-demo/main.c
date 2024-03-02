// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include "happy_ioctl.h"

static int __init mod_init(void)
{
	return init_happy_ioctl();
}

static void __exit mod_exit(void)
{
	cleanup_happy_ioctl();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates the usage of ioctl.");
