// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/gpio.h> /* legacy */
#include <linux/gpio/consumer.h> /* new gpio api */
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/fs.h>

#define GPIO_LED (21) //  + GPIO_DYNAMIC_BASE)
#define MINDBLOWING_DEVICE_NAME "mindblowing_gpio_device"
#define MINDBLOWING_DRIVER_NAME "mindblowing_gpio_driver"
#define MINDBLOWING_DEVICE_MINOR 123

static struct gpio_desc *mindblowing_gpio;

static ssize_t mindblowing_write(struct file *filp, const char __user *buf,
				 size_t len, loff_t *poff)
{
	uint8_t tmp_buf[10];
	memset(tmp_buf, '\0', sizeof(tmp_buf));

	if (copy_from_user(tmp_buf, buf, len)) {
		pr_err("%s(): failed\n", __func__);
		return -EFAULT;
	}
	pr_info("%s(): gpio %d set to %c\n", __func__, GPIO_LED, tmp_buf[0]);

	if ('1' == tmp_buf[0]) {
		gpiod_set_value(mindblowing_gpio, 1);
	} else if ('0' == tmp_buf[0]) {
		gpiod_set_value(mindblowing_gpio, 0);
	} else {
		pr_err("%s(): invalid input, set gpio to '1' or '0'\n",
		       __func__);
		return -EINVAL;
	}
	return len;
}

static struct file_operations fops = {
	.write = mindblowing_write,
};

static struct miscdevice mindblowing_mdev = {
	.name = MINDBLOWING_DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int mindblowing_probe(struct platform_device *pdev)
{
	int ret;

	pr_info("%s(): called\n", __func__);

	if (misc_register(&mindblowing_mdev)) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		goto err_device;
	}
	pr_info("%s(): got minor %i\n", __func__, mindblowing_mdev.minor);

	// gpio

	/**
	 * devm_gpiod_get - Resource-managed gpiod_get()
	 * @dev:	GPIO consumer
	 * @con_id:	function within the GPIO consumer
	 * @flags:	optional GPIO initialization flags
	 *
	 * Managed gpiod_get(). GPIO descriptors returned from this function are
	 * automatically disposed on driver detach. See gpiod_get() for detailed
	 * information about behavior and return values.
	 */
	// NB: binding "mindblowing" is the prefix of the DT handle:
	//     mindblowing-leds = ...
	mindblowing_gpio = devm_gpiod_get(&pdev->dev, "mindblowing", GPIOD_OUT_LOW);

	/**
	 * gpiod_direction_output - set the GPIO direction to output
	 * @desc:	GPIO to set to output
	 * @value:	initial output value of the GPIO
	 *
	 * Set the direction of the passed GPIO to output, such as gpiod_set_value() can
	 * be called safely on it. The initial value of the output must be specified
	 * as the logical value of the GPIO, i.e. taking its ACTIVE_LOW status into
	 * account.
	 *
	 * Return 0 in case of success, else an error code.
	 */
	ret = gpiod_direction_output(mindblowing_gpio, 0);
	if (ret) {
		pr_err("%s(): failed to set gpio direction\n", __func__);
		goto err_gpio;
	}

	pr_info("%s(): done\n", __func__);
	return 0;

err_gpio:
	gpiod_put(mindblowing_gpio);

err_device:

	return -EFAULT;
}

static int mindblowing_remove(struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);

	gpiod_put(mindblowing_gpio);
	misc_deregister(&mindblowing_mdev);

	return 0;
}

static struct of_device_id mindblowing_match[] = {
    {.compatible = "lothars,gpio-led"},
    {}
};

static struct platform_driver mindblowing_driver = {
    .probe = mindblowing_probe,
    .remove = mindblowing_remove,
    .driver = {
        .name = MINDBLOWING_DRIVER_NAME,
                .of_match_table = mindblowing_match,
    }
};
module_platform_driver(mindblowing_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with GPIO descriptors");
