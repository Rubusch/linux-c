// SPDX-License-Identifier: GPL-2.0+
/*
  linked list memory allocation
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/of_device.h>  /* struct of_device_id */

static int block_number = 10;
static int block_size = 5;
static int size_to_read = 0;
static int node_count = 1;
static int cnt = 0;

/* The node definition of the linked list elements */
typedef struct dnode
{
	char *buffer;
	struct dnode *next;
} data_node;

/* The list structure to manage the nodes of the linked list */
typedef struct lnode
{
	data_node *head;
	data_node *cur_write_node;
	data_node *cur_read_node;
	int cur_read_offset;
	int cur_write_offset;
} listic;
static listic new_listic;

/* Allocate the linked list elements and memory buffers

   First allocate a first node, then allocate the rest of the linked
   list nodes up to block_number through a loop.
 */
static int create_list(struct platform_device* pdev)
{
	data_node *new_node, *prev_node, *head_node;
	int idx;

	/* allocate the first node */
	new_node = devm_kmalloc(&pdev->dev, sizeof(data_node), GFP_KERNEL);
	if (!new_node) {
		return -ENOMEM;
	}

	/* allocate the memory buffer of the first node */
	new_node->buffer = devm_kmalloc(&pdev->dev, sizeof(*(new_node->buffer)) * block_size, GFP_KERNEL);
	if (!new_node->buffer) {
		return -ENOMEM;
	}
	new_node->next = NULL;
	new_listic.head = new_node;
	head_node = new_node;
	prev_node = new_node;

	/* allocate the rest of the linked list nodes up to
	   block_number */
	for (idx = 0; idx < block_number; idx++) {
		new_node = devm_kmalloc(&pdev->dev, sizeof(data_node), GFP_KERNEL);
		if (!new_node) {
			return -ENOMEM;
		}
		new_node->buffer = devm_kmalloc(&pdev->dev, sizeof(*(new_node->buffer)) * block_size, GFP_KERNEL);
		if (!new_node->buffer) {
			return -ENOMEM;
		}
		new_node->next = NULL;
		prev_node->next = new_node;
		prev_node = new_node;

	}

	new_node->next = head_node;

	new_listic.cur_read_node = head_node;
	new_listic.cur_write_node = head_node;
	new_listic.cur_read_offset = 0;
	new_listic.cur_write_offset = 0;

	return 0;
}

static ssize_t
lothars_dev_write(struct file* file, const char __user* buf, size_t size, loff_t* offset)
{
	int size_to_copy;

	pr_info("%s() - called\n", __func__);
	pr_info("%s() - node number: %d\n", __func__, node_count);
	if ((0 == *offset) || (1 == node_count))
		size_to_read += size;

	if (size < block_size - new_listic.cur_write_offset)
		size_to_copy = size;
	else
		size_to_copy = block_size - new_listic.cur_write_offset;

	if (copy_from_user(new_listic.cur_write_node->buffer + new_listic.cur_write_offset, buf, size_to_copy) ) {
		return -EFAULT;
	}

	*(offset) += size_to_copy;
	new_listic.cur_write_offset += size_to_copy;
	if (new_listic.cur_write_offset == block_size) {
		new_listic.cur_write_node = new_listic.cur_write_node->next;
		new_listic.cur_write_offset = 0;
		node_count++;
		if (block_number < node_count) {
			new_listic.cur_read_node = new_listic.cur_write_node;
			new_listic.cur_read_offset = 0;
			node_count = 1;
			cnt = 0;
			size_to_read = 0;
		}
	}

	return size_to_copy;
}

static ssize_t
lothars_dev_read(struct file* file, char __user* buf, size_t count, loff_t* offset)
{
	int size_to_copy, read_value;

	pr_info("%s() - called\n", __func__);
	read_value = size_to_read - (block_size * cnt);

	if (*offset < size_to_read) {
		if (read_value < block_size - new_listic.cur_read_offset)
			size_to_copy = read_value;
		else
			size_to_copy = block_size - new_listic.cur_read_offset;

		if (copy_to_user(buf, new_listic.cur_read_node->buffer + new_listic.cur_read_offset, size_to_copy))
			return -EFAULT;

		new_listic.cur_read_offset += size_to_copy;
		(*offset) += size_to_copy;

		if (new_listic.cur_read_offset == block_size) {
			cnt = cnt + 1;
			new_listic.cur_read_node = new_listic.cur_read_node->next;
			new_listic.cur_read_offset = 0;
		}

		return size_to_copy;
	} else {
		msleep(250);
		new_listic.cur_read_node = new_listic.head;
		new_listic.cur_write_node = new_listic.head;
		new_listic.cur_read_offset = 0;
		new_listic.cur_write_offset = 0;
		node_count = 1;
		cnt = 0;
		size_to_read = 0;
	}

	// else path reaches here
	return 0;
}

static int
lothars_dev_open(struct inode* inode, struct file* file)
{
	pr_info("%s() - called\n", __func__);
	return 0;
}

static int
lothars_dev_close(struct inode* inode, struct file* file)
{
	pr_info("%s() - called\n", __func__);
	return 0;
}

static const struct file_operations lothars_dev_fops = {
	.open = lothars_dev_open,
	.read = lothars_dev_read,
	.write = lothars_dev_write,
	.release = lothars_dev_close,
};

static struct miscdevice lothars_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lothars_dev",
	.fops = &lothars_dev_fops,
};

static int
lothars_probe(struct platform_device *pdev)
{
	int ret;
	pr_info("%s() - called\n", __func__);

	create_list(pdev);
	ret = misc_register(&lothars_miscdevice);
	if (0 != ret) {
		pr_err("%s() - could not register the miscdevice structure\n",
		       __func__);
		return ret;
	}

	pr_info("%s() - got minor %i\n",
		__func__, lothars_miscdevice.minor);

	return 0;
}

static int
lothars_remove(struct platform_device *pdev)
{
	pr_info("%s() - called", __func__);
	misc_deregister(&lothars_miscdevice);

	return 0;
}

/*
   For the miscdevice and usage of the device API the DT is needed
  */
static const struct of_device_id lothars_of_ids[] = {
	{ .compatible = "lothars,linked_memory" },
	{},
};
MODULE_DEVICE_TABLE(of, lothars_of_ids);

static struct platform_driver lothars_platform_driver = {
	.probe = lothars_probe,
	.remove = lothars_remove,
	.driver = {
		.name = "linked_memory",
		.of_match_table = lothars_of_ids,
	},
};
module_platform_driver(lothars_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("a memory allocation demo");
