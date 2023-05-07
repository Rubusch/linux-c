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
#include <linux/fs.h> /* register_chrdev_region(), unregister_chrdev_region() */
#include <linux/cdev.h> /* cdev_add(), cdev_del(),... */
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

#ifdef __KERNEL__

/*
  forwards
*/
int init_hello_ioctl(void);
void cleanup_hello_ioctl(void);

#endif

/*
  the major device number - we can't rely on dynamic registration
  anymore, because ioctls need to know it!
*/
#define MAJOR_NUM 100

/*
  name of the device file
*/
#define HELLO_DEVICE_FILENAME "lothars_chardev"
#define HELLO_CLASS_NAME "lothars_chardev_class"
#define HELLO_DEVICE_NAME "lothars_chardev_device"

#endif
