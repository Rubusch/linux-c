/*
  IRQ click device

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018

  VERIFIED:
  linux v6.3/aarch64
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
#include <linux/of.h>

static char *LOTHARS_KEY_NAME = "LOTHARS_KEY";

/* interrupt handler */
static irqreturn_t lothars_isr(int irq, void* data)
{
	struct device *dev = data;
	dev_info(dev, "lothars_isr() - interrupt received, key: '%s'\n",
		 LOTHARS_KEY_NAME);
	return IRQ_HANDLED;
}

static struct miscdevice lothars_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lothar_dev",
};

static int lothars_probe(struct platform_device *pdev)
{
	int ret, irq;
	struct gpio_desc *gpio;
	struct device *dev = &pdev->dev;

	dev_info(&pdev->dev, "lothars_probe() - called\n");

	/* first method to get the virtual linux irq number */
	gpio = devm_gpiod_get(dev, NULL, GPIOD_IN);
	if (IS_ERR(gpio)) {
		dev_err(dev, "lothars_probe() - gpio_get() failed\n");
		return PTR_ERR(gpio);
	}

	irq = gpiod_to_irq(gpio);
	if (irq < 0)
		return irq;
	dev_info(dev, "lothars_probe() - irq number is '%d' by gpiod_to_irq()\n",
		 irq);

	/* second method to get the firtual linux irq number */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "lothars_probe() - irq is not available\n");
		return -EINVAL;
	}
	dev_info(dev, "lothars_probe() - irq number is '%d' by platform_get_irq()\n",
		 irq);

	/* allocate the interrupt line */
	ret = devm_request_irq(dev, irq, lothars_isr,
			       IRQF_TRIGGER_FALLING,
			       LOTHARS_KEY_NAME, dev);
	if (ret) {
		dev_err(dev, "lothars_probe() - failed to request interrupt %d, error %d\n",
			irq, ret);
		return ret;
	}

	ret = misc_register(&lothars_miscdevice);
	if (ret != 0) {
		dev_err(dev, "lothars_probe() - could not register the misc device lothars_miscdevice\n");
		return ret;
	}

	dev_info(dev, "lothars_probe() - got minor %i\n", lothars_miscdevice.minor);
	dev_info(dev, "lothars_probe() - done\n");

	return 0;
}

static int lothars_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "lothars_remove() - called\n");
	misc_deregister(&lothars_miscdevice);

	return 0;
}

static const struct of_device_id lothars_of_ids[] = {
	{ .compatible = "lothars,intkey", }, // device-tree entry
	{ },
};
MODULE_DEVICE_TABLE(of, lothars_of_ids);

static struct platform_driver lothars_platform_driver = {
	.probe = lothars_probe,
	.remove = lothars_remove,
	.driver = {
		.name = "intkey",
		.of_match_table = lothars_of_ids,
		.owner = THIS_MODULE,
	}
};
module_platform_driver(lothars_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("external click device for issuing interrupts");
