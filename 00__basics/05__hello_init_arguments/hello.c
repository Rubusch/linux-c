/*
  Hello Module

  A  linux kernel module, first tested: kernel 2.6.18.

  Demonstrates passing of command line arguments through:
  # insmod hello.ko mystring="foobar" mybyte=255 myintArray=1

  This sets only the first element of the array.

  A "static" declaration for variables and functions necessary to
  avoid namespace conflicts with other functions by the same name (in
  same "common" namespace!).

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
*/
// TODO: check out how to pass array elements e.g. in a list

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>


// 1. set up globals as ".c-file-static" privates
static short int myshort = 1;
static int myint = 420;
static long int mylong = 9999;
static char *mystring = "blah";
static int myintArray[2] = {-1, -1};
static int arr_argc = 0;


/*
  2a. set up a modinfo parameter:

  module_param(name, type, perm);
  MODULE_PARAM_DESC(type, desc);

  name = parameter's name (char*)
  type = datatype (int)
  perm = permissions bit for exposing parameters in the sysfs later on (0000)
  desc = description for modinfo (char*)
*/
module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, " my short integer");

module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(myint, " my integer");

module_param(mylong, long, S_IRUSR);
MODULE_PARM_DESC(mylong, " my long integer");

module_param(mystring, charp, 0000);
MODULE_PARM_DESC(mystring, " my char pointer");


/*
  2b. set up array

  module_param_array(name, type, num, perm)
  MODULE_PARAM_DESC(type, desc);

  name = name of the array
  type = datatype
  num  = number of elements or NULL
  perm = permissions (0000)
  desc = description for modinfo (char*)
*/
module_param_array(myintArray, int, &arr_argc, 0000);
MODULE_PARM_DESC(myintArray, "my array of integers");


/*
  module official stuff
*/
static int __init hello_init(void)
{
	int idx=0;

	printk(KERN_INFO "Hello World\n");

	// 3. use the parameters, here print
	printk(KERN_INFO "myshort is a short integer: %hd\n", myshort);
	printk(KERN_INFO "myint is a integer: %d\n", myint);
	printk(KERN_INFO "mylong is a long integer: %ld\n", mylong);
	printk(KERN_INFO "mystring is a string: %s\n", mystring);

	for (idx=0; idx<(sizeof(myintArray) / sizeof(int)); ++idx) {
		printk(KERN_INFO "myintArray[%d] = %d\n", idx, myintArray[idx]);
	}
	printk(KERN_INFO "got %d arguments for myintArray.\n", arr_argc);

	return 0;
}


static void __exit hello_exit(void)
{
	printk(KERN_INFO "Goodbye World!\n");
}


module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <Lothar.Rubusch@gmx.ch>");
MODULE_DESCRIPTION("usage of init parameters");

