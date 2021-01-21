/*
  Hello Module

  A basic linux kernel module, 2.6.18. It demonstrates modinfo
  support, author, tainting (GPL License), description showed by:
  # modinfo hello


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

#define DRIVER_AUTHOR   "Lothar Rubusch <Lothar.Rubusch@gmx.ch>"
#define DRIVER_DESC     "a sample module"


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
