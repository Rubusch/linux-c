// SPDX-License-Identifier: GPL-2.0+
/*
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

#define TIME1 50
#define TIME2 500

static struct timer_list comp_timer;
static struct completion comp;

void timer_callback(struct timer_list *data)
{
	pr_info("%s(): timer expired\n", __func__);
	complete(&comp);
}

static int __init mod_init(void)
{
	int status;

	pr_info("%s(): called\n", __func__);

	timer_setup(&comp_timer, timer_callback, 0);
	init_completion(&comp);

	pr_info("%s(): time: %d ms\n", __func__, TIME1);
	mod_timer(&comp_timer, jiffies + msecs_to_jiffies(TIME1));
	status = wait_for_completion_timeout(&comp, msecs_to_jiffies(100));
	if (!status) {
		pr_warn("%s(): completion timed out\n", __func__);
	}

	reinit_completion(&comp);
	pr_info("%s(): start the timer the second time: %d ms\n", __func__, TIME2);
	mod_timer(&comp_timer, jiffies + msecs_to_jiffies(TIME2));
	status = wait_for_completion_timeout(&comp, msecs_to_jiffies(100));
	if (!status) {
		pr_warn("%s(): completion timed out!\n", __func__);
	}

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	del_timer(&comp_timer);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with completions.");
