// helloprocfs.c
/*
  creates a file in the procfile system

  reads and writes to the procfile, which can be read out or 
  written from the user site, by:
  cat /proc/lothars_proc_file
  echo "foobar" > /proc/lothars_proc_file

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
#include <linux/proc_fs.h> // procfs
#include <asm/uaccess.h> // copy_*_user

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates the usage of a procfs entry");

#define PROC_ENTRY_FILENAME  "lothars_proc_entry"
#define PROCFS_MAX_SIZE 2048

int init_module(void);
void cleanup_module(void);
static ssize_t procfs_read(struct file*, char*, size_t, loff_t*);
static ssize_t procfs_write(struct file*, const char*, size_t, loff_t*);
static int module_permission(struct inode*, int, struct nameidata*);
static int module_permission(struct inode*, int, struct nameidata*);
int procfs_open(struct inode*, struct file*);
int procfs_close(struct inode*, struct file*);

// buffer (2k) for this module
static char procfs_buffer[PROCFS_MAX_SIZE];

// size of the data hold in the buffer
static unsigned long procfs_buffer_size = 0;

// structure keeping information about the /proc file
static struct proc_dir_entry *my_proc_file;

// the proc file file operations
static struct file_operations my_proc_file_fops = {
  .read    = procfs_read,
  .write   = procfs_write,
  .open    = procfs_open,
  .release = procfs_close,
};

// the proc file inode operations
static struct inode_operations my_proc_file_iops = {
  .permission = module_permission,
};


/*
  read proc file
 
  prefer ssize_t instead of int!
  
  returns the number of bytes read from proc
//*/
static ssize_t procfs_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
  static int finished = 0;
  
  // return 0 when finished
  if(finished){
    printk(KERN_INFO "procfs_read: END\n");
    finished = 0;
    return 0;
  }
  
  finished = 1;

  // copy string from kernel's mem seg to the mem seg of the process that called
  // -> use get_from_user() for the other way round!
  if(copy_to_user(buffer, procfs_buffer, procfs_buffer_size)){
    return -EFAULT;
  }

  printk(KERN_INFO "procfs_read: read %lu bytes\n", procfs_buffer_size);
  return procfs_buffer_size;
}


/*
  write to proc

  prefer ssize_t!!!
  
  returns the number of written bytes
//*/
static ssize_t procfs_write(struct file* file, const char* buffer, size_t len, loff_t* off)
{
  if(len > PROCFS_MAX_SIZE){
    procfs_buffer_size = PROCFS_MAX_SIZE;
  }else{
    procfs_buffer_size = len;
  }

  if(copy_from_user(procfs_buffer, buffer, procfs_buffer_size)){
    return -EFAULT;
  }

  printk(KERN_INFO "procfs_write: write %lu bytes\n", procfs_buffer_size);
  return procfs_buffer_size;
}


/*
  permissions for the file
  
  the operation can be 
  0 - execute, run the "file" 
  2 - write, input to the kernel module
  4 - read, output from the kernel module

  this function sets the permissions, ls -l are only for reference 
  and can be overridden here

  returns 0 to allow access, else non-zero / not allowed
//*/
static int module_permission(struct inode *inode, int op, struct nameidata* foo)
{
  // everybody is allowed to read from our module, but only root (uid 0) may write it
  if(op == 4 || ((op == 2) && (current->euid == 0))){
    return 0;
  }

  // if it's anything else, access denied
  return -EACCES;
}


/*
  file is opened
  
  we don't care but increment the module's reference count
  
  returns 0 for success
//*/
int procfs_open(struct inode* inode, struct file* file)
{
  try_module_get(THIS_MODULE);
  return 0;
}


/*
  file is closed

  we don't care but decrement the module's reference count

  returns 0 for success
//*/
int procfs_close(struct inode* inode, struct file* file)
{
  module_put(THIS_MODULE);
  return 0;
}


/*
  linux init & clean up
//*/


int init_module(void)
{
  // create the /proc file
  if(NULL == (my_proc_file = create_proc_entry(PROC_ENTRY_FILENAME, 0644, NULL))){
    printk(KERN_ALERT "error: could not initialize /proc/%s\n", PROC_ENTRY_FILENAME);
    return -ENOMEM;
  }

  my_proc_file->owner = THIS_MODULE;
  my_proc_file->proc_iops = &my_proc_file_iops;
  my_proc_file->proc_fops = &my_proc_file_fops;
  my_proc_file->mode = S_IFREG | S_IRUGO | S_IWUSR;
  my_proc_file->uid = 0;
  my_proc_file->gid = 0;
  my_proc_file->size = 80;

  printk(KERN_INFO "/proc/%s created\n", PROC_ENTRY_FILENAME);

  return 0;
}

void cleanup_module(void)
{
  remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
  printk(KERN_INFO "/proc/%s removed\n", PROC_ENTRY_FILENAME);
}
