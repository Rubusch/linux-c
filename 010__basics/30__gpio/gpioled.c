// SPDX-License-Identifier: GPL-2.0+
/*
  uses the gpio lib

  NB: since 4.8 there is a "new" gpio implementation in the kernel
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>

#define DEVICE_NAME "lothars_gpio_driver"
#define GPIO_LED 4

static ssize_t
driver_write(struct file *file, const char *ubuf,
			    size_t count, loff_t *offs) {
	int size, not_written, nwritten;
	char value;

	pr_info("%s(): called\n", __func__);
	size = min(count, sizeof(value));
	not_written = copy_from_user(&value, ubuf, size);
	switch (value) {
		case '0':
			gpio_set_value(GPIO_LED, 0);
			break;
		case '1':
			gpio_set_value(GPIO_LED, 1);
			break;
		default:
			pr_warn("%s(): input invalid\n", __func__);
			break;
	}
	nwritten = size - not_written;

	return nwritten;
}

static struct file_operations fops = {
	.write = driver_write
};

static struct miscdevice gpio_dev = {
	.name = DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void) {
	pr_info("%s(): called\n", __func__);

	misc_register(&gpio_dev);

	if (gpio_request(GPIO_LED, "rpi-gpio-led")) {
		pr_err("%s(): failed to allocate gpio %d\n", __func__, GPIO_LED);
		return -EFAULT;
	}

	if (gpio_direction_output(GPIO_LED, 0)) {
		pr_err("%s(): gpio %d - faild to set gpio direction\n",
		       __func__, GPIO_LED);
		return -EFAULT;
	}

	return 0;
}

static void __exit mod_exit(void) {
	pr_info("%s(): called\n", __func__);
	gpio_set_value(GPIO_LED, 0);
	gpio_free(GPIO_LED);

	misc_deregister(&gpio_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with gpios");
