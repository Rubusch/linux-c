// SPDX-License-Identifier: GPL-2.0+
/*
   Char Device with Devicetree Binding

   Demonstrates how to connect the chardev driver to the
   devicetree. Three GPIOs are connected through expansion header to
   colored leds. The binding is set up in the DT.

   - Sets ledcolors in the device tree

   - Declare __iomem pointers that will hold the virtual addresses
     returned by the dev_ioremap() function
*/

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

// define major number
#define DEVICE_NAME "lothars_device"

/* statics */
struct led_dev
{
	struct miscdevice led_misc_device;
	u32 led_mask;   // different mask for R, G and B
	const char* led_name;
	char led_value[8];
};

#define BCM2710_PERI_BASE    0x3F000000
#define GPIO_BASE            (BCM2710_PERI_BASE + 0x200000)   /* GPIO controller */

#define GPIO_27              27
#define GPIO_22              22
#define GPIO_26              26

/* set and clear each individual led */
#define GPIO_27_INDEX        1 << (GPIO_27 % 32)
#define GPIO_22_INDEX        1 << (GPIO_22 % 32)
#define GPIO_26_INDEX        1 << (GPIO_26 % 32)

/* select the output function */
#define GPIO_27_FUNC         1 << ((GPIO_27 % 10) * 3)
#define GPIO_22_FUNC         1 << ((GPIO_22 % 10) * 3)
#define GPIO_26_FUNC         1 << ((GPIO_26 % 10) * 3)

/* mask the GPIO functions */
#define FSEL_27_MASK         0b111 << ((GPIO_27 % 10) * 3) /* red since bit 21 (FSEL27) */
#define FSEL_22_MASK         0b111 << ((GPIO_22 % 10) * 3) /* green since bit 6 (FSEL22) */
#define FSEL_26_MASK         0b111 << ((GPIO_26 % 10) * 3) /* blue since bit 18 (FSEL26) */

#define GPIO_SET_FUNCTION_LEDS (GPIO_27_FUNC | GPIO_22_FUNC | GPIO_26_FUNC)
#define GPIO_MASK_ALL_LEDS (FSEL_27_MASK | FSEL_22_MASK | FSEL_26_MASK)
#define GPIO_SET_ALL_LEDS (GPIO_27_INDEX | GPIO_22_INDEX | GPIO_26_INDEX)

#define GPFSEL2              GPIO_BASE + 0x08
#define GPSET0               GPIO_BASE + 0x1c
#define GPCLR0               GPIO_BASE + 0x28

/* declare __iomem pointers */
// NB: __iomem is a smart cookie for the tool "sparse" to avoid mixing
// device pointers with normal pointers
static void __iomem *GPFSEL2_V;
static void __iomem *GPSET0_V;
static void __iomem *GPCLR0_V;

static ssize_t led_write(struct file *file, const char __user* buff, size_t count, loff_t* ppos)
{
	const char* led_on = "on";
	const char* led_off = "off";
	struct led_dev *led_device;

	pr_info("%s(): is called\n", __func__);

	led_device = container_of(file->private_data, struct led_dev, led_misc_device);

	/*
	  terminal echo add '\n' character
	  led_device->led_value = "on\n" or "off\n", after copy_from_user()
	*/
	if (copy_from_user(led_device->led_value, buff, count)) {
		pr_warn("%s(): bad copied value\n", __func__);
		return -EFAULT;
	}

	/*
	  we add \0 replacing \n in led_device->led_value in probe()
	  initialization
	*/

	led_device->led_value[count - 1] = '\0';

	pr_info("%s(): received from userspace %s\n",
		__func__, led_device->led_value);

	if (!strcmp(led_device->led_value, led_on)) {
		iowrite32(led_device->led_mask, GPSET0_V);
	} else if (!strcmp(led_device->led_value, led_off)) {
		iowrite32(led_device->led_mask, GPCLR0_V);
	} else {
		pr_warn("%s(): bad value\n", __func__);
		return -EINVAL;
	}

	pr_info("%s(): done\n", __func__);
	return count;
}

static ssize_t led_read(struct file *file, char __user* buff,
			size_t count, loff_t* ppos)
{
	struct led_dev *led_device;

	pr_info("%s(): is called\n", __func__);

	led_device = container_of(file->private_data, struct led_dev, led_misc_device);

	if (0 == *ppos) {
		if (copy_to_user(buff, &led_device->led_value,
				 sizeof(led_device->led_value))) {
			pr_warn("%s(): failed to return led_value to userspace\n",
				__func__);
			return -EFAULT;
		}
		*ppos += 1;
		return sizeof(led_device->led_value);
	}
	pr_info("%s(): done\n", __func__);
	return 0;
}

