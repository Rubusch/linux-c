/*
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_sysfs(void);
void cleanup_hello_sysfs(void);

static ssize_t sysfs_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t sysfs_store(struct kobject *, struct kobj_attribute *,
			   const char *, size_t);

/*
  globals
*/

#define HELLO_SYSFS_NAME "lothars_sysfs"

volatile int hello_sysfs_value = 0;

// sysfs kobject and attribute
struct kobject *kobj_ref;
struct kobj_attribute hello_sysfs_attr =
	__ATTR(hello_sysfs_value, 0600, sysfs_show, sysfs_store);

/*
  implementation
*/

/*
  Called when the sysfs file is read.
*/
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr,
			  char *buf)
{
	ssize_t ret = sprintf(buf, "%d\n", hello_sysfs_value);
	printk(KERN_INFO "%s(%p, %p, '%s') - Read!\n", __func__, kobj, attr,
	       buf);
	return ret;
}

/*
  Called when the sysfs file is written.
*/
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t count)
{
	printk(KERN_INFO "%s(%p, %p, '%s', %lu) - Write!\n", __func__, kobj,
	       attr, buf, count);
	sscanf(buf, "%d", &hello_sysfs_value);
	return count;

	/*
	// NB: Don't confuse it with the following, which results in an endless loop!!!
	count = sscanf(buf, "%d", &hello_sysfs_value);
	return count;
	*/
}

/*
  start / stop module
*/

int init_hello_sysfs(void)
{
	printk(KERN_INFO "%s() initializing...\n", __func__);

	/* 1. Create sysfs directory */

	/**
	 * kobject_create_and_add() - Create a struct kobject dynamically and
	 *                            register it with sysfs.
	 * @name: the name for the kobject
	 * @parent: the parent kobject of this kobject, if any.
	 *
	 * This function creates a kobject structure dynamically and registers it
	 * with sysfs.  When you are finished with this structure, call
	 * kobject_put() and the structure will be dynamically freed when
	 * it is no longer being used.
	 *
	 * If the kobject was not able to be created, NULL will be returned.
	 */
	kobj_ref = kobject_create_and_add(HELLO_SYSFS_NAME, kernel_kobj);

	/* 2. Create sysfs file */
	if (sysfs_create_file(kobj_ref,
			      (struct attribute *)&hello_sysfs_attr)) {
		printk(KERN_ERR "creating sysfs file failed.\n");
		return -ENOMEM;
	}

	return 0;
}

void cleanup_hello_sysfs(void)
{
	/**
	 * kobject_put() - Decrement refcount for object.
	 * @kobj: object.
	 *
	 * Decrement the refcount, and if 0, call kobject_cleanup().
	 */
	kobject_put(kobj_ref);
	sysfs_remove_file(kernel_kobj, &hello_sysfs_attr.attr);

	printk(KERN_INFO "%s() READY.\n", __func__);
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_hello_sysfs();
}

static void __exit mod_exit(void)
{
	cleanup_hello_sysfs();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates sysfs usage.");
