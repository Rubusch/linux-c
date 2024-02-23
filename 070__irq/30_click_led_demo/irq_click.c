/*
  Keyled class demo
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>  /* include wait queue */

#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/kthread.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/of_device.h>

#define LED_NAME_LEN     32
#define INT_NUMBER        2
static const char *LOTHARS_KEY_NAME1 = "MIKROBUS_KEY1";
static const char *LOTHARS_KEY_NAME2 = "MIKROBUS_KEY2";

/* Specific led private structure

   The led_desc hoslds the gpio descriptor of each specific pin
   connected to one of the three leds.

   The keyled_priv structure will hold the global data used for all
   the led devices.
 */
struct led_device {
	char name[LED_NAME_LEN];
	struct gpio_desc *led_desc; // each led gpio_desc
	struct device *dev;
	struct keyled_priv *private; // pointer to the global private struct
};

/* Global private structure */
struct keyled_priv {
	u32 num_leds;
	u8 led_flag;
	u8 task_flag;
	u32 period;
	spinlock_t period_lock;
	struct task_struct *task;  // kthread task_struct
	struct class *led_class;   // the keyled class
	struct device *dev;
	dev_t led_devt;            // first device identifyer
	struct led_device *leds[]; // pointer to each led private struct
};

/* The kthread function

   Recover the led_device structure that was set as a parameter in the
   kthread_run() function. The kthread_should_stop() returns a
   non-zero value if there is a stop request submitted by the
   kthread_stop(). It will blink until it is exited.
 */
static int led_flash(void *data) {
	unsigned long flags;
	u32 value = 0, period = 0;
	struct led_device *led_dev = data;
	struct device *dev = led_dev->dev;

	dev_info(dev, "%s() - called", __func__);
	while (!kthread_should_stop()) {
		spin_lock_irqsave(&led_dev->private->period_lock, flags);
		period = led_dev->private->period;
		spin_unlock_irqrestore(&led_dev->private->period_lock, flags);

		value = !value;  // blink
		gpiod_set_value(led_dev->led_desc, value);
		msleep(period / 2); // NB: in case other sleep/delay might be better here
	}

	gpiod_set_value(led_dev->led_desc, 1);  // switch led off
	dev_info(dev, "%s() - done", __func__);
	return 0;
};

/*
  sysfs methods
*/

/* Switch each led on/off

   Receive two parameters, "on" or "off" from the user space. Each
   led_device structure is recovered by using the dev_get_drvdata()
   function.

   After having set up the connection between each led device and its
   led_device structure by using the dev_set_drvdata() in the probe(),
   the led_device_register() is called also within the probe(). If
   there is a running kthread, it will be stopped.

   If the parameter received is a "on", the specific led is switched
   on by previously switching all the other leds off. This is
   performed by calling gpiod_set_value(). Similarily vice-versa if
   "off" was called.
 */
static ssize_t
set_led_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int idx;
	char buffer[8];
	struct led_device *led_count;
	struct led_device *led = dev_get_drvdata(dev);

	dev_info(dev, "%s() - called", __func__);

	// replace \n added from the terminal
	memset(buffer, '\0', sizeof(buffer));
	strncpy(buffer, buf, count);
	*(buffer+(count-1)) = '\0';

	// in case of running kthread, stop it
	if (led->private->task_flag == 1) {
		kthread_stop(led->private->task);
		led->private->task_flag = 0;
	}
	dev_info(dev, "%s() - buffer: '%s'", __func__ , buffer);

	if (!strcmp(buffer, "on")) {
		if (led->private->led_flag == 1) {
			for (idx = 0; led->private->num_leds; idx++) {
				led_count = led->private->leds[idx];
				gpiod_set_value(led_count->led_desc, 1);
			}
			gpiod_set_value(led->led_desc, 0);
		} else {
			gpiod_set_value(led->led_desc, 0);
			led->private->led_flag = 1;
		}
	} else if (!strcmp(buffer, "off")) {
		gpiod_set_value(led->led_desc, 1);
	} else {
		dev_info(led->dev, "%s() - bad led value", __func__);
		return -EINVAL;
	}

	return count;
}
static DEVICE_ATTR_WO(set_led);

