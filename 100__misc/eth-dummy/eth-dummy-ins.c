// SPDX-License-Identifier: GPL-2.0+
/*
  Load this inserter module after the particular module to be probed.
 */
#include <linux/platform_device.h>
#include <linux/module.h>

// must match platform_driver.driver.name
#define PROBED_MODULE_NAME "lothars-eth-dummy"
static struct platform_device *pdev;

static int __init mod_add(void)
{
	int inst_id = 1; // unique id, e.g. base address
	pdev = platform_device_alloc(PROBED_MODULE_NAME, inst_id);
	platform_device_add(pdev);
	return 0;
}

static void __exit mod_put(void)
{
	platform_device_put(pdev);
}

module_init(mod_add);
module_exit(mod_put);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("A starter instead of a DT binding.");
