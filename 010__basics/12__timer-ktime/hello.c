// SPDX-License-Identifier: GPL-2.0+
/*
  VERIFIED:
  based on demo for kernel 4.9 version of "hello"
  kernel: 6.3.1/aarch64 (verified)
 */
#include <linux/module.h>
#include <linux/time.h>

static int num = 7;
static struct timespec64 start_time;

module_param(num, int, S_IRUGO); // NB: S_IRUGO == S_IRUSR | S_IRGRP | S_IROTH

void
say(void)
{
	int idx;
	for (idx = 1; idx <= num; idx++) {
		pr_info("%s(): [%d/%d] lothar's hello\n", __func__, idx, num);
	}
}

static int
__init hello_init(void)
{
	pr_info("%s(): called", __func__);

	ktime_get_ts64(&start_time);
	say();

	return 0;
}

static void
__exit hello_exit(void)
{
	struct timespec64 end_time;
	long int start, end;

	pr_info("%s(): called", __func__);
	ktime_get_ts64(&end_time);

	start = start_time.tv_sec & 0xFFF;
	end = end_time.tv_sec & 0xFFF;

	pr_info("%s(): unloading module after %ld secs\n",
		__func__, end - start);
	say();
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with ktime and timers.");
