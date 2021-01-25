/*
*/

#include "helloioctl.h"

static int __init mod_init(void)
{
	return init_hello_ioctl();
}

static void __exit mod_exit(void)
{
	cleanup_hello_ioctl();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@gmx.ch>");
MODULE_DESCRIPTION("Demonstrates the usage of ioctl.");