/* Blinking on the specific led running a kthread

   When receiving the "on" parameter from the userspace, all the leds
   will be switched off, then if there is no kthread running, it will
   be started a new one that will blink one of the leds with a
   specific period. Already running kthreads will be exited.
 */
static ssize_t
blink_on_led_store(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t count)
{
	int idx;
	char buffer[8];
	struct led_device *led_count;
	struct led_device *led = dev_get_drvdata(dev);

	dev_info(dev, "%s() - called", __func__);

	// replace \n added from the terminal
	memset(buffer, '\0', sizeof(buffer));
	strncpy(buffer, buf, count);
	*(buffer+(count-1)) = '\0';

	// in case of running kthread, stop it
	if (1 == led->private->led_flag) {
		for (idx = 0; idx < led->private->num_leds; idx++) {
			led_count = led->private->leds[idx];
			gpiod_set_value(led_count->led_desc, 1);
		}
	}
	dev_info(dev, "%s() - buffer: '%s'", __func__ , buffer);

	if (!strcmp(buffer, "on")) {
		if (0 == led->private->task_flag) {
			led->private->task = kthread_run(led_flash, led, "led_flash_thread");
			if (IS_ERR(led->private->task)) {
				dev_info(led->dev, "%s() - failed to create the task",
					 __func__);
				return PTR_ERR(led->private->task);
			}
		} else {
			return -EBUSY;
		}
	} else {
		dev_info(led->dev, "%s() - bad led value", __func__);
		return -EINVAL;
	}

	led->private->task_flag = 1;
	dev_info(led->dev, "%s() - done", __func__);

	return count;
}
static DEVICE_ATTR_WO(blink_on_led);

/* Switch off the blinking of any led

   When recieving an "off" from the userspace, blinking will be
   stopped. Running kthreads are exited.
 */
static ssize_t
blink_off_led_store(struct device *dev, struct device_attribute *attr,
		    const char *buf, size_t count)
{
	int idx;
	char buffer[8];
	struct led_device *led = dev_get_drvdata(dev);
	struct led_device *led_count;

	dev_info(dev, "%s() - called", __func__);

	// replace \n added from the terminal
	memset(buffer, '\0', sizeof(buffer));
	strncpy(buffer, buf, count);
	*(buffer+(count-1)) = '\0';
	dev_info(dev, "%s() - buffer: '%s'", __func__ , buffer);

	if (!strcmp(buffer, "off")) {
		if (1 == led->private->task_flag) {
			kthread_stop(led->private->task);
			for (idx = 0; idx < led->private->num_leds; idx++) {
				led_count = led->private->leds[idx];
				gpiod_set_value(led_count->led_desc, 1);
			}
		} else {
			return 0;
		}
	} else {
		dev_info(led->dev, "%s() - bad led value", __func__);
		return -EINVAL;
	}

	led->private->task_flag = 0;
	return count;
}
static DEVICE_ATTR_WO(blink_off_led);

