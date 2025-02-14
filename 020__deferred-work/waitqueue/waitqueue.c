// SPDX-License-Identifier: GPL-2.0+
/*
 * REFERENCES
 * - Linux Device Driver Programming, J. Madieu, 2022
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

static DECLARE_WAIT_QUEUE_HEAD(lothars_wq);
static int condition; /* static, so condition = 0 initialized */
static struct work_struct workit;

static void
work_handler(struct work_struct *work)
{
	pr_info("%s(): called\n", __func__);
	msleep(3000);
	pr_info("%s(): wake up\n", __func__);
	condition = 1;
	wake_up_interruptible(&lothars_wq);
}

static int __init mod_init(void)
{
	pr_info("%s(): called\n", __func__);

	INIT_WORK(&workit, work_handler);
	schedule_work(&workit);

	pr_info("%s(): going to sleep\n", __func__);
	wait_event_interruptible(lothars_wq, condition != 0);

	pr_info("%s(): woken up by the work job\n", __func__);
	return 0;
}

void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with waitqueues.");
