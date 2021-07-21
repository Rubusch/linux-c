// hellosyscall.c
/*
  system call "stealing" code snippet - DANGEROUS!

  this (seems to be /) is impossible with a 2.6.18 kernel, 
  thus it just shows a way for doing this. For more information 
  the sources must be studied more deeply!



  Not recommended on 2.6 kernels! If you really want to try this
  module you will have to apply the supplied patch agianst your 
  kernel and recompile it

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  "static" declaration for variables and functions necessary 
  to avoid namespace conflicts with other functions by the same 
  name (in same "common" namespace!).

  init_module(void) and cleanup_module(void) implementations 
  must not have "static" return types!

  C90 conformity: declarations of variables have to be made at 
  begin of each block (a function body is a block!)

  declaration follows the C90 standard

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h> // params to the module
#include <linux/unistd.h> // list of syscalls
#include <linux/sched.h> 
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates syscall stealing");

/*
  the syscall table - a table of functions
  
  we just define it as an external and the kernel will fill 
  it up for us when we are insmod'ed
//*/
extern void *sys_call_table[];

// uid we want to spy on - will be filled from the command line
static int uid;
module_param(uid, int, 0644);

/*
  a function pointer to the original system call

  there are two reasons why we keep this, rather than call the 
  original function (sys_open):

  -> somebody else might have replaced the system call before us
  note that in this case it is not 100% safe, because if another 
  module already replaced sys_open, then when we're inserted we'll 
  call the function in that module - and it might be removed 
  before we are!

  -> another reason for this is that we can't get sys_open, it's
  a static variable, so it is not exported
//*/
asmlinkage int (*original_call) (const char*, int, int);


/*
  anyway the function we'll replace is sys_open (the function called 
  when you call the open system call)

  the exact prototype with the number and type of arguments can be 
  found in fs/open.c
 
  in theory, this means that we're tied to the current version of the 
  kernel 

  in practice, the system calls almost never change (it would wreck 
  havoc and require programs to be recompiled, since the system calls 
  are the interface between the kernel and the processes)
//*/
asmlinkage int our_sys_open(const char* filename, int flags, int mode)
{
  int idx = 0;
  char ch = 0;

  // check if this is the user we're spying on 
  if(uid == current->uid){
    // report the file, if relevant
    printk("opened file by %d: ", uid);
    do{
      get_user(ch, filename + idx);
      ++idx;
      printk("%c", ch);
    }while(ch != 0);
    printk("\n");
  }

  // call the original sys_open - otherwise, we lose the ability to open files
  return original_call(filename, flags, mode);
}


/*
  linux init & clean up
//*/


int init_module(void)
{
  printk(KERN_INFO "I'm dangerous. I hope you did a ");
  printk(KERN_INFO "sync before you insmod'ed me\n");
  printk(KERN_INFO "my counterpart, cleanup_module(), is even");
  printk(KERN_INFO "more dangerous. If\n");
  printk(KERN_INFO "you value your file system, it will ");
  printk(KERN_INFO "be \"sync; rmmod\" \n");
  printk(KERN_INFO "when you remove this module.\n");

  // keep a pointer to the original function in original_call
  original_call = sys_call_table[__NR_open];

  // then replace the system call in the system call table with our_system_call
  sys_call_table[__NR_open] = our_sys_open;

  // to get the address of the function for system call foo, go to sys_call_table[__NR_foo]
  printk(KERN_INFO "spying on UID:%d\n", uid);

  return 0;
}

void cleanup_module(void)
{
  if(sys_call_table[__NR_open] != our_sys_open){
    printk(KERN_ALERT "somebody else also played with the ");
    printk(KERN_ALERT "open system call\n");
    printk(KERN_ALERT "the system may be left in ");
    printk(KERN_ALERT "an unstble state\n");
  }

  sys_call_table[__NR_open] = original_call;
}

