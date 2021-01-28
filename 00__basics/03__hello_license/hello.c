/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DRIVER_AUTHOR   "Lothar Rubusch <l.rubusch@gmail.com>"
#define DRIVER_DESC     "A sample module."


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

/*
  modinfo
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

/*
  uses the /dev/lothar_device
*/
MODULE_SUPPORTED_DEVICE("lothar_device");
