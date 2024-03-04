// SPDX-License-Identifier: GPL-2.0+
/*
  Operate the pwm device struct, w/o any hardware connected,
  just as demo
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pwm.h>
#include <linux/miscdevice.h>

#define DEV_NAME "happy_pwm"
#define PWM_ID 1

struct pwm_device *pwm0 = NULL;
u32 pwm_on_time = 500000000;

static ssize_t
driver_write(struct file *file, const char *ubuf,
	     size_t count, loff_t *offs)
{
	char val;
	pr_info("%s(): called\n", __func__);
	if (copy_from_user(&val, ubuf, count)) {
		pr_err("%s(): failed to copy content\n", __func__);
		return -EFAULT;
	}
	if (val < 'a' || val > 'j')
		pr_warn("%s(): invalid\n", __func__);
	else
		pwm_config(pwm0, 100000000 * (val - 'a'), 1000000000);

	return count;
}

static struct file_operations fops = {
	.write = driver_write
};

static struct miscdevice pwm_dev = {
	.name = DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);
	ret = misc_register(&pwm_dev);
	if (0 != ret) {
		pr_err("%s(): could not register the miscdevice\n", __func__);
		return -EFAULT;
	}

	// This is a DUMMY!!!
	// NB: it probably will just hang when loaded!!!
	pwm0 = pwm_request(PWM_ID, "pwm-backlight"); // a classic usage: backlight
	if (pwm0 == NULL) {
		pr_err("%s(): faild to request pwm-backlight\n", __func__);
		return -EFAULT;
	}

	pwm_config(pwm0, pwm_on_time, 1000000000);
	pwm_enable(pwm0);

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	pwm_disable(pwm0);
	pwm_free(pwm0);
	misc_deregister(&pwm_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with hw dummy devices.");
