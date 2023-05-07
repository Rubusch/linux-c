/*
  Project is taken from https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

//#include <linux/jiffies.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

// device
static ssize_t mindblowing_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t mindblowing_write(struct file *, const char __user *, size_t,
				 loff_t *);

// irq
static irqreturn_t gpio_irq_handler(int, void *);

// debouncing
extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;

/*
  globals
*/

#define GPIO_OUT 21
#define GPIO_IN 25

#define GPIO_OUT_NAME "GPIO_21"
#define GPIO_IN_NAME "GPIO_25"

#define MINBLOWING_DEVICE_NAME "mindblowing_gpio_device"
#define MINBLOWING_DEVICE_MINOR 123

// led
unsigned int led_toggle = 0;

// irq - needs to be global in order to free the irq at exit()
unsigned int gpio_irq = 0;

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

/*
  interrupt handler for GPIO 25, to be called at each risign edge
*/
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	static unsigned long flags = 0;

	// debounce
	unsigned long diff = jiffies - old_jiffie;
	if (20 > diff) {
		return IRQ_HANDLED;
	}
	old_jiffie = jiffies;

	// make LED toggle on/off on incomming interrupt event
	local_irq_save(flags);
	led_toggle = (0x01 ^ led_toggle);
	gpio_set_value(GPIO_OUT, led_toggle);
	printk(KERN_INFO "interrupt occurred - GPIO_OUT: %d\n",
	       gpio_get_value(GPIO_OUT));
	local_irq_restore(flags);

	return IRQ_HANDLED;
}

static ssize_t mindblowing_read(struct file *filp, char __user *buf, size_t len,
				loff_t *poff)
{
	uint8_t gpio_state = 0;
	gpio_state = gpio_get_value(GPIO_OUT);

	// write to user
	len = 1;
	if (copy_to_user(buf, &gpio_state, len)) {
		printk(KERN_ERR "%s() failed\n", __func__);
		return -EFAULT;
	}
	printk(KERN_INFO "%s() - GPIO_IN: %d\n", __func__, gpio_state);

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
	printk(KERN_INFO "%s() - GPIO_OUT [%d] set to %c\n", __func__, GPIO_OUT,
	       tmp_buf[0]);

	if ('1' == tmp_buf[0]) {
		gpio_set_value(GPIO_OUT, 1);
	} else if ('0' == tmp_buf[0]) {
		gpio_set_value(GPIO_OUT, 0);
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

	/* GPIO_OUT */

	/**
	 * "valid" GPIO numbers are nonnegative and may be passed to
	 * setup routines like gpio_request().  only some valid numbers
	 * can successfully be requested and used.
	 *
	 * Invalid GPIO numbers are useful for indicating no-such-GPIO in
	 * platform data and other tables.
	 */
	if (!gpio_is_valid(GPIO_OUT)) {
		printk(KERN_ERR "%s() - gpio %d is not valid\n", __func__,
		       GPIO_OUT);
		goto err_gpio_out;
	}

	if (0 > gpio_request(GPIO_OUT, GPIO_OUT_NAME)) {
		printk(KERN_ERR "%s() - gpio_request() failed\n", __func__);
		goto err_gpio_out;
	}

	/**
	 * set direction to "output"
	 */
	gpio_direction_output(GPIO_OUT, 0);

	/**
	 * Using this call the GPIO 21 will be visible in /sys/class/gpio/
	 * Now you can change the gpio values by using below commands also.
	 * # echo 1 > /sys/class/gpio/gpio21/value   (turn ON the LED)
	 * # echo 0 > /sys/class/gpio/gpio21/value   (turn OFF the LED)
	 * # cat /sys/class/gpio/gpio21/value        (read the value LED)
	 *
	 * the second argument prevents the direction from being changed.
	 */
	gpio_export(GPIO_OUT, false);

	/* GPIO_IN */

	if (!gpio_is_valid(GPIO_IN)) {
		printk(KERN_ERR "%s() - gpio %d is not valid\n", __func__,
		       GPIO_IN);
		goto err_gpio_in;
	}

	if (0 > gpio_request(GPIO_IN, GPIO_IN_NAME)) {
		printk(KERN_ERR "%s() - gpio_request() failed\n", __func__);
		goto err_gpio_in;
	}

	gpio_direction_input(GPIO_IN);

	// commented out, since debouncing is not supported on Raspi4
	//	if (0 > gpio_set_debounce(GPIO_IN, 200)) {
	//		printk(KERN_ERR "%s() - gpio_set_debounce() failed\n", __func__);
	//		goto err_gpio_in;
	//	}

	gpio_irq = gpio_to_irq(GPIO_IN);
	printk(KERN_INFO "gpio_irq = %d\n", gpio_irq);

	/**
	 * request_irq - Add a handler for an interrupt line
	 * @irq:The interrupt line to allocate
	 * @handler:Function to be called when the IRQ occurs.
	 * Primary handler for threaded interrupts
	 * If NULL, the default primary handler is installed
	 * @flags:Handling flags
	 * @name:Name of the device generating this interrupt
	 * @dev:A cookie passed to the handler function
	 *
	 * This call allocates an interrupt and establishes a handler; see
	 * the documentation for request_threaded_irq() for details.
	 *
	 * possible flags are:
	 * IRQF_TRIGGER_RISING
	 * IRQF_TRIGGER_FALLING
	 * IRQF_TRIGGER_HIGH
	 * IRQF_TRIGGER_LOW
	 */
	if (request_irq(gpio_irq, (void *)gpio_irq_handler, IRQF_TRIGGER_RISING,
			MINBLOWING_DEVICE_NAME, NULL)) {
		printk(KERN_ERR "%s() - cannot register IRQ\n", __func__);
		goto err_gpio_in;
	}

	printk(KERN_INFO "%s() done\n", __func__);
	return 0;

err_gpio_in:
	gpio_free(GPIO_OUT);

err_gpio_out:
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
	// irq
	free_irq(gpio_irq, NULL);

	// GPIO_IN
	gpio_free(GPIO_IN);

	// GPIO_OUT
	gpio_free(GPIO_OUT);

	// mind-blowing device
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
