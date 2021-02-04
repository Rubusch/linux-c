/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>


/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello(void);
void cleanup_hello(void);

// timer callback
enum hrtimer_restart timer_callback(struct hrtimer *);


/*
  globals
*/

// timer (in nano secs)
#define TIMEOUT 5000 * 1000000L
static struct hrtimer lothars_timer;

// data (demo)
static unsigned int counter = 0;


/*
  implementation
*/
enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
	printk(KERN_INFO "%s() - %d\n", __func__, counter++);

	/**
	 * hrtimer_forward_now - forward the timer expiry so it expires after now
	 * @timer:hrtimer to forward
	 * @interval:the interval to forward
	 *
	 * Forward the timer expiry so it will expire after the current time
	 * of the hrtimer clock base. Returns the number of overruns.
	 *
	 * Can be safely called from the callback function of @timer. If
	 * called from other contexts @timer must neither be enqueued nor
	 * running the callback and the caller needs to take care of
	 * serialization.
	 *
	 * Note: This only updates the timer expiry value and does not requeue
	 * the timer.
	 */
	hrtimer_forward_now(timer, ktime_set(0, TIMEOUT));

	/**
	   Possible values are:

	   HRTIMER_NORESTART   // Timer is not restarted
	   HRTIMER_RESTART     // Timer must be restarted
	 */
	return HRTIMER_RESTART;
}


/*
  start / stop module
*/

int init_hello(void)
{
	ktime_t ktime;

	printk(KERN_INFO "%s() - initializing...\n", __func__);

	/**
	 * ktime_set - Set a ktime_t variable from a seconds/nanoseconds value
	 * @secs:seconds to set
	 * @nsecs:nanoseconds to set
	 *
	 * Return: The ktime_t representation of the value.
	 */
	ktime = ktime_set(0, TIMEOUT);

	/**
	 * hrtimer_init - initialize a timer to the given clock
	 * @timer:the timer to be initialized
	 * @clock_id:the clock to be used
	 * @mode:       The modes which are relevant for intitialization:
	 *              HRTIMER_MODE_ABS, HRTIMER_MODE_REL, HRTIMER_MODE_ABS_SOFT,
	 *              HRTIMER_MODE_REL_SOFT
	 *
	 *              The PINNED variants of the above can be handed in,
	 *              but the PINNED bit is ignored as pinning happens
	 *              when the hrtimer is started
	 */
	hrtimer_init(&lothars_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	lothars_timer.function = &timer_callback;

	/**
	 * hrtimer_start - (re)start an hrtimer
	 * @timer:the timer to be added
	 * @tim:expiry time
	 * @mode:timer mode: absolute (HRTIMER_MODE_ABS) or
	 *relative (HRTIMER_MODE_REL), and pinned (HRTIMER_MODE_PINNED);
	 *softirq based mode is considered for debug purpose only!
	 */
	hrtimer_start(&lothars_timer, ktime, HRTIMER_MODE_REL);

	return 0;
}

void cleanup_hello(void)
{
	/**
	 * hrtimer_cancel - cancel a timer and wait for the handler to finish.
	 * @timer:the timer to be cancelled
	 *
	 * Returns:
	 *  0 when the timer was not active
	 *  1 when the timer was active
	 */
	hrtimer_cancel(&lothars_timer);

	printk(KERN_INFO "%s() - READY.\n", __func__);
}


/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_hello();
}

static void __exit mod_exit(void)
{
	cleanup_hello();
}

module_init(mod_init);
module_exit(mod_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates kernel high resolution timers.");
