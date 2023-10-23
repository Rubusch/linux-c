/*
  I2C LTC3206 Demo for the DC749A Board

  Based on the referred driver, there are some adjustments to my setup,
  about address, ENRGB, etc.

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018

  VERIFIED:
  linux v6.3/aarch64
*/

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>

#define LED_NAME_LEN        32
#define CMD_RED_SHIFT       4
#define CMD_BLUE_SHIFT      4
#define CMD_GREEN_SHIFT     0
#define CMD_MAIN_SHIFT      4
#define CMD_SUB_SHIFT       0
#define EN_CS_SHIFT         (1 << 2)

/* set an led_device struct for each 5 led device */
struct led_device {
	u8 brightness;
	struct led_classdev cdev;  // include/linux/led.h
	struct led_priv *private;
};

/*
  store the global parameters shared for the 5 led devices
  the parameters are updated after each led_control() call
 */
struct led_priv {
	u32 num_leds;
	u8 command[3];
	struct gpio_desc *display_cs;
	struct i2c_client *client;
};

/* function that writes to the i2c device */
static int ltc3206_led_write(struct i2c_client *client, const u8 *command)
{
	int ret = i2c_master_send(client, command, 3);
	if (0 <= ret)
		return 0;
	return ret;
}

/*
  the sysfs functions

  NB: declare a static device "sub"
  static DEVICE_ATTR(sub, S_IWUSR, NULL, sub_select);
  -> set sub_select() as struct device::store() of the "sub" object
*/
static ssize_t sub_select(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	const char *buffer = buf;
	struct i2c_client *client;
	struct led_priv *private;

	client = to_i2c_client(dev);
	private = i2c_get_clientdata(client);

	private->command[0] |= EN_CS_SHIFT;   /* set the 3rd bit A2 */
	ltc3206_led_write(private->client, private->command);

	if (!strcmp(buffer, "on")) {
		gpiod_set_value(private->display_cs, 1);  /* low */
		usleep_range(100, 200);   /* a sleep 10us - 20ms => usleep_range() */
		gpiod_set_value(private->display_cs, 0);  /* high */
	} else if (!strcmp(buffer, "off")) {
		gpiod_set_value(private->display_cs, 0);  /* high */
		usleep_range(100, 200);
		gpiod_set_value(private->display_cs, 1);  /* low */
	} else {
		dev_err(&client->dev, "bad led value\n");
		return -EINVAL;
	}

	return count;

}
static DEVICE_ATTR(sub, S_IWUSR, NULL, sub_select);

/*
  NB: declare a static device "rgb"
  static DEVICE_ATTR(rgb, S_IWUSR, NULL, rgb_select);
  -> set rgb_select() as the store() of the "rgb" object
*/
static ssize_t rgb_select(struct device *dev, struct device_attribute *attr,
			  const char* buf, size_t count)
{
	const char *buffer = buf;
	struct i2c_client *client = to_i2c_client(dev);
	struct led_priv *private = i2c_get_clientdata(client);

	private->command[0] &= ~(EN_CS_SHIFT);   /* clear the 3rd bit */
	ltc3206_led_write(private->client, private->command);

	if (!strcmp(buffer, "on")) {
		gpiod_set_value(private->display_cs, 1);   /* low */
		usleep_range(100, 200);
		gpiod_set_value(private->display_cs, 0);   /* high */
	} else if (!strcmp(buffer, "off")) {
		gpiod_set_value(private->display_cs, 0);   /* high */
		usleep_range(100, 200);
		gpiod_set_value(private->display_cs, 1);   /* low */
	} else {
		dev_err(&client->dev, "bad led value\n");
		return -EINVAL;
	}

	return count;
}
static DEVICE_ATTR(rgb, S_IWUSR, NULL, rgb_select);

static struct attribute *display_cs_attrs[] = {
	&dev_attr_rgb.attr,
	&dev_attr_sub.attr,
	NULL,
};

static struct attribute_group display_cs_group = {
	.name = "display_cs",
	.attrs = display_cs_attrs,
};

/*
  called for each led device when writing the brightness file under
  each device the command parameters are kept in the led_priv struct
  that is pointed inside each led_device struct
*/
static int led_control(struct led_classdev *led_cdev,
		       enum led_brightness value)
{
	struct led_classdev *cdev;
	struct led_device *led;
	led = container_of(led_cdev, struct led_device, cdev);
	cdev = &led->cdev;
	led->brightness = value;

	dev_info(cdev->dev, "the subsystem is %s\n", cdev->name);

	if (value > 15 || value < 0)
		return -EINVAL;

	if (0 == strcmp(cdev->name, "red")) {
		led->private->command[0] &= 0x0F;   /* clear the upper nibble */
		led->private->command[0] |= ((led->brightness << CMD_RED_SHIFT) & 0xF0);
	} else if (0 == strcmp(cdev->name, "blue")) {
		led->private->command[1] &= 0x0F;   /* clear the upper nibble */
		led->private->command[1] |= ((led->brightness << CMD_BLUE_SHIFT) & 0xF0);
	} else if (0 == strcmp(cdev->name, "green")) {
		led->private->command[1] &= 0xF0;   /* clear the lower nibble */
		led->private->command[1] |= ((led->brightness << CMD_GREEN_SHIFT) & 0x0F);
	} else if (0 == strcmp(cdev->name, "main")) {
		led->private->command[2] &= 0x0F;   /* clear the upper nibble */
		led->private->command[2] |= ((led->brightness << CMD_MAIN_SHIFT) & 0xF0);
	} else if (0 == strcmp(cdev->name, "sub")) {
		led->private->command[2] &= 0xF0;   /* clear the lower nibble */
		led->private->command[2] |= ((led->brightness << CMD_SUB_SHIFT) & 0x0F);
	} else
		dev_info(cdev->dev, "no display found\n");

