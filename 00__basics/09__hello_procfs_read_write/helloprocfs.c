/*
  helloprocfs


  Demonstrates writing to the procfs. The implementation does a proc
  file entry, and implements a read handler.

  Open a terminal:
  $ cat /proc/lothars_procfs_entry
      <waits...>

  See on the first terminal the following log entry:
  $ dmesg
      (...)
      Jan 21 23:01:52 debian kernel: read handler


  Now check out the write handler:
  $ echo "foo" | sudo tee -a /proc/lothars_procfs_entry
      foo
      tee: /proc/lothars_procfs_entry: Operation not permitted

  $ dmesg
      (...)
      Jan 21 23:27:51 debian kernel: write handler

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18

  Several upgrades, since 3.10 the create_proc_entry() was changed to
  the new function, then the mmap from userspace became obsolete,
  nowadays the param "char __user*" performs the mapping.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>


/* forwards */

static ssize_t read_procfs(struct file *, char __user*, size_t, loff_t *);
static ssize_t write_procfs(struct file *, const char __user*, size_t, loff_t *);
int start_procfs(void);
void stop_procfs(void);



/* macros / globals */

static struct proc_dir_entry *ent;

#define PROCFS_NAME "lothars_procfs_entry"

struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.read = read_procfs,
	.write = write_procfs,
};


static ssize_t read_procfs(struct file *filp, char __user *ubuf, size_t count, loff_t *offp)
{
	printk(KERN_INFO "read handler\n");
	return 0;
}


static ssize_t write_procfs(struct file *filp, const char __user *ubuf, size_t count, loff_t *offp)
{
	printk(KERN_INFO "write handler\n");
	return 0; // NB: when this is set to -1, the write returns
		  // right away (with an error), if not, it will
		  // freeze and stops only with a "sudo rmmod -f
		  // helloprocfs" from another shell
}


int start_procfs(void)
{
	if (NULL == (ent = proc_create(PROCFS_NAME, 0644, NULL, &proc_fops))) {
		printk(KERN_ALERT "/proc/%s failed\n", PROCFS_NAME);
	}
	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);

	return 0;
}


void stop_procfs(void)
{
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
