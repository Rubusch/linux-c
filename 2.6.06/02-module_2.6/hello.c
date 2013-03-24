// hello.c
/*
  linux kernel module, 2.6.18

  demonstrates the simpliest kernel module for a 2.6 kernel

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  "static" declaration for variables and functions necessary 
  to avoid namespace conflicts with other functions by the same 
  name (in same "common" namespace!).

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

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
