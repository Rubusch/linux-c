// SPDX-License-Identifier: GPL-2.0-only
/*
  kretprobe_example.c

  Here's a sample kernel module showing the use of return probes to
  report the return value and total time taken for probed function
  to run.

  usage: insmod kretprobe_example.ko func=<func_name>

  If no func_name is specified, kernel_clone is instrumented

  For more information on theory of operation of kretprobes, see
  Documentation/trace/kprobes.rst

  Build and insert the kernel module as done in the kprobe example.
  You will see the trace data in /var/log/messages and on the console
  whenever the probed function returns. (Some messages may be suppressed
  if syslogd is configured to eliminate duplicate messages.)

  USAGE

  in case use another symbol (hardcoded, or re-implement as param)
  # inmod probe

  log, and prform some actions, returns of the called symbol
  (function) will trigger and display the information such as pointer
  values, etc to investigation
  $ tail -f /var/log/messages
       Feb 24 17:53:03 ctrl001 kernel: [ 5153.638924] kernel_clone returned 3214 and took 1056092 ns to execute
       Feb 24 17:53:04 ctrl001 kernel: [ 5153.686299] kretprobe at 00000000353985b6 unregistered
       Feb 24 17:53:04 ctrl001 kernel: [ 5153.686354] Missed probing 0 instances of kernel_clone

       Feb 24 17:53:10 ctrl001 kernel: [ 5160.176391] Planted return probe at kernel_clone: 00000000353985b6
       Feb 24 17:53:10 ctrl001 kernel: [ 5160.180139] kernel_clone returned 3218 and took 1828070 ns to execute

       Feb 24 17:53:51 ctrl001 kernel: [ 5201.468742] kernel_clone returned 3219 and took 1057551 ns to execute
       Feb 24 17:53:58 ctrl001 kernel: [ 5208.473093] kernel_clone returned 3220 and took 1012499 ns to execute
       ...

  REFERENCE
  https://elixir.bootlin.com/linux/latest/source/samples/kprobes/kretprobe_example.c
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/sched.h>

static char func_name[KSYM_NAME_LEN] = "kernel_clone";
module_param_string(func, func_name, KSYM_NAME_LEN, 0644);
MODULE_PARM_DESC(func, "Function to kretprobe; this module will report the"
		 " function's execution time");

/* per-instance private data */
struct my_data {
	ktime_t entry_stamp;
};

/* Here we use the entry_hanlder to timestamp function entry */
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct my_data *data;

	if (!current->mm)
		return 1;/* Skip kernel threads */

	data = (struct my_data *)ri->data;
	data->entry_stamp = ktime_get();
	return 0;
}
NOKPROBE_SYMBOL(entry_handler);

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	unsigned long retval = regs_return_value(regs);
	struct my_data *data = (struct my_data *)ri->data;
	s64 delta;
	ktime_t now;

	now = ktime_get();
	delta = ktime_to_ns(ktime_sub(now, data->entry_stamp));
	pr_info("%s returned %lu and took %lld ns to execute\n",
		func_name, retval, (long long)delta);
	return 0;
}
NOKPROBE_SYMBOL(ret_handler);

static struct kretprobe my_kretprobe = {
	.handler= ret_handler,
	.entry_handler= entry_handler,
	.data_size= sizeof(struct my_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive= 20,
};

static int __init kretprobe_init(void)
{
	int ret;

	my_kretprobe.kp.symbol_name = func_name;
	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		pr_err("register_kretprobe failed, returned %d\n", ret);
		return ret;
		}
	pr_info("Planted return probe at %s: %p\n",
		my_kretprobe.kp.symbol_name, my_kretprobe.kp.addr);
	return 0;
}

static void __exit kretprobe_exit(void)
{
	unregister_kretprobe(&my_kretprobe);
	pr_info("kretprobe at %p unregistered\n", my_kretprobe.kp.addr);

	/* nmissed > 0 suggests that maxactive was set too low. */
	pr_info("Missed probing %d instances of %s\n",
		my_kretprobe.nmissed, my_kretprobe.kp.symbol_name);
}

module_init(kretprobe_init)
module_exit(kretprobe_exit)
MODULE_LICENSE("GPL");
