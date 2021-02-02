/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h> /* kthread_run(), kthread_create() */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/completion.h>
#include <linux/slab.h>



/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_completion(void);
void cleanup_hello_completion(void);

// kthread completion
static int wait_function(void *);

// chardev
static ssize_t chardev_read(struct file*, char __user *, size_t, loff_t *);



/*
  globals
*/

#define THREAD_NAME "lothars_wait_thread"
#define CHARDEV_NAME "lothars_chardev"

// kthread tasks
static struct task_struct *wait_thread;
struct completion *data_read_done = NULL;
int completion_flag = 0;

// mutex
struct mutex lothars_mutex;

// data
unsigned long read_counter = 0;

// chardev
dev_t dev = 0;
static struct class *dev_class;
static struct cdev chardev_cdev;

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.read = chardev_read,
};



/*
  implementation
*/

static int wait_function(void *vp)
{
	do {
		printk(KERN_INFO "waiting for event...\n");
		wait_for_completion(data_read_done); // blocking
		if (2 == completion_flag) {
			printk(KERN_INFO "event came from exit function\n");
			return 0;
		} else {
			printk(KERN_INFO "event came from READ function - %lu\n", read_counter++);
		}
		completion_flag = 0;
	} while (1);
	do_exit(0);
	return 0;
}


static ssize_t chardev_read(struct file* filp, char __user *buf, size_t len, loff_t* poff)
{
	printk(KERN_INFO "%s()\n", __func__);
	completion_flag = 1;
	if (!completion_done(data_read_done)) {
		complete(data_read_done);
	}
	return 0;
}


int init_hello_completion(void)
{
	printk(KERN_INFO "%s() started\n", __func__);

	if (0 > alloc_chrdev_region(&dev, 0, 1, CHARDEV_NAME)) {
		printk(KERN_ERR "alloc_chrdev_region() failed\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "%s() - major = %d, minor = %d\n", __func__, MAJOR(dev), MINOR(dev));

	cdev_init(&chardev_cdev, &fops);
	chardev_cdev.ops = &fops;

	if (0 > cdev_add(&chardev_cdev, dev, 1)) {
		printk(KERN_ERR "cdev_add() failed\n");
		goto err_cdev;
	}

	dev_class = class_create(THIS_MODULE, CHARDEV_NAME);
	if (NULL == dev_class) {
		printk(KERN_ERR "class_create() failed\n");
		goto err_class;
	}

	if (NULL == device_create(dev_class, NULL, dev, NULL, CHARDEV_NAME)) {
		printk(KERN_ERR "device_create() failed\n");
		goto err_device;
	}


	wait_thread = kthread_create(wait_function, NULL, THREAD_NAME);
	if (NULL == wait_thread) {
		printk(KERN_ERR "kthread_create() failed\n");
		goto err_kthread;
	}
	printk(KERN_INFO "kthread_create() successful\n");
	wake_up_process(wait_thread);

	data_read_done = kmalloc(sizeof(data_read_done), GFP_KERNEL);
	if (NULL == data_read_done) {
		printk(KERN_ERR "kmalloc() failed\n");
		goto err_kthread;
	}
	init_completion(data_read_done);

	return 0;

err_kthread:
	device_destroy(dev_class, dev);

err_device:
	class_destroy(dev_class);

err_class:
	cdev_del(&chardev_cdev);

err_cdev:
	unregister_chrdev_region(dev, 1);
	return -ENOMEM;
}


void cleanup_hello_completion(void)
{
	// wait / completion event
	completion_flag = 2;
	if (!completion_done(data_read_done)) {
		complete(data_read_done);
	}

	// completion
	kfree(data_read_done);

	// chardev
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&chardev_cdev);
	unregister_chrdev_region(dev, 1);

	printk("%s() READY.\n", __func__);
}


/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_hello_completion();
}

static void __exit mod_exit(void)
{
	cleanup_hello_completion();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates elementary usage of completion / waitqueue.");
