// SPDX-License-Identifier: GPL-2.0+
/*
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
