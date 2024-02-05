// SPDX-License-Identifier: GPL-2.0+
/*
  uses the gpio lib, at least partly deprecated
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>


static dev_t dev;
static struct class *dev_class;
static struct cdev hello_cdev;

#define DRIVER_NAME "lothars_gpio_driver"
#define DRIVER_CLASS "lothars_gpio_class"
#define GPIO_LED 4

static ssize_t
driver_write(struct file *File, const char *user_buffer,
			    size_t count, loff_t *offs) {
	int size, not_written, nwritten;
	char value;

	pr_info("%s(): called", __func__);
	size = min(count, sizeof(value));
	not_written = copy_from_user(&value, user_buffer, size);
	switch (value) {
		case '0':
			gpio_set_value(GPIO_LED, 0);
			break;
		case '1':
			gpio_set_value(GPIO_LED, 1);
			break;
		default:
			pr_warn("%s(): input invalid", __func__);
			break;
	}
	nwritten = size - not_written;

	return nwritten;
}

static int
driver_open(struct inode *device_file, struct file *instance) {
	pr_info("%s(): called", __func__);
	return 0;
}

static int
driver_close(struct inode *device_file, struct file *instance) {
	pr_info("%s(): called", __func__);
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.write = driver_write
};

static int __init mod_init(void) {
	pr_info("%s(): called", __func__);

	// alloc device number
	if (0 > alloc_chrdev_region(&dev, 0, 1, DRIVER_NAME)) {
		pr_err("%s(): alloc_chrdev_region() failed", __func__);
		return -ENOMEM;
	}
	pr_info("%s() major = %d, minor = %d", __func__, MAJOR(dev), MINOR(dev));

	cdev_init(&hello_cdev, &fops);

	dev_class = class_create(THIS_MODULE, DRIVER_CLASS);
	if (NULL == dev_class) {
		pr_err("%s(): class_create() failed", __func__);
		goto err_class;
	}

	if (NULL == device_create(dev_class, NULL, dev, NULL, DRIVER_NAME)) {
		pr_err("%s(): device_create() failed", __func__);
		goto err_device;
	}

	if (-1 == cdev_add(&hello_cdev, dev, 1)) {
		pr_err("%s(): cdev_add() failed", __func__);
		goto err_cdev;
	}

	// gpio output
	if (gpio_request(GPIO_LED, "rpi-gpio-led")) {
		pr_err("%s(): cannot allocate gpio %d", __func__, GPIO_LED);
		goto err_cdev;
	}

	if (gpio_direction_output(GPIO_LED, 0)) {
		pr_err("%s(): gpio %d - cannot sed direction to out!",
		       __func__, GPIO_LED);
		goto err_gpio_output;
	}

	return 0;

err_gpio_output:
	gpio_free(GPIO_LED);
err_cdev:
	cdev_del(&hello_cdev);
	device_destroy(dev_class, dev);
err_device:
	class_destroy(dev_class);
err_class:
	unregister_chrdev_region(dev, 1);
	return -1;
}

static void __exit mod_exit(void) {
	gpio_set_value(GPIO_LED, 0);
	gpio_free(GPIO_LED);
	cdev_del(&hello_cdev);
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev, 1);
	pr_info("%s(): done.", __func__);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with gpios");