/* set the blinking period */
static ssize_t
set_period_store(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t count)
{
	unsigned long flags;
	int ret, period;
	struct led_device *led = dev_get_drvdata(dev);

	dev_info(led->dev, "%s() - called", __func__);
	ret = sscanf(buf, "%u", &period);

	if (ret < 1 || period < 10 || period > 10000) {
		dev_err(dev, "%s() - invalid value", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&led->private->period_lock, flags);
	led->private->period = period;
	spin_unlock_irqrestore(&led->private->period_lock, flags);

	dev_info(led->dev, "%s() - period is set", __func__);
	return count;
}
static DEVICE_ATTR_WO(set_period);

/*
  Declare the sysfs structure

  In the probe(), a group of "sysfs atribute files" needs to be set to
  control each LED, with the line of code

    priv->led_class->dev_groups = led_groups
 */
static struct attribute *led_attrs[] = {
	&dev_attr_set_led.attr,
	&dev_attr_blink_on_led.attr,
	&dev_attr_blink_off_led.attr,
	&dev_attr_set_period.attr,
	NULL,
};
static const struct attribute_group led_group = {
	.attrs = led_attrs,
};
static const struct attribute_group *led_groups[] = {
	&led_group,
	NULL,
};

/*
  Allocate space for the global private struct and the 3 led private
  structs
*/
static inline int
sizeof_keyled_priv(int num_leds)
{
	return sizeof(struct keyled_priv) + (sizeof(struct led_device*) * num_leds);
}

/* First interrupt handler - async atomic context

   Recover the global private structure from the isr data
   argument. Increase the period variable by ten.
 */
static irqreturn_t
KEY_ISR1(int irq, void* data)
{
	struct keyled_priv *priv = data;
	dev_info(priv->dev, "%s() - interrupt MIKROBUS_KEY1 received, key: %s",
		 __func__, LOTHARS_KEY_NAME1);

	// inside the ISR use spin_locks to lock resources
	spin_lock(&priv->period_lock);
	priv->period = priv->period + 10;
	if ((10 > priv->period) || (10000 < priv->period)) {
		priv->period = 10;
	}
	spin_unlock(&priv->period_lock);

	dev_info(priv->dev, "%s() - the led period is %d",
		__func__, priv->period);

	return IRQ_HANDLED;
}

/* Second interrupt handler - async atomic context

   Here period is decreased by 10.
 */
static irqreturn_t
KEY_ISR2(int irq, void *data)
{
	struct keyled_priv *priv = data;

	dev_info(priv->dev, "%s() - interrupt MIKROBUS_KEY2 received, key %s",
		 __func__, LOTHARS_KEY_NAME2);

	spin_lock(&priv->period_lock);
	priv->period = priv->period - 10;
	if ((10 > priv->period) || (10000 < priv->period)) {
		priv->period = 10;
	}
	spin_unlock(&priv->period_lock);

	dev_info(priv->dev, "%s() - the led period is %d",
		 __func__, priv->period);

	return IRQ_HANDLED;
}

/* create the led devices under the sysfs keyled entry */
struct led_device*
led_device_register(const char *name, int count, struct device *parent,
		    dev_t led_devt, struct class *led_class)
{
	struct led_device *led;
	dev_t devt;
	int ret;

	// first allocate a new led device
	led = devm_kzalloc(parent, sizeof(struct led_device), GFP_KERNEL);
	if (!led) {
		return ERR_PTR(-ENOMEM);
	}

	// get the minor number of each device
	devt = MKDEV(MAJOR(led_devt), count);

	// create the device
	led->dev = device_create(led_class, parent, devt, led, "%s", name);
	if (IS_ERR(led->dev)) {
		ret = PTR_ERR(led->dev);
		return ERR_PTR(ret);
	}

	dev_info(led->dev, "%s() - major: %d",
		 __func__, MAJOR(led_devt));

	dev_info(led->dev, "%s() - minor: %d",
		 __func__, MINOR(devt));

	// init the device's data to recover later from each sysfs entry
	dev_set_drvdata(led->dev, led);
	strncpy(led->name, name, LED_NAME_LEN);
	dev_info(led->dev, "%s() - led %s added",
		 __func__, led->name);

	return led;
}

/* The probe()
   - Declare a pointer to a fwnode_handle structure and a pointer to
     the global private keyled_priv structure
   - Get the number of leds and interrupt devices by using the
     device_get_child_node_count(), five devices must be returned here
   - Allocate the private structures by calling devm_kzalloc() for the
     global structure and for three pointers to the led_devices
     structure
   - Allocate three device numbers with alloc_chrdev_region() and
     create keyled class with class_create()
   - Initiazlie a spinlock by using spin_lock_init(); the spinlock
     will be used to protect access to the period variable. Use
     spin_lock_irqsave() in usermode and spin_lock() inside the isr,
     when using smp architectures (e.g. for uniprocessor
     architectures, e.g. SAMA5D2, this is not needed).
   - The device_for_each_child_of_node() function walks for each child
     node, creating a sysfs device entry under /sys/class/keyled/ for
     each found led device (see the led_device_register() function).

     Official explanation given:
     You will get the GPIO descriptor of each led pin declared inside
     each led DT node by using the devm_get_gpiod_from_child()
     function, then you will set the direction to output by calling
     gpiod_direction_output(). The GPIO descriptor of each INT pin
     declared inside each key dt node is obtained by using the
     devm_get_gpiod_from_child() function, then the direction of the
     pin is set to input by using gpiod_direction_input(). The linux
     irq numbers are obtained by using gpiod_to_irq() and both
     interrutps are allocated by using devm_request_irq().
 */
static int
lothars_probe(struct platform_device* pdev)
{
	int count, ret, idx;
	unsigned int major;
	struct fwnode_handle *child;
	struct device *dev = &pdev->dev;
	struct keyled_priv *priv;

	dev_info(dev, "%s() - called\n", __func__);

	// get number of devices, should be one plus three pointers
	count = device_get_child_node_count(dev);
	if (!count) {
		return -ENODEV;
	}
	dev_info(dev, "%s() - there are %d nodes\n", __func__, count);

	// allocate all the private structures
	priv = devm_kzalloc(dev, sizeof_keyled_priv(count - INT_NUMBER), GFP_KERNEL);
	if (!priv) {
		dev_err(dev, "%s() - devm_kzalloc() failed!", __func__);
		return -ENOMEM;
	}
	dev_info(dev, "%s() - priv data structure allocated", __func__);

	// allocate 3 device numbers
	alloc_chrdev_region(&priv->led_devt, 0, count - INT_NUMBER, "keyled_class");
	major = MAJOR(priv->led_devt);
	dev_info(dev, "%s() - the major number is %d\n", __func__ , major);

	// create the led class
	priv->led_class = class_create(THIS_MODULE, "keyled");
	if (!priv->led_class) {
		dev_info(dev, "%s() - failed to allocate class\n", __func__);
		return -ENOMEM;
	}

	// set attributes for the led class
	priv->led_class->dev_groups = led_groups;
	priv->dev = dev;

	spin_lock_init(&priv->period_lock);

	// parse all the dt nodes
	device_for_each_child_node(dev, child) {  // loop macro!
		int irq, flags;
		struct gpio_desc *keyd;
		const char *label_name, *color_name, *trigger;
		struct led_device *new_led;

		fwnode_property_read_string(child, "label", &label_name);

		// parsing the dt led nodes
		if (0 == strcmp(label_name, "led")) {
			fwnode_property_read_string(child, "color", &color_name);

			/*
			  create led devices under keyled class
			  priv->num_leds is 0 for the first iteration
			  used to set the minor number of each device
			  increased to the end of the iteration
			 */
			new_led = led_device_register(color_name, priv->num_leds, dev,
						      priv->led_devt, priv->led_class);
			if (!new_led) {
				fwnode_handle_put(child);
				ret = PTR_ERR(new_led);
				for (idx = 0; idx < priv->num_leds - 1; idx++) {
					device_destroy(priv->led_class, MKDEV(MAJOR(priv->led_devt), idx));
				}
				class_destroy(priv->led_class);
				return ret;
			}

			// in v5.x
//			new_led->led_desc = devm_fwnode_get_gpiod_from_child(
//			        dev, NULL, child, GPIOD_ASIS, color_name);

			// in v6.3
			new_led->led_desc = devm_fwnode_gpiod_get(
				dev, child, NULL, GPIOD_ASIS, color_name);

			if (IS_ERR(new_led->led_desc)) {
				fwnode_handle_put(child);
				ret = PTR_ERR(new_led->led_desc);
				goto err;
			}
			new_led->private = priv;
			priv->leds[priv->num_leds] = new_led;
			priv->num_leds++;

			// set direction to output
			gpiod_direction_output(new_led->led_desc, 1);

			// set led state to off
			gpiod_set_value(new_led->led_desc, 1);

		// parsing the interrupt nodes
		} else if (0 == strcmp(label_name, "MIKROBUS_KEY_1")) {
			keyd = devm_fwnode_gpiod_get(dev, child, NULL, GPIOD_ASIS, label_name);
			gpiod_direction_input(keyd);
			fwnode_property_read_string(child, "trigger", &trigger);
			if (0 == strcmp(trigger, "falling")) {
				flags = IRQF_TRIGGER_FALLING;
			} else if (0 == strcmp(trigger, "rising")) {
				flags = IRQF_TRIGGER_RISING;
			} else if (0 == strcmp(trigger, "both")) {
				flags = IRQF_TRIGGER_FALLING || IRQF_TRIGGER_RISING;
			} else {
				return -EINVAL;
			}

			irq = gpiod_to_irq(keyd);
			if (0 > irq) {
				return irq;
			}

			ret = devm_request_irq(dev, irq, KEY_ISR1, flags, "ISR1", priv);
			if (ret) {
				dev_err(dev, "%s() - failed to request interrupt %d, error %d",
					__func__, irq, ret);
				return ret;
			}
			dev_info(dev, "%s() - irq number %d\n", __func__, irq);

		} else if (0 == strcmp(label_name, "MIKROBUS_KEY_2")) {
			keyd = devm_fwnode_gpiod_get(dev, child, NULL, GPIOD_ASIS, label_name);
			gpiod_direction_input(keyd);
			fwnode_property_read_string(child, "trigger", &trigger);
			if (0 == strcmp(trigger, "falling")) {
				flags = IRQF_TRIGGER_FALLING;
			} else if (0 == strcmp(trigger, "rising")) {
				flags = IRQF_TRIGGER_RISING;
			} else if (0 == strcmp(trigger, "both")) {
				flags = IRQF_TRIGGER_FALLING || IRQF_TRIGGER_RISING;
			} else {
				return -EINVAL;
			}

			irq = gpiod_to_irq(keyd);
			if (0 > irq) {
				return irq;
			}

			ret = devm_request_irq(dev, irq, KEY_ISR2, flags, "ISR2", priv);
			if (0 > ret) {
				dev_err(dev, "%s() - failed to request interrupt %d, error %d",
					__func__, irq, ret);
				goto err;
			}
			dev_info(dev, "%s() - irq number: %d\n", __func__, irq);
		} else {
			dev_info(dev, "%s() - bad device-tree value", __func__);
			ret = -EINVAL;
			goto err;
		}
	}
	dev_info(dev, "%s() - out of device-tree", __func__);

	// reset period to 10
	priv->period = 10;
	dev_info(dev, "%s() - the led period is: %d\n", __func__, priv->period);
	platform_set_drvdata(pdev, priv);

	dev_info(dev, "%s() - done", __func__);

	return 0;
err:
	// unregister everything in case of errors
	for (idx = 0; idx < priv->num_leds; idx++) {
		device_destroy(priv->led_class, MKDEV(MAJOR(priv->led_devt), idx));
	}
	class_destroy(priv->led_class);
	unregister_chrdev_region(priv->led_devt, priv->num_leds);

	return ret;
}

static int
lothars_remove(struct platform_device *pdev)
{
	int idx;
	struct led_device *led_count;
	struct keyled_priv *priv = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "%s() - called", __func__);
	if (1 == priv->task_flag) {
		kthread_stop(priv->task);
		priv->task_flag = 0;
	}

	if (1 == priv->led_flag) {
		for (idx = 0; idx < priv->num_leds; idx++) {
			led_count = priv->leds[idx];
			gpiod_set_value(led_count->led_desc, 1);
		}
	}

	for (idx = 0; idx < priv->num_leds; idx++) {
		device_destroy(priv->led_class, MKDEV(MAJOR(priv->led_devt), idx));
	}

	class_destroy(priv->led_class);
	unregister_chrdev_region(priv->led_devt, priv->num_leds);
	dev_info(&pdev->dev, "%s() - done", __func__);

	return 0;
}

/* The list of supported devices by this driver */
static const struct of_device_id lothars_of_ids[] = {
	{ .compatible = "lothars,ledpwm", },
	{ },
};
MODULE_DEVICE_TABLE(of, lothars_of_ids);

/* The platform_driver structure to be registered to the platform bus */
static struct platform_driver lothars_platform_driver = {
	.probe = lothars_probe,
	.remove = lothars_remove,
	.driver = {
		.name = "ledpwm",
		.of_match_table = lothars_of_ids,
	}
};

/* driver registration with generated simple init/exit() */
module_platform_driver(lothars_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("a keyled class demo");
