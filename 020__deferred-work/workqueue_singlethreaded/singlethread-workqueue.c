// SPDX-License-Identifier: GPL-2.0+
/*
 *  REFERENCES
 * - Linux Device Driver Programming, J. Madieu, 2022
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

struct work_data {
	int data;
	struct work_struct work_item;
};

struct workqueue_struct *wq;

static void work_handler(struct work_struct *work)
{
	struct work_data *wd;

	pr_info("%s(): called\n", __func__);
	wd = container_of(work, struct work_data, work_item);

	pr_info("%s(): data is %d\n", __func__, wd->data);
	kfree(wd);
}

static int __init mod_init(void)
{
	struct work_data *wd;

	pr_info("%s(): called\n", __func__);
	wq = create_singlethread_workqueue("singlethread_wq");
	wd = kmalloc(sizeof(*wd), GFP_KERNEL);

	wd->data = 47;
	INIT_WORK(&wd->work_item, work_handler);
	queue_work(wq, &wd->work_item);

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);

	flush_workqueue(wq);
	destroy_workqueue(wq);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with workqueues.");
