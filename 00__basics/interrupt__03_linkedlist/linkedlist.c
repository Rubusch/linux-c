/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h> /* file_operations */
#include <linux/slab.h>
#include <linux/cdev.h>


/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_linkedlist(void);
void cleanup_hello_linkedlist(void);

// chardev read()
static ssize_t hello_linkedlist_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t hello_linkedlist_write(struct file *, const char __user *, size_t, loff_t *);


/*
  globals
*/

// chardev device
#define HELLO_DEVICE_MINOR 76

#define HELLO_DEVICE_CHRDEV "lothars_chrdev"
#define HELLO_DEVICE_CLASS "lothars_class"
#define HELLO_DEVICE_NAME "lothars_device"

// chardev
dev_t dev;
static struct class *dev_class;
static struct cdev hello_linkedlist_cdev;

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.read = hello_linkedlist_read,
	.write = hello_linkedlist_write,
};

// linked list using kernel's double linked list
struct lothars_list {
	struct list_head list; // kernel's list implementation
	unsigned long long data;
};
LIST_HEAD(head_node);


/*
  implementation
*/

/*
  chardev read
*/
static ssize_t hello_linkedlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	struct lothars_list *tmp;
	int cnt = 0;

	printk( KERN_INFO "%s()", __func__);

	// traversing list [MACRO for a FOR loop]
	list_for_each_entry(tmp, &head_node, list) {
		printk(KERN_INFO "node %d data = %llu\n", cnt++, tmp->data);
	}
	printk(KERN_INFO "total nodes: %d\n", cnt);

	return 0;
}

/*
  chardev write
*/
static ssize_t hello_linkedlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	struct lothars_list *tmp_node = NULL;
	int ret = -1;
	unsigned long long val = -1;

	printk(KERN_INFO "%s()\n", __func__);

	// convert buf (user) to int (kernel)
	ret = kstrtoull_from_user(buf, len, 10, &val);
	if (0 > ret) {
		printk(KERN_ERR "invalid value\n");
		return -EINVAL;
	}
	printk(KERN_INFO "received %llu", val);
	*off = len;

	// create node
	tmp_node = kmalloc(sizeof(struct lothars_list), GFP_KERNEL);
	if (NULL == tmp_node) {
		printk(KERN_ERR "kmalloc() failed to alloc linked list\n");
		return -ENOMEM;
	}

	// assign received data
	tmp_node->data = val;

	// init the list within the struct
	if (list_empty(&head_node))
		INIT_LIST_HEAD(&tmp_node->list);

	// add node to linked list
	list_add_tail(&tmp_node->list, &head_node);

	return len;
}

/*
  start / stop module
*/

int init_hello_linkedlist(void)
{
	printk(KERN_INFO "%s() initializing...\n", __func__);

	if (0 > alloc_chrdev_region(&dev, HELLO_DEVICE_MINOR, 1, HELLO_DEVICE_CHRDEV)) {
		printk(KERN_ERR "alloc_chrdev_region() failed\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "%s() major = %d, minor = %d\n", __func__, MAJOR(dev), MINOR(dev));

	cdev_init(&hello_linkedlist_cdev, &fops);

	if (0 > cdev_add(&hello_linkedlist_cdev, dev, 1)) {
		printk(KERN_ERR "cdev_add() failed\n");
		goto err_cdev;
	}

	dev_class = class_create(THIS_MODULE, HELLO_DEVICE_CLASS);
	if (NULL == dev_class) {
		printk(KERN_ERR "class_create() failed\n");
		goto err_class;
	}

	if (NULL == device_create(dev_class, NULL, dev, NULL, HELLO_DEVICE_NAME)) {
		printk(KERN_ERR "device_create() failed\n");
		goto err_device;
	}

	return 0;

err_device:
	device_destroy(dev_class, dev);

err_class:
	class_destroy(dev_class);

err_cdev:
	cdev_del(&hello_linkedlist_cdev);
	unregister_chrdev_region(dev, 1);

	return -ENOMEM;
}

void cleanup_hello_linkedlist(void)
{
	// linked list
	struct lothars_list *cursor, *tmp;
	list_for_each_entry_safe(cursor, tmp, &head_node, list) {
		list_del(&cursor->list);
		kfree(cursor);
	}

	// chardev
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&hello_linkedlist_cdev);
	unregister_chrdev_region(dev, 1);

	printk(KERN_INFO "%s() READY.\n", __func__);
}


/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_hello_linkedlist();
}

static void __exit mod_exit(void)
{
	cleanup_hello_linkedlist();
}

module_init(mod_init);
module_exit(mod_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates the usage of linkedlist!");
