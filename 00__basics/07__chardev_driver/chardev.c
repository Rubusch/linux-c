/*
  Char Dev Module

  Linux kernel module (2.6.18)

  Usage:
  $ make
  $ sudo insmod chardev.ko
  $ dmesg
      (...)
      [198458.510364] I was assigned major number 244.
      [198458.510365] To talk to the device, create a dev file with
      [198458.510366] 'sudo mknod /dev/lothars_char_dev c 244 0'
      (...)
  $ sudo mknod /dev/lothars_char_dev c 244 0

// FIXME: /dev/lothars_char_dev not writeable                  
  $ echo "hi" > /dev/lothars_char_dev
  $ cat /dev/lothars_char_dev

  Additionally it creates an entry in /proc/devices


  Implementation

  1. Define the specific functions and define a file_operations
  structure and set the funcions to be used for which operation.

  2. Define and implement each operation for reading from user space
  to kernel space use put_user

  NB: Init_module(void) and exit_module(void) implementations must not
  have "static" return types!

  C90 conformity: declarations of variables have to be made at begin
  of each block (a function body is a block!)

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>



/*
  forwards
*/

int start_hello(void);
void stop_hello(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);


/*
  globals (usually better placed in a .h file)
*/

// dev name as it appears in /proc/devices
#define DEVICE_NAME "lothars_char_dev"

// max length of the messag from the device
#define BUF_LEN 80

// major number assigned to our device driver
static int Major;

// is device open? Used to prevent multiple access to device
static int Device_Open = 0;

// the message the device will give when asked
static char msg[BUF_LEN];

// pointer to write for sending messages
static char* msg_Ptr;

/*
  Define the used file_operations fops.
*/
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};


/*
  Definitions and implementations (source)
*/

/*
  Open the device.

  Returns 0 if ok, else -EBUSY (device not available).
*/
static int device_open(struct inode* inode, struct file* file)
{
	static int counter = 0; // C90 conformity and mixed declarations!

	// check if the device is ok
	if (Device_Open) return -EBUSY;

	// increase the usage counter (Device_Open)
	// and set a static (globally reused in this func) counter
	++Device_Open;
	counter = 0;
	sprintf(msg, "I already told you %d times Hello World!\n", ++counter);
	msg_Ptr = msg;

	// use the attached device
	try_module_get(THIS_MODULE);

	return 0;
}


/*
  Release the device.

  Returns 0 if ok.
*/
static int device_release(struct inode* inode, struct file* file)
{
	// ready for next caller - decrement the usage counter
	--Device_Open;

	// remove the device
	module_put(THIS_MODULE);

	return 0;
}


/*
  Read from the device.

  Reads data from user space into kernel space.

  Returns the number of bytes read from user space.
*/
static ssize_t device_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
	// number of bytes actually written to the buffer
	int bytes_read = 0;

	// if we're at the end of the message return 0 signifying end of file
	if (0 == *msg_Ptr) return 0;

	// actually put the data into the buffer
	while (length && *msg_Ptr) {
		/*
		  The buffer is in the user data segment, not the
		  kernel segment so "*" assignment won't work. We have
		  to use put_user which copies data from the kernel
		  data segment to the user data segment.
		*/
		put_user(*(++msg_Ptr), ++buffer);

		// adjust counters
		--length;
		++bytes_read;
	}

	// most read functions return the number of bytes put into the buffer
	return bytes_read;
}


/*
  Called when a process writes to dev file: echo "hi" > /dev/hello

  Returns -EINVAL error, the operation isn't implemented - this
  depends on the device.
*/
static ssize_t device_write(struct file* filp, const char* buff, size_t len, loff_t* off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}



/*
  linux stuff: init and exit
*/
int start_hello(void)
{
	// register the device to the kernel, by doing this the device gets a major number
	if (0 > (Major = register_chrdev(0, DEVICE_NAME, &fops))) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "I was assigned major number %d.\n", Major);
	printk(KERN_INFO "To talk to the device, create a dev file with\n");
	printk(KERN_INFO "'sudo mknod /dev/%s c %d 0'\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return 0;
}

void stop_hello(void)
{
	// and unregister the char dev driver and its major number
	unregister_chrdev(Major, DEVICE_NAME);
}



static int __init mod_init(void)
{
	return start_hello();
}

static void __exit mod_exit(void)
{
	stop_hello();
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates the usage of a character device");
