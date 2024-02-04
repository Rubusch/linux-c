// SPDX-License-Identifier: GPL-2.0+
/*
  NB: Memory Copying

  Kernel -> Userspace: copy_to_user();
  Userspace -> Kernel: copy_from_user();

  - userspace builds on virtual memory
  - prevents crashes due to inaccessible regions
  - handles architecture specific issues

  ---
  REFERENCES:
  - Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
  - https://www.cs.fsu.edu/~cop4610t/lectures/project2/procfs_module/proc_module.pdf

  VERIFIED:
  v3.10/x86, upgraded
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>  /* procfs */
#include <linux/slab.h>     /* kmalloc(), later prefer devm_malloc() */
#include <linux/uaccess.h>  /* copy_to_user(), copy_from_user() */

static int open_procfs(struct inode *, struct file *);
static ssize_t read_procfs(struct file *, char __user *, size_t, loff_t *);
static ssize_t write_procfs(struct file *, const char __user *, size_t,
			    loff_t *);

int start_procfs(void);
void stop_procfs(void);

static struct proc_dir_entry *ent;

#define PROCFS_NAME "lothars_procfs_entry"
#define PROC_FILE_PERMS 0644
#define PROC_PARENT_DIR NULL

/* // The old legacy file_operations
static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.open = open_procfs,
	.read = read_procfs,
	.write = write_procfs,
};
/*/
// NB: the following needs totally different parameters
static const struct proc_ops proc_fops = {
	.proc_open = open_procfs,
	.proc_read = read_procfs,
	.proc_write = write_procfs,
};
// */

static char *message;
static int read_p;

static int open_procfs(struct inode *inode, struct file *file)
{
	pr_info("%s()", __func__);

	read_p = 1;

	message = kmalloc(
		20 * sizeof(*message),
		GFP_KERNEL); // alternative use: __GFP_WAIT|__GFP_IO|__GFP_FS
	if (NULL == message) {
		pr_alert("%s(): Error at allocation\n", __func__);
		return -ENOMEM;
	}

	strcpy(message, "Hello ProcFS!\n");

	return 0;
}

static ssize_t read_procfs(struct file *filp, char __user *buf, size_t count,
			   loff_t *offp)
{
	int len = 0, ret = 0;

	pr_info("%s()\n", __func__);
	len = strlen(message);

	read_p = !read_p; // toggle, read until 0, then return
	if (read_p) {
		return 0;
	}

	pr_info("READ\n");
	ret = copy_to_user(buf, message, len);
	if (0 > ret) {
		return ret;
	}
	return len;
}

static ssize_t write_procfs(struct file *filp, const char __user *buf,
			    size_t count, loff_t *offp)
{
//	pr_info("%s()\n", __func__); // can be bulky
	return 0;
}

int start_procfs(void)
{
	pr_info("%s()\n", __func__);

	ent = proc_create(PROCFS_NAME, PROC_FILE_PERMS, PROC_PARENT_DIR,
			  &proc_fops);
	if (NULL == ent) {
		printk(KERN_ALERT "/proc/%s failed\n", PROCFS_NAME);
		proc_remove(ent);
		// alternative: remove_proc_entry(PROCFS_NAME, NULL);
		return -ENOMEM;
	}
	pr_info("/proc/%s created\n", PROCFS_NAME);

	return 0;
}

void stop_procfs(void)
{
	pr_info("%s()\n", __func__);

	kfree(message);
	proc_remove(ent);
	pr_info("/proc/%s removed\n", PROCFS_NAME);
}

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
MODULE_DESCRIPTION("even more mess with procfs entries");
