// SPDX-License-Identifier: GPL-2.0+
/*
  demonstrates vmalloc, allocation of virtual memory (needed remapping
  first on 32-bit systems)
*/
#include<linux/init.h>
#include<linux/module.h>
#include <linux/vmalloc.h>

static void *ptr;

static int __init mod_init(void)
{
	int val = 0xabcd;
	unsigned long size = 8192;

	pr_info("%s(): called\n", __func__);
	ptr = vmalloc(size);
	if (!ptr) {
		pr_err("%s(): vmalloc failed\n", __func__);
		return -ENOMEM;
	}
	pr_info("%s(): vmalloc successfully\n", __func__);

	ptr = ((char*) &val);
	pr_info("%s(): *ptr1: 0x%08x", __func__, *((int*) ptr));

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	vfree(ptr);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with vmalloc()");
