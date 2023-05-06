/*
  Hello Module

  linux kernel module (2.6.18)

  Demonstrates a kbuild multiple files setup for a kernel module. Load
  it as usual with:
  $ sudo insmod ./multiple.ko

  main.c - The entry point, definition of __init / __exit functions,
  and setup of MODULE_* macros. Additionally the driver header(s) can
  be included.

  Makefile: Set the 'all' target, and basic build environment
  variables, such as KERNELDIR, define a target for building the
  "module" and a target for cleaning up, "clean".

  Kbuild: Define flags, e.g. -O3, defines, etc. in the Kbuild
  file. Specify which .o files, i.e. based on .c files with the same
  name, shall be build. Declare the includes by setting paths to
  "Kbuild.include" files, starting the path from $(src). Finally
  define the main object as obj-... target.

  Kbuild.include: Under the path where there are header files place
  Kbuild.include files, specifying "-include" targets.


  A "static" declaration for variables and functions necessary to
  avoid namespace conflicts with other functions by the same name (in
  same "common" namespace!).

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
*/

#include "hello.h"

/*
  1. Define init and exit functions.
*/
static int __init mod_init(void)
{
	return start_hello();
}

static void __exit mod_exit(void)
{
	cleanup_hello();
}

/*
  2. Register init and exit functions.
*/
module_init(mod_init);
module_exit(mod_exit);

/*
  3. Delcare MODULE_* macros.
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
