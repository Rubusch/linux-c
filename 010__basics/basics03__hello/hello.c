/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello(void);
void cleanup_hello(void);

/*
  globals
*/

#define DRIVER_AUTHOR "Lothar Rubusch <l.rubusch@gmail.com>"
#define DRIVER_DESC "messing with sample modules"

/*
  implementation
*/

///

/*
  non-static start / stop module functions
*/

int init_hello(void)
{
	pr_info("Hello World!\n");
	return 0;
}

void cleanup_hello(void)
{
	pr_info("Goodbye World!\n");
}

/*
  static init / exit
*/

static int __init mod_init(void)
{
	return init_hello();
}

static void __exit mod_exit(void)
{
	cleanup_hello();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
