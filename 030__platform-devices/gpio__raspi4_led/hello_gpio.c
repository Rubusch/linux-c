/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/fs.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

// device
static ssize_t mindblowing_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t mindblowing_write(struct file *, const char __user *, size_t,
				 loff_t *);

/*
  globals
*/

#define GPIO_LED 21
#define GPIO_LED_NAME "GPIO_21"

#define MINBLOWING_DEVICE_NAME "mindblowing_gpio_device"
#define MINBLOWING_DEVICE_MINOR 123

// device
dev_t dev = 0;
static struct class *dev_class;
static struct cdev mindblowing_cdev;

// device fops
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = mindblowing_read,
	.write = mindblowing_write,
};

/*
  implementation
*/

static ssize_t mindblowing_read(struct file *filp, char __user *buf, size_t len,
				loff_t *poff)
{
	uint8_t gpio_state = 0;
	gpio_state = gpio_get_value(GPIO_LED);

	// write to user
	len = 1;
	if (copy_to_user(buf, &gpio_state, len)) {
		printk(KERN_ERR "%s() failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static ssize_t mindblowing_write(struct file *filp, const char __user *buf,
				 size_t len, loff_t *poff)
{
	uint8_t tmp_buf[10];
	memset(tmp_buf, '\0', sizeof(tmp_buf));

	if (copy_from_user(tmp_buf, buf, len)) {
		printk(KERN_ERR "%s() failed\n", __func__);
		return -EFAULT;
	}
	printk(KERN_INFO "%s() gpio %d set to %c\n", __func__, GPIO_LED,
	       tmp_buf[0]);

	if ('1' == tmp_buf[0]) {
		gpio_set_value(GPIO_LED, 1);
	} else if ('0' == tmp_buf[0]) {
		gpio_set_value(GPIO_LED, 0);
	} else {
		printk(KERN_ERR "%s() invalid input, set gpio to '1' or '0'\n",
		       __func__);
		return -EINVAL;
	}
	return len;
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	if (0 > alloc_chrdev_region(&dev, 0, MINBLOWING_DEVICE_MINOR,
				    MINBLOWING_DEVICE_NAME)) {
		printk(KERN_ERR "alloc_chrdev_region() failed\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "%s() - major %d, minor %d\n", __func__, MAJOR(dev),
	       MINOR(dev));

	cdev_init(&mindblowing_cdev, &fops);

	if (0 > cdev_add(&mindblowing_cdev, dev, 1)) {
		printk(KERN_ERR "%s() - cdev_add() failed\n", __func__);
		goto err_cdev;
	}

	dev_class = class_create(THIS_MODULE, MINBLOWING_DEVICE_NAME);
	if (!dev_class) {
		printk(KERN_ERR "%s() - class_create() failed\n", __func__);
		goto err_class;
	}

	if (!device_create(dev_class, NULL, dev, NULL,
			   MINBLOWING_DEVICE_NAME)) {
		printk(KERN_ERR "%s() - device_create() failed\n", __func__);
		goto err_device;
	}

	// gpio

	/**
	 * "valid" GPIO numbers are nonnegative and may be passed to
	 * setup routines like gpio_request().  only some valid numbers
	 * can successfully be requested and used.
	 *
	 * Invalid GPIO numbers are useful for indicating no-such-GPIO in
	 * platform data and other tables.
	 */
	if (!gpio_is_valid(GPIO_LED)) {
		printk(KERN_ERR "%s() - gpio %d is not valid\n", __func__,
		       GPIO_LED);
		goto err_gpio;
	}

	if (0 > gpio_request(GPIO_LED, GPIO_LED_NAME)) {
		printk(KERN_ERR "%s() - gpio_request() failed\n", __func__);
		goto err_gpio;
	}

	/**
	 * set direction to "output"
	 */
	gpio_direction_output(GPIO_LED, 0);

	/**
	 * Using this call the GPIO 21 will be visible in /sys/class/gpio/
	 * Now you can change the gpio values by using below commands also.
	 * # echo 1 > /sys/class/gpio/gpio21/value   (turn ON the LED)
	 * # echo 0 > /sys/class/gpio/gpio21/value   (turn OFF the LED)
	 * # cat /sys/class/gpio/gpio21/value        (read the value LED)
	 *
	 * the second argument prevents the direction from being changed.
	 */
	gpio_export(GPIO_LED, false);

	printk(KERN_INFO "%s() done\n", __func__);
	return 0;

err_gpio:
	device_destroy(dev_class, dev);

err_device:
	class_destroy(dev_class);

err_class:
	cdev_del(&mindblowing_cdev);

err_cdev:
	unregister_chrdev_region(dev, 1);

	return -EFAULT;
}

static void __exit mod_exit(void)
{
	gpio_unexport(GPIO_LED);
	gpio_free(GPIO_LED);

	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&mindblowing_cdev);
	unregister_chrdev_region(dev, 1);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates a GPIO driver!");
