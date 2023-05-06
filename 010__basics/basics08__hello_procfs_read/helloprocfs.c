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


  Writing to the procfs is (still) not supported.

  ---
  REFERENCES:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18

  VERIFIED:
  v3.10/x86
  Several upgrades, since 3.10 the create_proc_entry() was changed to
  the new function, then the mmap from userspace became obsolete,
  nowadays the param "char __user*" performs the mapping.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

/* forwards */

static ssize_t read_procfs(struct file *, char __user *, size_t, loff_t *);
int start_procfs(void);
void stop_procfs(void);

/* macros / globals */

static struct proc_dir_entry *ent;

#define PROCFS_NAME "lothars_procfs_entry"

struct proc_ops proc_fops = {
	.proc_read = read_procfs,
};

static ssize_t read_procfs(struct file *filp, char __user *ubuf, size_t count,
			   loff_t *offp)
{
	pr_info("read handler\n");
	return count;
}

int start_procfs(void)
{
	/*
	  static inline struct proc_dir_entry *proc_create(
	      const char *name
              , umode_t mode
	      , struct proc_dir_entry *parent
	      , const struct file_operations *proc_fops)

	  name: The name of the proc entry
	  mode: The access mode for proc entry
	  parent: The name of the parent directory under /proc
	  proc_fops: The structure in which the file operations for
	             the proc entry will be created.
	*/
	if (NULL == (ent = proc_create(PROCFS_NAME, 0644, NULL, &proc_fops))) {
		printk(KERN_ALERT "/proc/%s failed\n", PROCFS_NAME);
	}
	pr_info("/proc/%s created\n", PROCFS_NAME);

	return 0;
}

void stop_procfs(void)
{
	proc_remove(ent);
	pr_info("/proc/%s removed\n", PROCFS_NAME);
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
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with procfs entries");
