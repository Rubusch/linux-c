#include <linux/module.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>

// initialize module (and interrupt)
static int __init skeleton_init_module (void) {
  printk("initializing module\n");
  
  return 0;
}

// close and cleanup module
static void __exit skeleton_cleanup_module (void) {
  int* bla=NULL;
  printk("cleaning up module and accessing null pointer: %d \n", *bla);
}

module_init(skeleton_init_module);
module_exit(skeleton_cleanup_module);
MODULE_AUTHOR("fabian.meier");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Device Driver to provoce oooooops");
