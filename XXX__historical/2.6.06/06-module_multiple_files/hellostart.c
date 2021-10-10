// hellostart.c
/*
  linux kernel module (2.6.18)

  demonstrates multiple files in a kernel module
  load it with
  insmod ./multiple.ko

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  "static" declaration for variables and functions necessary 
  to avoid namespace conflicts with other functions by the same 
  name (in same "common" namespace!).

  declaration follows the C90 standard

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/

#include <linux/kernel.h>
#include <linux/module.h>

int init_module(void)
{
	printk(KERN_INFO "Hello World!\n");
	return 0;
}
