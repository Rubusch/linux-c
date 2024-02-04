// SPDX-License-Identifier: GPL-2.0+
#ifndef __HELLO_H
#define __HELLO_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/*
  macros - It is also possible place content for MODULE_* macros in a
           header file.
*/
#define DRIVER_AUTHOR "Lothar Rubusch <l.rubusch@gmail.com>"
#define DRIVER_DESC "messing with distributed files and a kbuild setup"

/*
  forwards - The functions called from static init/exit functions.
*/
int start_hello(void);
void cleanup_hello(void);

#endif
