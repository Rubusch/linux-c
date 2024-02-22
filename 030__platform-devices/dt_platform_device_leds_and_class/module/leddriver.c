/*
   LED with Devicetree Binding

   Demonstrated LED class (device class) demo.
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/leds.h>
//#include <linux/miscdevice.h> /* now instead of the miscdevice use the new led class device */

// TODO check missing userspace                    
#define DEVICE_NAME "lothars_device"

/* statics */
struct led_dev
{
	u32 led_mask;   // different mask for R, G and B
	void __iomem *base;
	struct led_classdev cdev; // the new 'led_classdev'
};

#define BCM2710_PERI_BASE    0x3F000000
#define GPIO_BASE            (BCM2710_PERI_BASE + 0x200000)   /* GPIO controller */

#define GPIO_27              27
#define GPIO_22              22
#define GPIO_26              26

/* offsets */
#define GPFSEL2_offset       0x08
#define GPSET0_offset        0x1c
#define GPCLR0_offset        0x28

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
static void __iomem *GPFSEL2_V;
static void __iomem *GPSET0_V;
static void __iomem *GPCLR0_V;


static void led_control(struct led_classdev *led_cdev, enum led_brightness b)
{
	struct led_dev *led = container_of(led_cdev, struct led_dev, cdev);

	iowrite32(GPIO_MASK_ALL_LEDS, led->base + GPCLR0_offset);

	if (LED_OFF != b) {   // led ON
		iowrite32(led->led_mask, led->base + GPSET0_offset);
	} else {
		iowrite32(led->led_mask, led->base + GPCLR0_offset);  // led OFF
	}
}

static int __init ledclass_probe(struct platform_device* pdev)
{
	void __iomem *g_ioremap_addr;
	struct device_node *child;
	struct resource *res;
	u32 GPFSEL_read, GPFSEL_write;
	struct device *dev = &pdev->dev;
	int count, ret=0;

	pr_info("%s(): started\n", __func__);

	// get your first memory resource from device tree
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}

	dev_info(dev, "res->start = 0x%08lx\n", (long unsigned int) res->start);
	dev_info(dev, "res->end = 0x%08lx\n", (long unsigned int) res->end);

	// ioremap your memory region
	g_ioremap_addr = devm_ioremap(dev, res->start, resource_size(res));
	if (!g_ioremap_addr) {
		dev_err(dev, "ioremap failed\n");
		return -ENOMEM;
	}

	// from device tree
	count = of_get_child_count(dev->of_node);
	if (!count)
		return -EINVAL;
	dev_info(dev, "there are %d nodes\n", count);

	// enable all leds and set dir to output
	// (particular approach to RPI3b)
	GPFSEL_read = ioread32(g_ioremap_addr + GPFSEL2_offset); // read actual value
	GPFSEL_write = (GPFSEL_read & ~GPIO_MASK_ALL_LEDS) | (GPIO_SET_FUNCTION_LEDS & GPIO_MASK_ALL_LEDS);
	iowrite32(GPFSEL_write, g_ioremap_addr + GPFSEL2_offset);   // set dir leds to output
	iowrite32(GPIO_SET_ALL_LEDS, g_ioremap_addr + GPCLR0_offset);  // clear all the leds, output is low

	for_each_child_of_node(dev->of_node, child) {
		struct led_dev *led_device;
		struct led_classdev *cdev;
		led_device = devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
		if (!led_device)
			return -ENOMEM;

		cdev = &led_device->cdev;

		led_device->base = g_ioremap_addr;

		of_property_read_string(child, "label", &cdev->name);
		if (0 == strcmp(cdev->name, "red")) {
			led_device->led_mask = GPIO_27_INDEX;
			led_device->cdev.default_trigger = "heartbeat"; // NB: first item has the "trigger" set to heardbeat
		} else if (0 == strcmp(cdev->name, "green")) {
			led_device->led_mask = GPIO_22_INDEX;
		} else if (0 == strcmp(cdev->name, "blue")) {
			led_device->led_mask = GPIO_26_INDEX;
		} else {
			dev_info(dev, "bad device tree value\n");
			return -EINVAL;
		}

		// disable timer trigger until led is on
		led_device->cdev.brightness = LED_OFF;
		led_device->cdev.brightness_set = led_control;  // register led_control() for brightness control here

		ret = devm_led_classdev_register(dev, &led_device->cdev);
		if (ret) {
			dev_err(dev, "failed to register the led %s\n", cdev->name);
			of_node_put(child);
			return ret;
		}
	}

	dev_info(dev, "%s(): done\n", __func__);
	return 0;
}

static int __exit ledclass_remove(struct platform_device* pdev)
{
	// NB: use dev_info() for device context printouts
	dev_info(&pdev->dev, "ledclass_remove() started\n");

// TODO is implementation missing?
//	device_destroy(dev_class, dev);
//	class_destroy(dev_class);
//	cdev_del(&hello_chardev_cdev);
//	unregister_chrdev_region(dev, 1);

	dev_info(&pdev->dev, "ledclass_remove() done\n");
	return 0;
}

/* registration for devicetree binding */
static const struct of_device_id ledclass_of_ids[] = {
	{ .compatible = "lothars,RGBclassleds" },
	{},
};
MODULE_DEVICE_TABLE(of, ledclass_of_ids);

static struct platform_driver led_platform_driver = {
	.probe = ledclass_probe,
	.remove = ledclass_remove,
	.driver = {
		.name = "RGBclassleds",
		.of_match_table = ledclass_of_ids,
	}
};

/*
  init / exit
*/
static int led_init(void)
{
	int ret;

	pr_info("%s(): started\n", __func__);

	ret = platform_driver_register(&led_platform_driver);
	if (ret != 0) {
		pr_err("%s(): platform value returned %d\n", __func__, ret);
		return ret;
	}

	GPFSEL2_V = ioremap(GPFSEL2, sizeof(u32));
	GPSET0_V = ioremap(GPSET0, sizeof(u32));
	GPCLR0_V = ioremap(GPCLR0, sizeof(u32));

	pr_info("%s(): done\n", __func__);
	return 0;
}

static void led_exit(void)
{
	pr_info("%s(): started\n", __func__);

	platform_driver_unregister(&led_platform_driver);

	pr_info("%s(): done\n", __func__);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with ledclass device");
