// replacing_printk.c
/*
  CODE NOT TESTED - BASICALLY JUST A SNIPPET!

  This is NOT a kernel module - it's a reimplementation for 
  the print_string() function contained in the kernel sources!!!

  works for the 2.6.x series of the linux kernel!
  there is a difference between 2.6.x == 2.6.6 and 2.6.x > 2.6.6
  This is a naive approach (!), the right way to deal with this is 
  described in section 2 of linux/Documentation/SubmittingPatches

  each process and ressource is treated as TASK

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

  After Peter Jay Salzman (C) 2001
//*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("does something usefull =)");

// functions
static void print_string(char *str)
{
	// tty struct went into signal struct in 2.6.6
	struct tty_struct *my_tty = NULL;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 6))
	// the tty for the current task
	my_tty = current->tty;

#else
	// the tty for the current task, for 2.6.6+ kernels
	my_tty = current->signal->tty;

#endif

	// if my_tty is NULL, the current task has no tty you can print to
	// (ie, if it's a daemon) - if so, there's nothing we can do
	if (my_tty != NULL) {
		/*
      my_tty->driver is a struct which holds the tty's functions,
      one of which (write) is used to write strings to the tty.
      It can be used to take a string either from the user's or 
      kernel's memory segment.

      The function's 1st parameter is the tty to write to,
      because the same function would normally be used for all 
      tty's of a certain type. The 2nd parameter controls whether 
      the function receives a string from kernel memory (false, 0)
      or from user memory (true, non zero).
      BTW: this param has been removed in Kernels > 2.6.9
      The (2nd) 3rd parameter is a pointer to a string.
      The (3rd) 4th parameter is the length of the string.
      
      As you will see below, sometimes it's necessary to use
      preprocessor stuff to create code that works for different
      kernel versions. The (naive) approach we've taken here
      does not scale well. The right way to deal with this is 
      described in section 2 of linux/Documentation/SubmittingPatches
    //*/

		// the tty itself:
		((my_tty->driver)->write)(
			my_tty
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2.6.9))
			,
			0 // don't take the string from userspace!
#endif
			,
			str // string
			,
			strlen(str)); // length

		// unchecked!
	}
}
