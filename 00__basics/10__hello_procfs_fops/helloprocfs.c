/*
  helloprocfs


  Demonstrates writing to the procfs. The implementation does a proc
  file entry, and implements a read handler.

  Build and load module:
  $ make
  $ sudo insmod ./helloprocfs.ko


  Open a terminal:
  $ cat /proc/lothars_procfs_entry
      <waits...>

  See on the first terminal the following log entry:
  $ dmesg | tail
      (...)
      Jan 21 23:01:52 debian kernel: read handler


  Now check out the write handler:
  $ echo "foo" | sudo tee -a /proc/lothars_procfs_entry
      foo
      tee: /proc/lothars_procfs_entry: Operation not permitted

  $ dmesg | tail
      (...)
      Jan 21 23:27:51 debian kernel: write handler


  ---
  NB: Memory Copying

  Kernel -> Userspace: copy_to_user();
  Userspace -> Kernel: copy_from_user();

  - userspace builds on virtual memory
  - prevents crashes due to inaccessible regions
  - handles architecture specific issues

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18

  https://www.cs.fsu.edu/~cop4610t/lectures/project2/procfs_module/proc_module.pdf
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h> /* procfs */
#include <linux/slab.h> /* kmalloc() */
#include <linux/uaccess.h> /* copy_to_user(), copy_from_user() */


/* forwards */

static int open_procfs(struct inode*, struct file*);
static ssize_t read_procfs(struct file *, char __user*, size_t, loff_t *);
static ssize_t write_procfs(struct file *, const char __user*, size_t, loff_t *);

int start_procfs(void);
void stop_procfs(void);


/* macros / globals */

static struct proc_dir_entry *ent;

#define PROCFS_NAME "lothars_procfs_entry"
#define PROC_FILE_PERMS 0644
#define PROC_PARENT_DIR NULL

//*
static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.open = open_procfs,
	.read = read_procfs,
	.write = write_procfs,
};
/*/
// NB: the following needs totally different parameters
static const struct proc_ops proc_ops = {
	.proc_owner = THIS_MODULE,
	.proc_open = open_procfs,
	.proc_read = read_procfs,
	.proc_write = write_procfs,
};
// */


static char *message;
static int read_p;


/*
  implementation
*/

static int open_procfs(struct inode* inode, struct file* file)
{
	printk(KERN_INFO "%s()", __func__);

	read_p = 1;

	message = kmalloc(20 * sizeof(*message), GFP_KERNEL); // alternative use: __GFP_WAIT|__GFP_IO|__GFP_FS
	if (NULL == message) {
		printk(KERN_ALERT "%s(): Error at allocation\n", __func__);
		return -ENOMEM;
	}

	strcpy(message, "Hello ProcFS!\n");

	return 0;
}


static ssize_t read_procfs(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	int len = 0;

	printk(KERN_INFO "%s()\n", __func__);

	len = strlen(message);

	read_p = !read_p; // toggle, read until 0, then return
	if (read_p) {
		return 0;
	}

	printk("READ\n");
	copy_to_user(buf, message, len);
	return len;
}


static ssize_t write_procfs(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	printk(KERN_INFO "%s()\n", __func__);
	return 0;
}


int start_procfs(void)
{
	printk(KERN_INFO "%s()\n", __func__);

	ent = proc_create(PROCFS_NAME, PROC_FILE_PERMS, PROC_PARENT_DIR, &proc_ops);
	if (NULL == ent) {
		printk(KERN_ALERT "/proc/%s failed\n", PROCFS_NAME);
		proc_remove(ent); // alternative: remove_proc_entry(PROCFS_NAME, NULL);
		return -ENOMEM;
	}
	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);

	return 0;
}


void stop_procfs(void)
{
	printk(KERN_INFO "%s()\n", __func__);

	kfree(message);
	proc_remove(ent);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}


/*
  init / exit
*/
static int __init mod_init(void)
{
	return start_procfs();
}

static void __exit mod_exit(void)
{
	stop_procfs();
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@gmx.ch>");
MODULE_DESCRIPTION("demonstrates the usage of a procfs entry");
