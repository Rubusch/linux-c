// hello.c
/*
  linux kernel module, kernel: 2.6.18-6

  demonstrates "initdata" method to set global data for a module

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  "static" declaration for variables and functions necessary 
  to avoid namespace conflicts with other functions by the same 
  name (in same "common" namespace!).

  C90 conform

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/*
  modinfo stuff
//*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstration of initdata");

/*
  uses the /dev/lothar_device
//*/
MODULE_SUPPORTED_DEVICE("lothar_device");


static int hello_data __initdata = 3;


static int __init init_hello()
{
  printk(KERN_INFO "Hello World!\n");
  return 0;
}


static void __exit exit_hello()
{
  printk(KERN_INFO "Goodbye World!\n");
}

module_init(init_hello);
module_exit(exit_hello);