	return ltc3206_led_write(led->private->client, led->private->command);
}

static int __init ltc3206_probe(struct i2c_client *client)
{
	int count, ret;
	u8 value[3];
	struct fwnode_handle *child;
	struct device *dev = &client->dev;
	struct led_priv *private;

	dev_info(dev, "platform_probe enter\n");

	/*
	  set address
	*/

	// The LTC3206 responds to only one 7-bit address which
	// has been factory programmed to 0011011.
	// (LTC3206 datasheet)
	//
	// i.e. 0x36, since the last bit is 'r/w' which for the
	// LTC3206 will always be 'w' i.e. 0
	client->addr = 0x1B;  // 0|0|0|1|1|0|1|1
	// NB: works, r/w bit is for the rpi at least "in front"!

	/*
	  set blue led maximum value for i2c testing
	  ENRGB must be set to VCC to do the testing
	*/
	value[0] = 0x00;
	value[1] = 0xF0;
	value[2] = 0x00;
	i2c_master_send(client, value, 3);

	dev_info(dev, "led BLUE is ON\n");

	count = device_get_child_node_count(dev);
	if (!count)
		return -ENODEV;

	dev_info(dev, "there are %d nodes\n", count);

	private = devm_kzalloc(dev, sizeof(*private), GFP_KERNEL);
	if (!private)
		return -ENOMEM;

	// init i2c_client instance to object "private" [struct led_priv]
	private->client = client;

	// set object "private" as driver data to the client object
	i2c_set_clientdata(client, private);

	/*
	  The flags parameter is used to optionally specify a
	  direction and initial value for the GPIO. Values can be:

	  GPIOD_ASIS or 0 to not initialize the GPIO at all. The
	      direction must be set later with one of the dedicated
	      functions.

	  GPIOD_IN to initialize the GPIO as input.

	  GPIOD_OUT_LOW to initialize the GPIO as output with a value
	      of 0.

	  GPIOD_OUT_HIGH to initialize the GPIO as output with a value
 	      of 1.

	  GPIOD_OUT_LOW_OPEN_DRAIN same as GPIOD_OUT_LOW but also
	      enforce the line to be electrically used with open
	      drain.

	  GPIOD_OUT_HIGH_OPEN_DRAIN same as GPIOD_OUT_HIGH but also
  	      enforce the line to be electrically used with open drain
	 */

	// NB: (at least) the first insmod after boot up, will show
	// the enable line from LOW to HIGH, while follow up rmmod and
	// insmods simply will turn the LED off or light it up,
	// respectively
	private->display_cs = devm_gpiod_get(dev, NULL, GPIOD_OUT_HIGH);

	if (IS_ERR(private->display_cs)) {   // evaluate with IS_ERR() macro
		ret = PTR_ERR(private->display_cs);
		dev_err(dev, "unable to claim gpio\n");
		return ret;
	}
	gpiod_direction_output(private->display_cs, 1);

	/* register sysfs hooks */
	ret = sysfs_create_group(&client->dev.kobj, &display_cs_group);
	if (0 > ret) {
		dev_err(&client->dev, "could not register sysfs group\n");
		return ret;
	}

	/* parse all the child nodes */
	device_for_each_child_node(dev, child) {
		struct led_device *led_device;
		struct led_classdev *cdev;

		led_device = devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
		if (!led_device)
			return -ENOMEM;

		cdev = &led_device->cdev;
		led_device->private = private;

		// read property "label" from the given fwnode
		// "child", and store the value into val "cdev->name")
		fwnode_property_read_string(child, "label", &cdev->name);

		// evaluate the extracted cdev->name
		if ((0 == strcmp(cdev->name, "main"))
		    || (0 == strcmp(cdev->name, "sub"))
		    || (0 == strcmp(cdev->name, "red"))
		    || (0 == strcmp(cdev->name, "blue"))
		    || (0 == strcmp(cdev->name, "green")) ) {
			led_device->cdev.brightness_set_blocking = led_control;
			ret = devm_led_classdev_register(dev, &led_device->cdev);
			if (ret)
				goto err;
			dev_info(cdev->dev, "the subsystem is %s and num is %d\n",
				 cdev->name, private->num_leds);
		} else {
			dev_err(dev, "bad devicetree value\n");
			return -EINVAL;
		}
	}

	dev_info(dev, "ltc3206_probe() done\n");
	return 0;

err:
	fwnode_handle_put(child);
	sysfs_remove_group(&client->dev.kobj, &display_cs_group);
	return ret;
}

static void ltc3206_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "ltc3206_remove enter\n");
	sysfs_remove_group(&client->dev.kobj, &display_cs_group);
	dev_info(&client->dev, "ltc3206_remove() exit\n");
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "lothars,ltc3206"},
	{},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static const struct i2c_device_id ltc3206_id[] = {
	{ "ltc3206", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, ltc3206_id);

static struct i2c_driver ltc3206_driver = {
	.probe = ltc3206_probe,
	.remove = ltc3206_remove,
	.id_table = ltc3206_id,
	.driver = {
		.name = "ltc3206",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_i2c_driver(ltc3206_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("LTC3206 multidisplay driver via I2C master connection");
