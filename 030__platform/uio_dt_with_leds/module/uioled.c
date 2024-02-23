/*
   LED with UIO / devicetree binding

   Demonstrates UIO usage
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/uio_driver.h>


#define DEVICE_NAME "lothars_device"

static struct uio_info the_uio_info;

static int mod_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct device *dev = &pdev->dev;
	void __iomem *g_ioremap_addr;

	dev_info(dev, "mod_probe() called\n");

	// get the first memory resource from devicetree
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "IORESOURCE_MEM, 0 does not exist!\n");
		return -EINVAL;
	}

	dev_info(dev, "res->start = 0x%08lx\n", (long unsigned int) res->start);
	dev_info(dev, "res->end = 0x%08lx\n", (long unsigned int) res->end);

	// ioremap our memory region and get virtual address
	g_ioremap_addr = devm_ioremap(dev, res->start, resource_size(res));
	if (!g_ioremap_addr) {
		dev_err(dev, "ioremap() failed\n");
		return -ENOMEM;
	}

	// initialize uio_info's struct uio_mem array
	the_uio_info.name = DEVICE_NAME; // required
	the_uio_info.version = "1.0";    // required
	the_uio_info.mem[0].name = "demo_uio_driver_hw_region";
	the_uio_info.mem[0].memtype = UIO_MEM_PHYS;
	the_uio_info.mem[0].addr = res->start;              // physical address needed for the kernel user mapping
	the_uio_info.mem[0].size = resource_size(res);
	the_uio_info.mem[0].internal_addr = g_ioremap_addr; // virtual address for internal driver use

	// register the uio device
	ret = uio_register_device(&pdev->dev, &the_uio_info);
	if (ret != 0) {
		dev_info(dev, "could not register device led_uio\n");
	}

	return ret;
}

static int mod_remove(struct platform_device *pdev)
{
	uio_unregister_device(&the_uio_info);
	dev_info(&pdev->dev, "mod_remove() called\n");

	return 0;
}

static const struct of_device_id mod_of_ids[] = {
	{ .compatible = "lothars,uio" },
	{},
};

MODULE_DEVICE_TABLE(of, mod_of_ids);

static struct platform_driver mod_platform_driver = {
	.probe = mod_probe,
	.remove = mod_remove,
	.driver = {
		.name = "uio",
		.of_match_table = mod_of_ids,
		.owner = THIS_MODULE,
	}
};
module_platform_driver(mod_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("messing with uio and devicetree");
