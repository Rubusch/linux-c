// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>

#define HELLO_DEVICE_NAME "lothars_device"

static ssize_t hello_linkedlist_read(struct file *, char __user *, size_t,
				     loff_t *);
static ssize_t hello_linkedlist_write(struct file *, const char __user *,
				      size_t, loff_t *);

static struct file_operations fops = {
	.read = hello_linkedlist_read,
	.write = hello_linkedlist_write,
};

static struct miscdevice hello_device = {
	.name = HELLO_DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

// linked list using kernel's double linked list
struct lothars_list {
	struct list_head list; // kernel's list implementation
	unsigned long long data;
};
LIST_HEAD(head_node);

static ssize_t
hello_linkedlist_read(struct file *filp, char __user *buf,
		      size_t len, loff_t *off)
{
	struct lothars_list *tmp;
	int cnt = 0;

	pr_info("%s(): called\n", __func__);

	// traversing list [MACRO for a FOR loop]
	list_for_each_entry (tmp, &head_node, list) {
		pr_info("%s(): node %d data = %llu\n",
			__func__, cnt++, tmp->data);
	}
	pr_info("%s(): total nodes: %d\n", __func__, cnt);

	return 0;
}

static ssize_t
hello_linkedlist_write(struct file *filp, const char __user *buf,
		       size_t len, loff_t *off)
{
	struct lothars_list *tmp_node = NULL;
	int ret = -1;
	unsigned long long val = -1;

	pr_info("%s(): called\n", __func__);

	// convert buf (user) to int (kernel)
	ret = kstrtoull_from_user(buf, len, 10, &val);
	if (0 > ret) {
		pr_err("%s(): invalid value\n", __func__);
		return -EINVAL;
	}
	pr_info("%s(): received %llu\n", __func__, val);
	*off = len;

	// create node
	tmp_node = kmalloc(sizeof(struct lothars_list), GFP_KERNEL);
	if (NULL == tmp_node) {
		pr_err("%s(): failed to alloc linked list\n", __func__);
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

static int __init mod_init(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);
	ret = misc_register(&hello_device);
	if (0 != ret) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static void __exit mod_exit(void)
{
	struct lothars_list *cursor, *tmp;

	pr_info("%s(): called\n", __func__);
	list_for_each_entry_safe (cursor, tmp, &head_node, list) {
		list_del(&cursor->list);
		kfree(cursor);
	}
	misc_deregister(&hello_device);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates the usage of linkedlist!");
