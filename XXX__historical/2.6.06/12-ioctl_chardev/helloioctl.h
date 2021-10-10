// helloioctl.h
/*
  the declarations here have to be in a header file, because
  they need to be known both to the kernel module (in chardev.c)
  and the process calling ioctl (ioctl.c)

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

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/

#ifndef HELLO_IOCTL_H
#define HELLO_IOCTL_H

#include <linux/kernel.h>

/*
  the major device number - we can't rely on dynamic registration 
  anymore, because ioctls need to know it!
//*/
#define MAJOR_NUM 100

/*
  set the message of the device driver

  _IOR means:
  creating an ioctl command number for passing information from a 
  user process to the kernel module

  MAJOR_NUM - is the major device number

  0 - is the number of the command (there could be several with 
  different meanings)

  char* - type we want to get from the process to the kernel
//*/
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)

/*
  get message of the device driver

  this IOCTL is used for output, to get the message of the device driver
  
  however, we still need the buffer to place the message in to be input, 
  as it is allocated by the process
//*/
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)

/*
  get the n'th byte of the message

  this IOCTL is used for both input and output
  
  it receives from the user a number, n, and returns "message[n]"
//*/
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)

/*
  name of the device file
//*/
#define DEVICE_FILE_NAME "lothars_char_dev"

#endif
