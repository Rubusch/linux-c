// SPDX-License-Identifier: GPL-2.0+
/*
  demonstrates kmalloc
*/
#include <linux/module.h>
#include <linux/init.h>

struct driver_data {
	char data[64];
	u8 val;
};

static u32 *ptr1;
struct driver_data *ptr2;

static int __init mod_init(void) {
	pr_info("%s(): called", __func__);

	// ptr1 - kmalloc()
	ptr1 = kmalloc(sizeof(*ptr1), GFP_KERNEL);
	if (NULL == ptr1) {
		pr_err("%s(): failed to kmalloc() ptr1", __func__);
		return -ENOMEM;
	}
	pr_info("%s(): *ptr1: 0x%08x - kmalloc()", __func__, *ptr1);

	*ptr1 = 0xc00abcde;
	pr_info("%s(): *ptr1: 0x%08x", __func__, *ptr1);
	kfree(ptr1);

	// ptr1 - kzalloc()
	ptr1 = kzalloc(sizeof(*ptr1), GFP_KERNEL);
	if (NULL == ptr1) {
		pr_err("%s(): failed to kzalloc() the ptr1", __func__);
		return -ENOMEM;
	}
	pr_info("%s(): *ptr1: 0x%08x - kzalloc()", __func__, *ptr1);

	*ptr1 = 0xc0012345;
	pr_info("%s(): *ptr1: 0x%08x", __func__, *ptr1);
	kfree(ptr1);

	// ptr2 - kzalloc
	ptr2 = kzalloc(sizeof(*ptr2), GFP_KERNEL);
	if (NULL == ptr2) {
		pr_err("%s(): failed to kzalloc() ptr2", __func__);
		return -ENOMEM;
	}
	ptr2->val = 123;
	strcpy(ptr2->data, "Humpty Dumpty sat on a wall.");

	pr_info("%s(): ptr2->val %d, ptr2->data '%s' (global data)",
		__func__, ptr2->val, ptr2->data);

	return 0;
}

static void __exit mod_exit(void) {
	pr_info("%s(): called", __func__);

	pr_info("%s(): ptr2->val %d, ptr2->data '%s' (global data)",
		__func__, ptr2->val, ptr2->data);

	kfree(ptr2);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with kmalloc()");
