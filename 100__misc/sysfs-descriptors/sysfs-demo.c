// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kobject.h>

#include "sysfs_common.h"

#define SYSFS_NODE_NAME  "lothars-sysfs"

struct d_attr {
	struct attribute attr;
	int value; // this is our data
};

// instance: notify
static struct d_attr notify = {
	.attr.name="notify",
	.attr.mode = 0644,
	.value = 0,
};

// instance: trigger
static struct d_attr trigger = {
	.attr.name="trigger",
	.attr.mode = 0644,
	.value = 0,
};

/*
  Array of attr instances

  The ATTRIBUTE_GROUPS() macro reduces the extension "_attrs[]", and
  provides another extension "_groups" to be used when initializing the
  struct kobj_type, later.
 */
static struct attribute * d_attrs[] = {
	&notify.attr,
	&trigger.attr,
	NULL
};
ATTRIBUTE_GROUPS(d); // NB: reduces the extension "_attrs[]" from above, 

static ssize_t
sysfs_show(struct kobject *kobj, struct attribute *attr, char *buf) // TODO page  
{
	struct d_attr *da = container_of(attr, struct d_attr, attr);

	pr_info( "%s(): called - (%s)\n", __func__, da->attr.name );
	return scnprintf(buf, PAGE_SIZE, "%s: %d\n", da->attr.name, da->value);
}

static struct kobject *kobj_ref;

static ssize_t
sysfs_store(struct kobject *kobj, struct attribute *attr,
	    const char *buf, size_t len)
{
	struct d_attr *da = container_of(attr, struct d_attr, attr);

	pr_info("%s(): called\n", __func__);

	sscanf(buf, "%d", &da->value);
	pr_info("%s(): sysfs_notify store %s = %d\n",
		__func__, da->attr.name, da->value);

	if (0 == strcmp(da->attr.name, "notify")) {
		notify.value = da->value;
		sysfs_notify(kobj_ref, NULL, "notify");
	} else if (0 == strcmp(da->attr.name, "trigger")) {
		trigger.value = da->value;
		sysfs_notify(kobj_ref, NULL, "trigger");
	}
	return sizeof(int);
}

static struct sysfs_ops s_ops = {
	.show = sysfs_show,
	.store = sysfs_store,
};

static struct kobj_type k_type = {
	.sysfs_ops = &s_ops,
	.default_groups = d_groups, // by macro created "d_groups"
};

//static struct kobject *kobj_ref; // TODO rm - duplicate?     

static int __init mod_init(void)
{
	int err = -1;

	pr_info("%s(): called\n", __func__);

	// NB: kobj_ref = kobject_create() is not exported
	kobj_ref = kzalloc(sizeof(*kobj_ref), GFP_KERNEL);
	if (kobj_ref) {
		kobject_init(kobj_ref, &k_type);
		if (kobject_add(kobj_ref, NULL, "%s", SYSFS_NODE_NAME)) {
			err = -1;
			pr_info("%s(): kobject_add() failed\n", __func__);
			kobject_put(kobj_ref);
			kobj_ref = NULL;
		}
		err = 0;
	}
	return err;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	if (kobj_ref) {
		kobject_put(kobj_ref);
		kfree(kobj_ref);
	}
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with sysfs poll.");
