// procinterface.c
/*
  frontend for various structures in the kernel via procfs

  declarations follow the C90 standard
//*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("reads and writes from a /proc entry");

/*
  defines / macros
//*/

#define PROCFS_BUFSIZ 1024
#define PROCFS_NAME "debug_interface"

/*
  prototypes
//*/

int init_module(void);
void cleanup_module(void);

int proc_read(char *, char **, off_t, int, int *, void *);
int proc_write(struct file *, const char *, unsigned long, void *);

/*
  globals
//*/

static struct proc_dir_entry *Procfs_file;
static char Procfs_buffer[PROCFS_BUFSIZ];
static unsigned long Procfs_curr_bufsiz = 0;

/*
  functions
//*/

/*
  reads from the procfs

  returns the buffersize, or 0 if finished
//*/
int proc_read(char *buffer, char **buffer_location, off_t offset,
	      int buffer_length, int *eof, void *data)
{
	int ret = 0;
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", PROCFS_NAME);
	if (0 < offset) {
		ret = 0;

	} else {
		memcpy(buffer, Procfs_buffer, Procfs_curr_bufsiz);
		ret = Procfs_curr_bufsiz;
	}

	return ret;
}

/*
  writes to the procfs

  returns the written buffer size
//*/
int proc_write(struct file *file, const char *buffer, unsigned long count,
	       void *data)
{
	Procfs_curr_bufsiz = count;
	if (Procfs_curr_bufsiz > PROCFS_BUFSIZ) {
		Procfs_curr_bufsiz = PROCFS_BUFSIZ;
	}

	if (copy_from_user(Procfs_buffer, buffer, Procfs_curr_bufsiz)) {
		return -EFAULT;
	}
	return Procfs_curr_bufsiz;
}

/*
  linux init & clean up
//*/

int init_module(void)
{
	if (NULL ==
	    (Procfs_file = create_proc_entry(PROCFS_NAME, 0644, NULL))) {
		remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "error: could not initialize /proc/%s\n",
		       PROCFS_NAME);
		return -ENOMEM;
	}

	Procfs_file->read_proc = proc_read;
	Procfs_file->write_proc = proc_write;
	Procfs_file->owner = THIS_MODULE;
	Procfs_file->mode = S_IFREG | S_IRUGO;
	Procfs_file->uid = 0;
	Procfs_file->gid = 0;
	Procfs_file->size = 37;

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;
}

void cleanup_module(void)
{
	remove_proc_entry(PROCFS_NAME, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}
