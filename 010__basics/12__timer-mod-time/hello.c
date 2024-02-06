// SPDX-License-Identifier: GPL-2.0+
/*
  VERIFIED:
  based on demo for kernel 4.9 version of "hello"
  kernel: 6.3.1/aarch64 (verified)
 */
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/time.h>

static struct timer_list start_timer;

void
say_cb(struct timer_list * data)
{
	pr_info("%s(): lothar's timer callback\n", __func__);
}

static int
__init mod_init(void)
{
	pr_info("%s(): called", __func__);
	timer_setup(&start_timer, &say_cb, 0);
	mod_timer(&start_timer, jiffies + msecs_to_jiffies(1000));
	return 0;
}

static void
__exit mod_exit(void)
{
	pr_info("%s(): called", __func__);
	del_timer(&start_timer);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with old timers.");
