// SPDX-License-Identifier: GPL-2.0+
/*
  Hello IOCTL

  A loadable kernelmodule and a corresponding userspace application to
  show the usage of ioctl mechanisms.

  The ioctl implementation needs the following steps:
  - Create IOCTL command in driver
  - Write IOCTL function in the driver
  - Create IOCTL command in a Userspace application
  - Use the IOCTL system call in a Userspace

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
  Highly inspired by / many thanks to embetronicx.com (2021) - https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
*/

#include "helloioctl.h"

static int __init mod_init(void)
{
	return init_hello_ioctl();
}

static void __exit mod_exit(void)
{
	cleanup_hello_ioctl();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("demonstrates the usage of ioctl");
