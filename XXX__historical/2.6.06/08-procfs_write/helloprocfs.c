// procfs.c
/*
  linux kernel module (2.6.18)

  demonstrates writing to the procfs
  does a proc file entry, so that it's possible to read by doing:
  cat /proc/lothars_procfs_entry

  writing to the procfs is not supported

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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates the usage of a procfs entry");

#define PROCFS_NAME "lothars_procfs_entry"

// the struct holds information about the /proc file
struct proc_dir_entry *my_proc_file = NULL;

/*
  func puts data into the proc fs file.


  parameters:

  buffer - buffer where the data will be entered, if we decide to use the /proc
  buffer_location - useful if we don't want to use the buffer allocated by the kernel
  offset - current position in the file  
  buffer_length - size of the buffer in the first argument
  eof - write a "1" here to indicate EOF  
  data - a pointer to data (useful in case of a common read for multiple /proc/... entries)


  return value:

  a return value of zero means you have no further information at this time 
  (end of file). a negative return value is an error condition


  for more information

  "the way I discovered what to do with this function wasn't by reading documentation, 
  but by reading the code which used it. 
  I just looked to see what used the get_info field of proc_dir_entry struct (I used 
  a combination of find and grep, if you're interested), and I saw that it is used in 
  <kernel source directory>/fs/proc/array.c

  if something is unknown about the kernel, this is usually the way to go, in Linux 
  we have the great advantage of having the kernel source code for free - use it."
  [Salzman]
//*/
int procfile_read(char *buffer, char **buffer_location, off_t offset,
		  int buffer_length, int *eof, void *data)
{
	int ret = 0; // C90
	printk(KERN_INFO "procfile_read(/proc/%s) called\n", PROCFS_NAME);

	/*
    "we give all of our information in one go, so if the user asks us if we have 
    more information the answer should always be no!

    this is important because the standard read function from the library would 
    continue to issue the read system call until the kernel replies that it has 
    no more information, or until its buffer is filled"
    [Salzman]
  //*/

	if (offset > 0) {
		// we have finished to read, return 0
		ret = 0;

	} else {
		// fill the buffer, return the buffer size
		ret = sprintf(buffer, "HelloWorld!\n");
	}

	return ret;
}

/*
  linux stuff, init and exit
//*/

int init_module(void)
{
	// init proc file entry
	if (NULL ==
	    (my_proc_file = create_proc_entry(PROCFS_NAME, 0644, NULL))) {
		remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "Error: Counld not initialize /proc/%s\n",
		       PROCFS_NAME);
		return -ENOMEM;
	}

	my_proc_file->read_proc = procfile_read;
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
	remove_proc_entry(PROCFS_NAME, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}
