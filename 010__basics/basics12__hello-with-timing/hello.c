/*
  hello demo with timer information


  obtain timing information taken from, e.g.:
  linux/drivers/usb/mon/mon_text.c +188

     struct timespec64 now;
     unsigned int stamp;
     ktime_get_ts64(&now);
     stamp = now.tv_sec & 0xFFF;  / * 2^32 = 4294967296. Limit to 4096s. * /
     stamp = stamp * USEC_PER_SEC + now.tv_nsec / NSEC_PER_USEC;

  USAGE:
  $ sudo insmod ./hello.ko num=3
  $ sudo rmmod hello
  $ dmesg | tail
     [ 6709.087341] [6/7] lothar's hello
     [ 6709.087350] [7/7] lothar's hello
     [ 6727.144308] lothar's init
     [ 6727.144335] [1/3] lothar's hello
     [ 6727.144353] [2/3] lothar's hello
     [ 6727.144362] [3/3] lothar's hello
     [ 6730.722522] lothar's exit - unloading module after 3 secs
     [ 6730.722547] [1/3] lothar's hello
     [ 6730.722557] [2/3] lothar's hello
     [ 6730.722566] [3/3] lothar's hello

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018

  VERIFIED:
  based on demo for kernel 4.9 version of "hello"
  kernel: 6.3.1/aarch64 (verified)
 */
#include <linux/module.h>
#include <linux/time.h>

static int num = 7;
static struct timespec64 start_time;

module_param(num, int, S_IRUGO); // NB: S_IRUGO == S_IRUSR | S_IRGRP | S_IROTH

static int __init hello_init(void);
static void __exit hello_exit(void);
void say(void);

static int __init hello_init(void)
{
	ktime_get_ts64(&start_time);
	pr_info("lothar's init\n");
	say();
	return 0;
}

void say(void)
{
	int idx;
	for (idx = 1; idx <= num; idx++) {
		pr_info("[%d/%d] lothar's hello\n", idx, num);
	}
}

static void __exit hello_exit(void)
{
	struct timespec64 end_time;
	long int start, end;
	ktime_get_ts64(&end_time);

	start = start_time.tv_sec & 0xFFF;
	end = end_time.tv_sec & 0xFFF;

	pr_info("lothar's exit - unloading module after %ld secs\n",
			end - start);
	say();
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("rpi workspace test");
