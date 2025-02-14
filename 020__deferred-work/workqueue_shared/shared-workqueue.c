// SPDX-License-Identifier: GPL-2.0+
/*
 * REFERENCES
 * - Linux Device Driver Programming, J. Madieu, 2022
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

struct work_data {
	struct work_struct work_item;
	wait_queue_head_t private_wq;
	int data;
};

static int sleep; /* 0 initialized */

static void
work_handler(struct work_struct *work)
{
	struct work_data *wd;

	pr_info("%s(): called\n", __func__);
	wd = container_of(work, struct work_data, work_item);
	pr_info("%s(): data is %d\n", __func__, wd->data);

	msleep(2000);
	wake_up_interruptible(&wd->private_wq);
	kfree(wd);
}

static int __init mod_init(void)
{
	struct work_data *wd;

	pr_info("%s(): called\n", __func__);
	wd = kmalloc(sizeof(*wd), GFP_KERNEL);

	wd->data = 47;
	INIT_WORK(&wd->work_item, work_handler);
	init_waitqueue_head(&wd->private_wq);

	schedule_work(&wd->work_item);
	pr_info("%s(): going to sleep...\n", __func__);
	wait_event_interruptible(wd->private_wq, sleep != 0);
	pr_info("%s(): wake up!\n", __func__);
	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with waitqueues.");
