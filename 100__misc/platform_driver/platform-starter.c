// SPDX-License-Identifier: GPL-2.0+
/*
*/
#include <linux/platform_device.h>
#include <linux/module.h>

#define LOTHARS_PLATFORM_DRIVER "lothars-platform-dummy"
static struct platform_device *pdev;

static int __init pf_add(void)
{
	int inst_id = 0; // instance unique id eg. base address

	pr_info("%s(): called\n", __func__);

	pdev = platform_device_alloc(LOTHARS_PLATFORM_DRIVER, inst_id);
	platform_device_add(pdev);

	pr_info("%s(): platform driver added\n", __func__);
	return 0;
}

static void __exit pf_put(void)
{
	pr_info("%s(): called\n", __func__);
	platform_device_put(pdev);
}

module_init(pf_add);
module_exit(pf_put);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with platform drivers.");
