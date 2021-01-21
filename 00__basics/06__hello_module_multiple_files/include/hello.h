#ifndef __HELLO_H
#define __HELLO_H


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


/*
  macros - It is also possible place content for MODULE_* macros in a
           header file.
*/
#define DRIVER_AUTHOR  "Lothar Rubusch <Lothar.Rubusch@gmx.ch>"
#define DRIVER_DESC    "Distributed files kbuild setup"


/*
  forwards - The functions called from static init/exit functions.
*/
int start_hello(void);
void cleanup_hello(void);


#endif
