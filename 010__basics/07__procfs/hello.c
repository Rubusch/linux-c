// SPDX-License-Identifier: GPL-2.0+
/*
  NB: Memory Copying

  Kernel -> Userspace: copy_to_user();
  Userspace -> Kernel: copy_from_user();

  - userspace builds on virtual memory
  - prevents crashes due to inaccessible regions
  - handles architecture specific issues
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

static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

static const struct proc_ops proc_fops = {
	.proc_open = open_procfs,
	.proc_read = read_procfs,
	.proc_write = write_procfs,
};

static char *message;
static int read_p;

static int
open_procfs(struct inode *inode, struct file *file)
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

static ssize_t
read_procfs(struct file *filp, char __user *buf, size_t count,
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

static ssize_t
write_procfs(struct file *filp, const char __user *buf,
			    size_t count, loff_t *offp)
{
	pr_info("%s()\n", __func__);
	return -EINVAL;
}

static int
__init mod_init(void)
{
	pr_info("%s()\n", __func__);
	proc_folder = proc_mkdir("lothars_dir", NULL);
	if (NULL == proc_folder) {
		pr_err("%s(): failed create procfs folder", __func__);
		return -ENOMEM;
	}

	proc_file = proc_create("lothars_file", 0666, proc_folder, &proc_fops);
	if (NULL == proc_file) {
		pr_err("%s(): failed creating procfs file", __func__);
		proc_remove(proc_folder);
		return -ENOMEM;
	}

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s()\n", __func__);

	kfree(message);
	proc_remove(proc_file);
	proc_remove(proc_folder);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("even more mess with procfs entries");
