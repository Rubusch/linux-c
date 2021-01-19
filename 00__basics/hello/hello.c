// hello.c
/*
  very basic linux kernel module

  Demonstrates the simpliest hello world module for a 2.4 through
  5.4.75 kernel

  When directly implementing the init_module() and cleanup_module()
  no "static" return values allowed!!!

  ---
  References:
  Linux Kernel Module Programming Guide", Peter Jay Salzman, 2007-05-18
*/

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
