// helloprocfs.c
/*
  linux kernel module, 2.6.18

  write (user) to /proc, read it out with
  echo "foobar" > /proc/lothars_proc_entry  

  read (user) the proc file system, write some text to /proc
  cat /proc/lothars_proc_entry

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

#include <linux/kernel.h> // kernel
#include <linux/module.h> // module
#include <linux/proc_fs.h> // procfs
#include <asm/uaccess.h> // copy_from_user()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates the usage of a procfs entry");

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "lothars_proc_entry"

// holds proc info
static struct proc_dir_entry *my_proc_file;

// buffer for characters in this module
static char procfs_buffer[PROCFS_MAX_SIZE];

// size of the buffer
static unsigned long procfs_buffer_size = 0;

/*
  reads the procfs

  returns the buffersize, or 0 if finished
//*/
int procfile_read(char *buffer, char **buffer_location, off_t offset,
		  int buffer_length, int *eof, void *data)
{
	int ret;
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", PROCFS_NAME);
	if (offset > 0) {
		// we have finished to read, return 0
		ret = 0;
	} else {
		// fill the buffer, return the buffer size
		memcpy(buffer, procfs_buffer, procfs_buffer_size);
		ret = procfs_buffer_size;
	}

	return ret;
}

/*
  writes to the procfs
  
  returns the written buffer size
//*/
int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	// get buffer size
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	// write data to the buffer
	if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
		return -EFAULT;
	}
	return procfs_buffer_size;
}

/*
  linux stuff, init and cleanup
//*/

int init_module(void)
{
	// create the /proc entry
	if (NULL ==
	    (my_proc_file = create_proc_entry(PROCFS_NAME, 0644, NULL))) {
		// XXX - proc_root issue in 2.6.27
		//    remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "error: could not initialize /proc/%s\n",
		       PROCFS_NAME);
		return -ENOMEM;
	}
	my_proc_file->read_proc = procfile_read;
	my_proc_file->write_proc = procfile_write;
	my_proc_file->owner = THIS_MODULE;
	my_proc_file->mode = S_IFREG | S_IRUGO;
	my_proc_file->uid = 0;
	my_proc_file->gid = 0;
	my_proc_file->size = 37;

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;
}

void cleanup_module(void)
{
	// XXX - proc_root issue in 2.6.27
	//  remove_proc_entry(PROCFS_NAME, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}
