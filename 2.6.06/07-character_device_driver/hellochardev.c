// hellochardev.c
/*
  linux kernel module (2.6.18)

  usage:
  # mknod /dev/%s c %d 0
  $ echo "hi" > /dev/lothars_char_dev
  $ cat /dev/lothars_char_dev

  it also has an entry in /proc/devices

  Implementation
  1. define a file_operations structure and set the funcions 
  to be used for which operation
  2. define and implement each operation
  for reading from user space to kernel space use put_user

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  "static" declaration for variables and functions necessary 
  to avoid namespace conflicts with other functions by the same 
  name (in same "common" namespace!).

  init_module(void) and exit_module(void) implementations 
  must not have "static" return types!

  C90 conformity: declarations of variables have to be made at 
  begin of each block (a function body is a block!)

  declaration follows the C90 standard

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/
// FIXME: only forced module unload possible!!!

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates the usage of a character device");


/*
  declarations and global variables (header)
//*/


int init_module(void);
void cleanup_module(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

// return value
#define SUCCESS 0

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
  define the used file_operations
//*/
static struct file_operations fops = {
  .read = device_read, 
  .write = device_write,
  .open = device_open,
  .release = device_release
};


/*
  definitions and implementations (source)
//*/

/*
  open the device
  
  returns 0 if ok, else -EBUSY (device not available)
//*/
static int device_open(struct inode* inode, struct file* file)
{  
  static int counter = 0; // C90 conformity and mixed declarations!

  // check if the device is ok
  if(Device_Open) return -EBUSY;

  // increase the usage counter (Device_Open) 
  // and set a static (globally reused in this func) counter
  ++Device_Open;
  counter = 0;
  sprintf(msg, "I already told you %d times Hello World!\n", ++counter);
  msg_Ptr = msg;

  // use the attached device
  try_module_get(THIS_MODULE);

  return SUCCESS;
}


/*
  release the device

  returns 0 if ok
//*/
static int device_release(struct inode* inode, struct file* file)
{
  // ready for next caller - decrement the usage counter
  --Device_Open;

  // remove the device
  module_put(THIS_MODULE);

  return SUCCESS;
}


/*
  read from the device

  reads data from user space into kernel space, therefore:
  MOST INTERESTING FUNCTION!

  returns the number of bytes read from user space
//*/
static ssize_t device_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
  // number of bytes actually written to the buffer
  int bytes_read = 0;

  // if we're at the end of the message return 0 signifying end of file
  if(0 == *msg_Ptr) return 0;

  // actually put the data into the buffer
  while(length && *msg_Ptr){
    /*
      the buffer is in the user data segment, not the kernel segment 
      so "*" assignment won't work. We have to use put_user which copies 
      data from the kernel data segment to the user data segment.
    //*/
    put_user(*(++msg_Ptr), ++buffer);

    // adjust counters
    --length;
    ++bytes_read;
  }

  // most read functions return the number of bytes put into the buffer
  return bytes_read;
}


/*
  called when a process writes to dev file: echo "hi" > /dev/hello

  returns -EINVAL error, the operation isn't implemented - this depends on the device
//*/
static ssize_t device_write(struct file* filp, const char* buff, size_t len, loff_t* off)
{
  printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  return -EINVAL;
}



/*
  linux stuff: init and exit
//*/
int init_module(void)
{
  // register the device to the kernel, by doing this the device gets a major number
  if( 0 > (Major = register_chrdev(0, DEVICE_NAME, &fops))){
    printk(KERN_ALERT "Registering char device failed with %d\n", Major);
    return Major;
  }

  printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
  printk(KERN_INFO "the driver, you now have to create a dev file with\n");
  printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
  printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
  printk(KERN_INFO "the device file.\n");
  printk(KERN_INFO "Remove the device file and module when done.\n");

  return SUCCESS;
}

void cleanup_module(void)
{
  // and unregister the char dev driver and its major number
  int ret;
  if(0 > (ret = unregister_chrdev(Major, DEVICE_NAME))){
    printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
  }
}