// fops - declare a file operations structure
static const struct file_operations led_fops = {
	.read = led_read,
	.write = led_write,
};

/* of - platform driver stuff, here with init/exit setup */

static int led_probe(struct platform_device* pdev)
{
	struct led_dev *led_device;
	int ret;
	char led_val[8] = "off\n";

	pr_info("%s(): called\n", __func__);

	led_device = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);
	if (!led_device) {
		pr_warn("%s(): failed\n", __func__);
		return -ENOMEM;
	}

	of_property_read_string(pdev->dev.of_node, "label", &led_device->led_name);
	led_device->led_misc_device.minor = MISC_DYNAMIC_MINOR;
	led_device->led_misc_device.name = DEVICE_NAME;
	led_device->led_misc_device.fops = &led_fops;

	if (0 == strcmp(led_device->led_name, "ledred")) {
		led_device->led_mask = GPIO_27_INDEX;
	} else if (0 == strcmp(led_device->led_name, "ledgreen")) {
		led_device->led_mask = GPIO_22_INDEX;
	} else if (0 == strcmp(led_device->led_name, "ledblue")) {
		led_device->led_mask = GPIO_26_INDEX;
	} else {
		pr_warn("%s(): bad devicetree value\n", __func__);
		return -EINVAL;
	}

	/* initialize the led status to OFF */
	memcpy(led_device->led_value, led_val, sizeof(led_val));

	ret = misc_register(&led_device->led_misc_device);
	if (ret) {
		return ret;
	}
	platform_set_drvdata(pdev, led_device);

	pr_info("%s(): done\n", __func__);
	return 0;
}

static int led_remove(struct platform_device *pdev)
{
	struct led_dev *led_device = platform_get_drvdata(pdev);

	pr_info("led_remove() started\n");
	misc_deregister(&led_device->led_misc_device);
	pr_info("led_remove() done\n");
	return 0;
}

// platform device registration
static const struct of_device_id led_of_ids[] = {
	{ .compatible = "lothars,RGBleds" },
	{},
};
MODULE_DEVICE_TABLE(of, led_of_ids);

static struct platform_driver led_platform_driver = {
	.probe = led_probe,
	.remove = led_remove,
	.driver = {
		.name = "RGBleds",
		.of_match_table = led_of_ids,
	},
};

/*
  here the leds are connected to read/write
*/
static int __init led_init(void)
{
	int ret;
	u32 GPFSEL_read, GPFSEL_write;

	pr_info("led_init started\n");
	ret = platform_driver_register(&led_platform_driver);
	if (ret != 0) {
		pr_err("platform value returned %d\n", ret);
		return ret;
	}

	// NB: don't use ioremap() -> prefer the managed devm_ioremap()
	GPFSEL2_V = ioremap(GPFSEL2, sizeof(u32));
	GPSET0_V = ioremap(GPSET0, sizeof(u32));
	GPCLR0_V = ioremap(GPCLR0, sizeof(u32));

	// read
	GPFSEL_read = ioread32(GPFSEL2_V);   /* read current value */

	/*
	  set to 0 - 3 bits of each FSEL and keep equal the rest of bits,
	  then set to 1 the first bit of each FSEL, to set 3 GPIOs to output
	*/

	// write
	GPFSEL_write = (GPFSEL_read & ~GPIO_MASK_ALL_LEDS) |
		(GPIO_SET_FUNCTION_LEDS & GPIO_MASK_ALL_LEDS);

	iowrite32(GPFSEL_write, GPFSEL2_V); // set leds to output
	iowrite32(GPIO_SET_ALL_LEDS, GPCLR0_V); // clear all the leds,
						// output is low

	pr_info("led_init() done\n");
	return 0;
}

static void __exit led_exit(void)
{
	pr_info("led_exit() started\n");
	iowrite32(GPIO_SET_ALL_LEDS, GPCLR0_V);   /* clear all the leds */

	iounmap(GPFSEL2_V);
	iounmap(GPSET0_V);
	iounmap(GPCLR0_V);

	platform_driver_unregister(&led_platform_driver);

	pr_info("led_exit() done\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with leds");
