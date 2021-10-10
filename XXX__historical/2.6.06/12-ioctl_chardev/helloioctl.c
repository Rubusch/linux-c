// helloioctl.c
/*
  creates an input/output character device

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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "helloioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates the usage of ioctl");

#define SUCCESS 0
#define DEVICE_NAME "lothars_char_dev"
#define BUF_SIZ 80

// protoptypes
int init_module(void);
void cleanup_module(void);

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t,
			    loff_t *);
int device_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

// is the device open right now? used to prevent concurent access into the same device
static int Device_used = 0;

// message the device will give when asked
static char Message[BUF_SIZ];

// message is larger than the size of the buffer -> fill in device_read
static char *Ptr_message = NULL;

// a pointer to this is kept in the device table, thus it can't be local (static) to init_module
// unimplemented funcs have NULL
struct file_operations Fops_device = {
	.read = device_read,
	.write = device_write,
	.ioctl = device_ioctl,
	.open = device_open,
	.release = device_release, // same as close
};

/*
  called whenever a process attempts to open the device file
  
  returns 0 if success
//*/
static int device_open(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_open(%p)\n", file);
#endif

	// don't want to talk to 2 processes at the same time
	if (Device_used)
		return -EBUSY;

	++Device_used;
	Ptr_message = Message;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/*
  called to release the device again

  returns 0 if success
//*/
static int device_release(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_release(%p, %p)\n", inode, file);
#endif

	// ready for our next caller
	--Device_used;
	module_put(THIS_MODULE);

	return SUCCESS;
}

/*
  called whenever a process which has already opened the device file, attempts to read form it

  returns the bytes read
//*/
static ssize_t device_read(struct file *file, char __user *buffer,
			   size_t length, loff_t *offset)
{
	// number of bytes actually written to the buffer
	int bytes_read = 0;

#ifdef DEBUG
	printk(KERN_INFO "device_read(%p, %p, %d)\n", file, buffer, length);
#endif

	// if we're at the end of the message, return 0 (eof)
	if (0 == *Ptr_message) {
		return 0;
	}

	// actually put the data into the buffer
	while (length && *Ptr_message) {
		// ATTENTION: data is in the user segment
		put_user(*(++Ptr_message), ++buffer);
		--length;
		++bytes_read;
	}

#ifdef DEBUG
	printk(KERN_INFO "read %d bytes, %d left\n", bytes_read, length);
#endif

	// read functions are supposed to return the number of bytes actually inserted into the buffer
	return bytes_read;
}

/*
  called when someone tries to write into our device file

  returns the number of input characters used
//*/
static ssize_t device_write(struct file *file, const char __user *buffer,
			    size_t length, loff_t *offset)
{
	int idx = 0;

#ifdef DEBUG
	printk(KERN_INFO "device_write(%p, %s, %d)", file, buffer, length);
#endif

	for (idx = 0; (idx < length) && (idx < BUF_SIZ); ++idx) {
		get_user(Message[idx], buffer + idx);
	}
	Ptr_message = Message;

	// return the number of the input characters used
	return idx;
}

/*
  called whenever a process tries to do an ioctl on the device
  ioctl_num - number (id) of the ioctl operations that was called
  ioctl_param - param given to the ioctl
//*/
int device_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num,
		 unsigned long ioctl_param)
{
	int idx = 0;
	char *tmp = NULL;
	char ch = 0;

	// according to the ioctl that was called
	switch (ioctl_num) {
	case IOCTL_SET_MSG:
		// receive a pointer to a message (user space) and set that to be the device's message
		// get the param given to ioctl by the process
		tmp = (char *)ioctl_param;

		// determine length of the message
		get_user(ch, tmp);
		for (idx = 0; ch && (idx < BUF_SIZ); ++idx, ++tmp) {
			get_user(ch, tmp);
		}
		device_write(file, (char *)ioctl_param, idx, 0);
		break;

	case IOCTL_GET_MSG:
		// pass current message to the calling process -
		// the parameter we got is a pointer, fill it
		idx = device_read(file, (char *)ioctl_param, 99, 0);

		// put a zero at the end of the buffer, so it will be properly terminated
		put_user('\0', (char *)ioctl_param + idx);
		break;

	case IOCTL_GET_NTH_BYTE:
		// input (ioctl_param) and output (return value of this function)
		return Message[ioctl_param];
		break;
	}

	return SUCCESS;
}

/*
  linux init & clean up
//*/

int init_module(void)
{
	int ret = 0;

	// register the character device
	if (0 > (ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops_device))) {
		printk(KERN_ALERT
		       "sorry, failed registering the char dev with %d\n",
		       ret);
		return ret;
	}

	printk(KERN_INFO "ok, the major device number is %d\n", MAJOR_NUM);
	printk(KERN_INFO "if you want to talk to the device driver,\n");
	printk(KERN_INFO "you'll have to create a device file, do a:\n");
	printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
	printk(KERN_INFO "the device file name is important, because\n");
	printk(KERN_INFO "the ioctl program assumes that's the\n");
	printk(KERN_INFO "file you'll use.\n");

	return SUCCESS;
}

void cleanup_module(void)
{
	int ret = 0;
	if (0 > (ret = unregister_chrdev(MAJOR_NUM, DEVICE_NAME))) {
		printk(KERN_ALERT "error: unregister_chrdev: %d\n", ret);
	}
	printk(KERN_INFO "character device unregistered\n");
	printk(KERN_INFO "READY.\n");
}
