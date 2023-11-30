// SPDX-License-Identifier: GPL-2.0+
/*
  The declarations here have to be in a header file, because they need
  to be known both to the kernel module (in chardev.c) and the process
  calling ioctl (ioctl.c).
*/

#ifndef HELLO_IOCTL_H
#define HELLO_IOCTL_H

#ifdef __KERNEL__

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> /* kmalloc() */
#include <linux/uaccess.h> /* copy_from_user(), copy_to_user() */
#include <linux/ioctl.h>

#else

#include <sys/ioctl.h>

#endif

#define SUCCESS 0

/*
#define "ioctl name" __IOX("magic number","command number","argument type")

  where IOX can be :
    "IO": an ioctl with no parameters
    "IOW": an ioctl with write parameters (copy_from_user)
    "IOR": an ioctl with read parameters (copy_to_user)
    "IOWR": an ioctl with both write and read parameters

  The Magic Number is a unique number or character that will
  differentiate our set of ioctl calls from the other ioctl
  calls. some times the major number for the device is used here.

  Command Number is the number that is assigned to the ioctl. This is
  used to differentiate the commands from one another.

  The last is the type of data.
*/
#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)

#define BUF_SIZ 80

// protoptypes
int init_hello_ioctl(void);
void cleanup_hello_ioctl(void);

/*
  the major device number - we can't rely on dynamic registration
  anymore, because ioctls need to know it!
*/
#define MAJOR_NUM 100

/*
  set the message of the device driver

  _IOR means:
  creating an ioctl command number for passing information from a user
  process to the kernel module

  MAJOR_NUM - is the major device number

  0 - is the number of the command (there could be several with
  different meanings)

  char* - type we want to get from the process to the kernel
*/
//#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char*)

/*
  get message of the device driver

  this IOCTL is used for output, to get the message of the device
  driver

  however, we still need the buffer to place the message in to be
  input, as it is allocated by the process
*/
//#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char*)

/*
  get the n'th byte of the message

  this IOCTL is used for both input and output

  it receives from the user a number, n, and returns "message[n]"
*/
//#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)

/*
  name of the device file
*/
#define HELLO_DEVICE_FILENAME "lothars_chardev"
#define HELLO_CLASS_NAME "lothars_chardev_class"
#define HELLO_DEVICE_NAME "lothars_chardev_device"

#endif
