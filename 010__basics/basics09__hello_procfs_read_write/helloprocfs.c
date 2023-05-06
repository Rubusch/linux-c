/*
  ---
  REFERENCES:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18

  VERIFIED:
  v3.10/x86, upgraded
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

/* forwards */
static ssize_t read_procfs(struct file *, char __user *, size_t, loff_t *);
static ssize_t write_procfs(struct file *, const char __user *, size_t,
			    loff_t *);

int start_procfs(void);
void stop_procfs(void);

/* macros / globals */
static struct proc_dir_entry *ent;

#define PROCFS_NAME "lothars_procfs_entry"

struct proc_ops proc_fops = {
	.proc_read = read_procfs,
	.proc_write = write_procfs,
};

static ssize_t read_procfs(struct file *filp, char __user *ubuf, size_t count,
			   loff_t *offp)
{
	pr_info("read handler\n");
	return 0;
}

static ssize_t write_procfs(struct file *filp, const char __user *ubuf,
			    size_t count, loff_t *offp)
{
	pr_info("write handler\n");
	return -1; // NB: when this is set to -1, the write returns
		// right away (with an error), if not, it will
		// freeze and stops only with a "sudo rmmod -f
		// helloprocfs" from another shell
}

int start_procfs(void)
{
	pr_info("%s() initializing...\n", __func__);
        // NB: avoid packed implementation like this in kernel source
	if (NULL == (ent = proc_create(PROCFS_NAME, 0644, NULL, &proc_fops))) {
		printk(KERN_ALERT "/proc/%s failed\n", PROCFS_NAME);
	}
	pr_info("%s() /proc/%s created\n", __func__, PROCFS_NAME);

	return 0;
}

void stop_procfs(void)
{
	proc_remove(ent);
	pr_info("%s() /proc/%s removed\n", __func__, PROCFS_NAME);
	pr_info("%s() READY.\n", __func__);
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
