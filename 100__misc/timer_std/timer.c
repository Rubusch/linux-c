// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello(void);
void cleanup_hello(void);

void timer_callback(struct timer_list *);

// timer (in milli secs)
#define TIMEOUT 500
static struct timer_list lothars_timer;

static unsigned int counter = 0;

void timer_callback(struct timer_list *data)
{
	int ret;

	pr_info("%s(): called\n", __func__);
	pr_info("%s(): %d\n", __func__, counter++);
	/**
	 * mod_timer - modify a timer's timeout
	 * @timer: the timer to be modified
	 * @expires: new timeout in jiffies
	 *
	 * mod_timer() is a more efficient way to update the expire field of an
	 * active timer (if the timer is inactive it will be activated)
	 *
	 * mod_timer(timer, expires) is equivalent to:
	 *
	 *     del_timer(timer); timer->expires = expires; add_timer(timer);
	 *
	 * Note that if there are multiple unserialized concurrent users of the
	 * same timer, then mod_timer() is the only safe way to modify the timeout,
	 * since add_timer() cannot modify an already running timer.
	 *
	 * The function returns whether it has modified a pending timer or not.
	 * (ie. mod_timer() of an inactive timer returns 0, mod_timer() of an
	 * active timer returns 1.)
	 */
	// re-enable timer - working as a periodic timer when reenabled here
	ret = mod_timer(&lothars_timer, jiffies + msecs_to_jiffies(TIMEOUT));
	if (ret) {
		pr_err("%s(): mod_timer() failed\n", __func__);
	}
}

static int __init mod_init(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);

	/**
	 * timer_setup - prepare a timer for first use
	 * @timer: the timer in question
	 * @callback: the function to call when timer expires
	 * @flags: any TIMER_* flags
	 *
	 * Regular timer initialization should use either DEFINE_TIMER() above,
	 * or timer_setup(). For timers on the stack, timer_setup_on_stack() must
	 * be used and must be balanced with a call to destroy_timer_on_stack().
	 */
	timer_setup(&lothars_timer, timer_callback, 0);
        // alternative is setup_timer()

	/**
	 * msecs_to_jiffies: - convert milliseconds to jiffies
	 * @m:time in milliseconds
	 *
	 * conversion is done as follows:
	 *
	 * - negative values mean 'infinite timeout' (MAX_JIFFY_OFFSET)
	 *
	 * - 'too large' values [that would result in larger than
	 *   MAX_JIFFY_OFFSET values] mean 'infinite timeout' too.
	 *
	 * - all other values are converted to jiffies by either multiplying
	 *   the input value by a factor or dividing it with a factor and
	 *   handling any 32-bit overflows.
	 *   for the details see __msecs_to_jiffies()
	 *
	 * msecs_to_jiffies() checks for the passed in value being a constant
	 * via __builtin_constant_p() allowing gcc to eliminate most of the
	 * code, __msecs_to_jiffies() is called if the value passed does not
	 * allow constant folding and the actual conversion must be done at
	 * runtime.
	 * the HZ range specific helpers _msecs_to_jiffies() are called both
	 * directly here and from __msecs_to_jiffies() in the case where
	 * constant folding is not possible.
	 */
	ret = mod_timer(&lothars_timer, jiffies + msecs_to_jiffies(TIMEOUT));
	if (ret) {
		pr_err("%s(): mod_timer() failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static void __exit mod_exit(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);
	/**
	 * del_timer - deactivate a timer.
	 * @timer: the timer to be deactivated
	 *
	 * del_timer() deactivates a timer - this works on both active and inactive
	 * timers.
	 *
	 * The function returns whether it has deactivated a pending timer or not.
	 * (ie. del_timer() of an inactive timer returns 0, del_timer() of an
	 * active timer returns 1.)
	 */
	ret = del_timer(&lothars_timer);
	if (ret) {
		pr_err("%s(): del_timer() failed\n", __func__);
	}

	pr_info("%s(): READY.\n", __func__);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates kernel timers.");
