// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/module.h>
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>

static struct hrtimer highres;
u64 start_time;

static enum hrtimer_restart
say_time(struct hrtimer *timer)
{
	u64 now = jiffies; // current time
	pr_info("%s(): hello hrtimer started %u msec ago",
		__func__, jiffies_to_msecs(now - start_time));

	return HRTIMER_NORESTART;
}

static int
__init hello_init(void)
{
	pr_info("%s(): called", __func__);
	hrtimer_init(&highres, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	highres.function = &say_time;
	start_time = jiffies; // current time
	hrtimer_start(&highres, ms_to_ktime(1000), HRTIMER_MODE_REL);

	return 0;
}

static void
__exit hello_exit(void)
{
	pr_info("%s(): called", __func__);
	hrtimer_cancel(&highres);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with hrtimers.");
