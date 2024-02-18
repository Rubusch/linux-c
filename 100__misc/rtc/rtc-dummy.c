// SPDX-License-Identifier: GPL-2.0+
/*
*/
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/err.h>
#include <linux/rtc.h>
#include <linux/of.h>

#define PROBED_MODULE_NAME "lothars-rtc-dummy"
static struct timespec64 begin_time;

// dummy for fake rtc chip emulation
static time64_t time = 0;

// Helper: convert timespec to integer type
static inline unsigned long timespec_to_ulong(struct timespec64 *ts)
{
	return ts->tv_nsec < NSEC_PER_SEC/2 ? ts->tv_sec : ts->tv_sec + 1;
}

/*
  Basically a "renamer" for the called function to interpret what
  happens here.
*/
static inline void get_uptime(struct timespec64* ts)
{
	ktime_get_raw_ts64(ts); // returns raw monotonic time
}

static int rtcdrv_read_time(struct device *dev, struct rtc_time *tm)
{
	struct timespec64 now, diff;

	pr_info("%s(): called\n", __func__);
	get_uptime(&now);
	diff = timespec64_sub(now, begin_time);
	rtc_time64_to_tm(time + timespec_to_ulong(&diff), tm);

	return rtc_valid_tm(tm);
}

static int rtcdrv_set_time(struct device *dev, struct rtc_time *tm)
{
	pr_info("%s(): called\n", __func__);
	get_uptime(&begin_time);
	time = rtc_tm_to_time64(tm);
	return 0;
}

static const struct rtc_class_ops rtcdrv_ops = {
	.read_time = rtcdrv_read_time,
	.set_time = rtcdrv_set_time,
//	.read_alarm = rtcdrv_read_alarm,
//	.set_alarm = rtcdrv_set_alarm,
//	.proc = rtcdrv_procfs,
};

static const struct of_device_id rtcdrv_ids[] = {
    { .compatible = "lothars,rtc-dummy", },
    { },
};

static int pdrv_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	pr_info("%s(): called\n", __func__);

	// first, get time (or check if rtc device is useable,...)
	get_uptime(&begin_time);

	rtc = devm_rtc_allocate_device(&pdev->dev);
	if (IS_ERR(rtc))
		return PTR_ERR(rtc);

	rtc->ops = &rtcdrv_ops;

	platform_set_drvdata(pdev, rtc);
	dev_info(&pdev->dev, "%s(): begin_time is %lu, rtc_time is %llu\n",
			__func__, timespec_to_ulong(&begin_time), time);

	/*
	  Things like feature configs, wakeup, interrupt settings may
	  happen here:

	      clear_bit(RTC_FEATURE_UPDATE_INTERRUPT, rtc->features);
	      clear_bit(RTC_FEATURE_ALARM, rtc->features);
	      device_init_wakeup(&pdev->dev, true);
	      ...
	*/
	return devm_rtc_register_device(rtc);
}

static int __exit pdrv_remove(struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);
	device_init_wakeup(&pdev->dev, 0);
	return 0;
}

static struct platform_driver pdrv_driver = {
	.probe = pdrv_probe,
	.remove = pdrv_remove,
	.driver = {
		.name = PROBED_MODULE_NAME,
		.of_match_table = of_match_ptr(rtcdrv_ids),
	},
};
module_platform_driver(pdrv_driver);

/*
  NB: with DT binding, avoid registering pdrv_probe() and
  pdrv_remove(), init the platform_device with the following macro:
      module_platform_driver_probe(pdrv_driver, pdrv_probe);
// */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with rtc drivers.");
