/*
  helloprocfs


  Demonstrates writing to the procfs. The implementation does a proc
  file entry, and implements a read handler with sequence operations.

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
  REFERENCES:
  - Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
  - https://www.cs.fsu.edu/~cop4610t/lectures/project2/procfs_module/proc_module.pdf

  VERIFIED:
  - v5.4/x86
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h> /* procfs */
#include <linux/slab.h> /* kmalloc() */
#include <linux/uaccess.h> /* copy_to_user(), copy_from_user() */
#include <linux/seq_file.h> /* struct seq_operations */

/* forwards */

static int open_procfs(struct inode *, struct file *);
static ssize_t read_procfs(struct file *, char __user *, size_t, loff_t *);
static ssize_t write_procfs(struct file *, const char __user *, size_t,
			    loff_t *);

static void *proper_seq_start(struct seq_file *, loff_t *);
static void proper_seq_stop(struct seq_file *, void *);
static void *proper_seq_next(struct seq_file *, void *, loff_t *);
static int proper_seq_show(struct seq_file *, void *);

int start_hello_procfs(void);
void stop_hello_procfs(void);

/* macros / globals */

static struct proc_dir_entry *ent;

#define PROCFS_NAME "lothars_procfs_entry"
#define PROC_FILE_PERMS 0644
#define PROC_PARENT_DIR NULL

static struct proc_ops proc_fops = {
	.proc_open = open_procfs,
	.proc_read = read_procfs,
	.proc_write = write_procfs,
};

static struct seq_operations proc_sops = {
	.start = proper_seq_start,
	.stop = proper_seq_stop,
	.next = proper_seq_next,
	.show = proper_seq_show,
};

static char *message;
static int read_p;

/*
  implementation
*/

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

	return seq_open(file, &proc_sops);
}

static ssize_t read_procfs(struct file *filp, char __user *buf, size_t count,
			   loff_t *offp)
{
	int len = 0;
	pr_info("%s()\n", __func__);
	len = strlen(message);

	read_p = !read_p; // toggle, read until 0, then return
	if (read_p) {
		return 0;
	}

	pr_info("READ\n");
	copy_to_user(buf, message, len);
	return len;
}

static ssize_t write_procfs(struct file *filp, const char __user *buf,
			    size_t count, loff_t *offp)
{
	pr_info("%s()\n", __func__);
	return 0;
}

/*
  The function is called at the begin of the sequence, i.e. when
  - the /proc file is read (first time)
  - after the function stops (end of sequence)

  Returns NULL at the end of a sequence or a "non-NULL" at the
  beginning of a sequence.
*/
static void *proper_seq_start(struct seq_file *pseqfile, loff_t *pos)
{
	static unsigned long cnt = 0;

	if (0 == *pos) { // beginning a new sequence
		return &cnt; // begin sequence, return a non-NULL
	}
	*pos = 0; // end of sequence, set *pos to 0 and return NULL
	return NULL;
}

/*
  The function is called at the end of a sequence.
*/
static void proper_seq_stop(struct seq_file *pseqfile, void *ptr)
{
	// nothing to do, since we use just a static value in start()
}

/*
  The function is called after the beginning of a sequence, and called
  again until the return is NULL, which ends the sequence.
*/
static void *proper_seq_next(struct seq_file *pseqfile, void *ptr, loff_t *pos)
{
	unsigned long *tmp_ptr = (unsigned long *)ptr;
	++(*tmp_ptr);
	++(*pos);

	return NULL;
}

/*
  This function is called for each "step of a sequence".
*/
static int proper_seq_show(struct seq_file *pseqfile, void *ptr)
{
	loff_t *seq_ptr_offset = (loff_t *)ptr;

	seq_printf(pseqfile, "%Ld\n", *seq_ptr_offset);

	return 0;
}

/*
  start/stop hello module
*/

int start_hello_procfs(void)
{
	pr_info("%s()\n", __func__);

	ent = proc_create(PROCFS_NAME, PROC_FILE_PERMS, PROC_PARENT_DIR,
			  &proc_fops);
	if (NULL == ent) {
		pr_alert("/proc/%s failed\n", PROCFS_NAME);
		proc_remove(
			ent); // alternative: remove_proc_entry(PROCFS_NAME, NULL);
		return -ENOMEM;
	}
	pr_info("/proc/%s created\n", PROCFS_NAME);

	return 0;
}

void stop_hello_procfs(void)
{
	pr_info("%s()\n", __func__);

	kfree(message);
	proc_remove(ent);
	pr_info("/proc/%s removed\n", PROCFS_NAME);
}

/*
  init / exit
*/
static int __init mod_init(void)
{
	return start_hello_procfs();
}

static void __exit mod_exit(void)
{
	stop_hello_procfs();
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("yet another messy procfs entry");
