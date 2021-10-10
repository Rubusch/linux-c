/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_devnum(void);
void cleanup_devnum(void);

/*
  globals
*/

#define LOTHARS_DEVICE_NAME "lothars_device"
#define DEVICE_MAJOR_NUMBER 123
#define DEVICE_MINOR_NUMBER 123

// Statically assigning a device number
// (better use dynamic assignment of device numbers)
dev_t dev = MKDEV(DEVICE_MAJOR_NUMBER, DEVICE_MINOR_NUMBER);

/*
  implementation
*/

///

/*
  start / stop module
*/

int init_devnum(void)
{
	printk(KERN_INFO "%s() initializing...!\n", __func__);
	/**
	 * register_chrdev_region() - register a range of device numbers
	 * @from: the first in the desired range of device numbers; must include
	 *        the major number.
	 * @count: the number of consecutive device numbers required
	 * @name: the name of the device or driver.
	 *
	 * Return value is zero on success, a negative error code on failure.
	 */
	register_chrdev_region(dev, 1, LOTHARS_DEVICE_NAME);
	printk(KERN_INFO "%s() major = %d, minor = %d\n", __func__, MAJOR(dev),
	       MINOR(dev));
	return 0;
}

void cleanup_devnum(void)
{
	/**
	 * unregister_chrdev_region() - unregister a range of device numbers
	 * @from: the first in the range of numbers to unregister
	 * @count: the number of device numbers to unregister
	 *
	 * This function will unregister a range of @count device numbers,
	 * starting with @from.  The caller should normally be the one who
	 * allocated those numbers in the first place...
	 */
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s() READY.\n", __func__);
}

/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_devnum();
}

static void __exit mod_exit(void)
{
	cleanup_devnum();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates major and minor device numbers!");
