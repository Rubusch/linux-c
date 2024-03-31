/*
  ---
  REFERENCES:
  https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
*/

#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/gpio/consumer.h> /* new gpio api */
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>


#define GPIO_OUT_NAME "output"
#define GPIO_IN_NAME "input"

#define MINDBLOWING_DEVICE_NAME "mindblowing_gpio_device"
#define MINDBLOWING_DRIVER_NAME "mindblowing_gpio_driver"

static struct gpio_desc *mindblowing_gpio_in;
static struct gpio_desc *mindblowing_gpio_out;
static irqreturn_t gpio_irq_handler(int, void *);

// debouncing [in case prefer gpiod_debounce() ]
extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;

// led
unsigned int led_toggle = 0;

// irq - needs to be global in order to free the irq at exit()
unsigned int gpio_irq = 0;

/*
  interrupt handler for GPIO 25, to be called at each risign edge
*/
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	static unsigned long flags = 0;
	unsigned long diff = jiffies - old_jiffie; // debouncing

	pr_info("%s(): called\n", __func__);
	if (20 > diff) {
		return IRQ_HANDLED;
	}
	old_jiffie = jiffies;

	// make LED toggle on/off upon incomming interrupt event
	local_irq_save(flags);
	led_toggle = (0x01 ^ led_toggle);
	gpiod_set_value(mindblowing_gpio_out, led_toggle);
	pr_info("%s(): interrupt occurred - GPIO_OUT: %d\n",
		__func__, gpiod_get_value(mindblowing_gpio_out));
	local_irq_restore(flags);

	return IRQ_HANDLED;
}

static ssize_t mindblowing_read(struct file *filp, char __user *buf, size_t len,
				loff_t *poff)
{
	uint8_t gpio_state = 0;

	pr_info("%s(): called\n", __func__);
	gpio_state = gpiod_get_value(mindblowing_gpio_out);

	// write to user
	len = 1;
	if (copy_to_user(buf, &gpio_state, len)) {
		pr_err("%s(): failed\n", __func__);
		return -EFAULT;
	}
	pr_info("%s(): GPIO_IN: %d\n", __func__, gpio_state);

	return 0;
}

static ssize_t mindblowing_write(struct file *filp, const char __user *buf,
				 size_t len, loff_t *poff)
{
	uint8_t tmp_buf[10];

	pr_info("%s(): called\n", __func__);
	memset(tmp_buf, '\0', sizeof(tmp_buf));

	if (copy_from_user(tmp_buf, buf, len)) {
		pr_err("%s(): failed\n", __func__);
		return -EFAULT;
	}
	pr_info("%s(): GPIO_OUT set to %c\n", __func__, tmp_buf[0]);

	if ('1' == tmp_buf[0]) {
		gpiod_set_value(mindblowing_gpio_out, 1);
	} else if ('0' == tmp_buf[0]) {
		gpiod_set_value(mindblowing_gpio_out, 0);
	} else {
		pr_err("%s(): invalid input, set gpio to '1' or '0'\n",
		       __func__);
		return -EINVAL;
	}
	return len;
}

static struct file_operations fops = {
	.read = mindblowing_read,
	.write = mindblowing_write,
};

static struct miscdevice mindblowing_mdev = {
	.name = MINDBLOWING_DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int mindblowing_probe(struct platform_device* pdev)
{
	int ret;

	pr_info("%s(): called\n", __func__);

	/* device */
	if (misc_register(&mindblowing_mdev)) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		goto err_device;
	}
	pr_info("%s(): got minor %i\n", __func__, mindblowing_mdev.minor);

	/* gpio */
	mindblowing_gpio_in = devm_gpiod_get(&pdev->dev, "input", GPIOD_IN);
	ret = gpiod_direction_input(mindblowing_gpio_in);
	if (ret) {
		pr_err("%s(): failed to set gpio direction input\n", __func__);
		goto err_gpio_input;
	}

	mindblowing_gpio_out = devm_gpiod_get(&pdev->dev, "output", GPIOD_OUT_LOW);
	ret = gpiod_direction_output(mindblowing_gpio_out, 0);
	if (ret) {
		pr_err("%s(): failed to set gpio direction output\n", __func__);
		goto err_gpio_output;
	}

	/* irq */
	gpio_irq = gpiod_to_irq(mindblowing_gpio_in);

	/**
	 * devm_request_threaded_irq - allocate an interrupt line for a managed device
	 * @dev: device to request interrupt for
	 * @irq: Interrupt line to allocate
	 * @handler: Function to be called when the IRQ occurs
	 * @thread_fn: function to be called in a threaded interrupt context. NULL
	 *	    for devices which handle everything in @handler
	 * @irqflags: Interrupt type flags
	 * @devname: An ascii name for the claiming device, dev_name(dev) if NULL
	 * @dev_id: A cookie passed back to the handler function
	 *
	 * Except for the extra @dev argument, this function takes the
	 * same arguments and performs the same function as
	 * request_threaded_irq().  IRQs requested with this function will be
	 * automatically freed on driver detach.
	 *
	 * If an IRQ allocated with this function needs to be freed
	 * separately, devm_free_irq() must be used.
	 *
	 * Possible flags are:
	 * IRQF_TRIGGER_RISING
	 * IRQF_TRIGGER_FALLING
	 * IRQF_TRIGGER_HIGH
	 * IRQF_TRIGGER_LOW
	 */
	ret = devm_request_threaded_irq(&pdev->dev,
					gpio_irq,
					NULL,
					gpio_irq_handler,
					IRQF_TRIGGER_RISING |IRQF_ONESHOT,
					MINDBLOWING_DEVICE_NAME,
					NULL);
	if (ret) {
		pr_err("%s(): cannot register IRQ\n", __func__);
		goto err_gpio_output;
	}

	pr_info("%s(): ok\n", __func__);
	return 0;

err_gpio_output:
	gpiod_put(mindblowing_gpio_in);

err_gpio_input:
err_device:

	return -EFAULT;
}

static int mindblowing_remove(struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);

	gpiod_put(mindblowing_gpio_in);
	gpiod_put(mindblowing_gpio_out);
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
MODULE_DESCRIPTION("Messing with GPIO and IRQ");
