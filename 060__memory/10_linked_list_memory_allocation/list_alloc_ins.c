// SPDX-License-Identifier: GPL-2.0+
/*
  This approach avoids implementing a DT binding to trigger the
  probe() of a platform driver module.
*/
#include <linux/platform_device.h>
#include <linux/module.h>

#define PROBE_DRIVER_NAME "linked_memory"
static struct platform_device *pdev;

static int __init pf_add(void)
{
	int inst_id = 0; // instance unique id eg. base address
	pdev = platform_device_alloc(PROBE_DRIVER_NAME, inst_id);
	platform_device_add(pdev);
	return 0;
}

static void __exit pf_put(void)
{
	platform_device_put(pdev);
}

module_init(pf_add);
module_exit(pf_put);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Omitting a DT binding.");
