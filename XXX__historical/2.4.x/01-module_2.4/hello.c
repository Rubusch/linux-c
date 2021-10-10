// hello.c
/*
  linux kernel module, 2.6.18

  demonstrates the simpliest hello world module for a 2.4 kernel

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  when directly implementing the init_module() and cleanup_module()
  no "static" return values allowed!!!

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/

#include <linux/module.h>
#include <linux/kernel.h>

int init_module(void)
{
	printk(KERN_INFO "Hello World!\n");
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye World!\n");
}
