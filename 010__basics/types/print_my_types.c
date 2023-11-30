// SPDX-License-Identifier: GPL-2.0+
/*
 */

#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/init.h>

//#include <linux/types.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int show_types(void);

/*
  globals
*/

#define DRIVER_AUTHOR "Lothar Rubusch <l.rubusch@gmail.com>"
#define DRIVER_DESC "messing with sample modules"

/*
  implementation
*/

int show_types(void)
{
	pr_info("--- types ---\n");
	{
		int type_size = sizeof(unsigned short);
		pr_info("unsigned short [%d]: %u -> %u\n", type_size, 0, USHRT_MAX);
		pr_info("short [%d]: %d -> %d\n", type_size, SHRT_MIN, SHRT_MAX);
	}

	{
		int type_size = sizeof(unsigned int);
		pr_info("unsigned int [%d]: %u -> %u\n", type_size, 0, UINT_MAX);
		pr_info("int [%d]: %d -> %d\n", type_size, INT_MIN, INT_MAX);
	}

	{
		int type_size = sizeof(unsigned long);
		pr_info("unsigned long [%d]: %lu -> %lu\n", type_size, (long)0, ULONG_MAX);
		pr_info("long [%d]: %ld -> %ld\n", type_size, LONG_MIN, LONG_MAX);
	}
	{
		int type_size = sizeof(unsigned long long);
		pr_info("unsigned long long [%d]: %llu -> %llu\n", type_size, (long long)0, ULLONG_MAX);
		pr_info("long long [%d]: %lld -> %lld\n", type_size, LLONG_MIN, LLONG_MAX);
	}

	pr_info("unsigned int pointer max: %lu\n", (long) UINTPTR_MAX);
	return 0;
}

/*
  static init / exit
*/
static int __init mod_init(void)
{
	return show_types();
}

static void __exit mod_exit(void)
{
	pr_info("=== types done ===\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
