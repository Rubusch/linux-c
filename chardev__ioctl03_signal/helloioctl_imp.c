/*

  - A userspace application connects to this module (chardev driver)
    via device_ioctl() and writes the signal number which the
    application is capable to serve.

  - A separate trigger via 'cat /dev/lothars_device'
    i.e. device_read() will then issue the passed signal number
    (SIG_LOTHAR) to the userspace application.

  - The userspace application has a signal handler which captures the
    signal and performs the action (quits itself).

  - The device_release() will cleanup the signal handling together
    with the chardev itself.

  NB: Generally, this is usefull in an approach to register an
  userspace application via ioctl, but then e.g. in an interrupt
  handler issue the signal (as kind of an interrupt) to notify the
  userspace application.

  The problem in this for a demo is how to simulate hardware
  interrupts without the need to rebuild the entire kernel and export
  internal vector_irq lists or such..

  Hence, this awkward solution here to issue a Signal on a read
  chardev.
*/

#include "helloioctl.h"



/*
  forwards
*/

static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char __user*, size_t, loff_t*);
static long device_ioctl(struct file*, unsigned int, unsigned long);



/*
  globals
*/

// signal
static struct task_struct *task;
static int signum = 0;

// chardev
dev_t dev = 0;
static struct class *dev_class;
static struct cdev hello_chardev;

// fops
static struct file_operations hello_chardev_fops = {
	.owner = THIS_MODULE,
	.release = device_release,
	.read = device_read,
	.unlocked_ioctl = device_ioctl,
};



/*
  implementation
*/


/*
  Called when the device file is closed.

  Returns 0 if success.
*/
static int device_release(struct inode* inode, struct file* file)
{
	struct task_struct *ref_task = get_current();
	printk(KERN_INFO "CHARDEV: %s()\n", __func__);

	// delete the task
	if (ref_task == task) {
		task = NULL;
	}
	return SUCCESS;
}


/*
  Called whenever a process which has already opened the device file,
  attempts to read form it.

  Returns the number of bytes read.
*/
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset)
{
	struct kernel_siginfo info;

	printk(KERN_INFO "CHARDEV: %s()\n", __func__);

	printk(KERN_INFO "CHARDEV: %s() sending SIGNAL to userspace application\n", __func__);
	memset(&info, 0, sizeof(info));
	info.si_signo = SIG_LOTHAR;
	info.si_code = SI_QUEUE;
	info.si_int = 1;

	if (NULL != task) {
		printk(KERN_INFO "CHARDEV: processing signal to app...\n");
		if (0 > send_sig_info(SIG_LOTHAR, &info, task)) {
			printk(KERN_ERR "CHARDEV: Unable to send signal\n");
		}
	}

	// read functions are supposed to return the number of bytes
	// actually inserted into the buffer
	return 0;
}


/*
  Called whenever a process tries to do an ioctl on the device.

  @cmd: The number (id) of the ioctl operations that was called.
  @ioctl_param: The param given to the ioctl.

  Returns 0 in case of success.
*/
static long device_ioctl(struct file* file, unsigned int cmd, unsigned long ioctl_param)
{
	printk(KERN_INFO "CHARDEV: %s(%p, %u, %lu)\n", __func__, file, cmd, ioctl_param);
	switch (cmd) {
	case WR_VALUE:
		// the userspace application passes a signum (which itself may understand)
		copy_from_user(&signum, (int32_t*) ioctl_param, sizeof(signum));
		printk(KERN_INFO "CHARDEV: %s() sig = %d\n", __func__, signum);
		task = get_current();
		break;
	default:
		printk(KERN_ERR "CHARDEV: %s() unknown cmd\n", __func__);
		break;
	}
	return SUCCESS;
}


/*
  linux init & clean up
*/

int init_hello_ioctl(void)
{
	if (0 > alloc_chrdev_region(&dev, MINOR_NUM, 1, HELLO_DEVICE_FILENAME)) {
		printk(KERN_ERR "CHARDEV: device allocation failed!\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "CHARDEV: %s() - major = %d, minor = %d\n", __func__, MAJOR(dev), MINOR(dev));


	cdev_init(&hello_chardev, &hello_chardev_fops);


	if (0 > cdev_add(&hello_chardev, dev, 1)) {
		printk(KERN_ERR "CHARDEV: %s() adding char device to the system failed\n", __func__);
		goto err_cdev;
	}


	dev_class = class_create(THIS_MODULE, HELLO_CLASS_NAME);
	if (NULL == dev_class) {
		printk(KERN_ERR "CHARDEV: %s() creating a struct class structure failed\n", __func__);
		goto err_class;
	}


	if (NULL == device_create(dev_class, NULL, dev, NULL, HELLO_DEVICE_NAME)) {
		printk(KERN_ERR "CHARDEV: %s() \n", __func__);
		goto err_device;
	}
	printk(KERN_INFO "CHARDEV: %s() device driver init - OK\n", __func__);


	return SUCCESS;


err_device:
	class_destroy(dev_class);

err_class:
	cdev_del(&hello_chardev);

err_cdev:
	unregister_chrdev_region(dev, 1);

	return -ENOMEM;
}

void cleanup_hello_ioctl(void)
{
	/**
	 * device_destroy() - removes a device that was created with device_create()
	 * @class: pointer to the struct class that this device was registered with
	 * @devt: the dev_t of the device that was previously registered
	 *
	 * This call unregisters and cleans up a device that was created with a
	 * call to device_create().
	 */
	device_destroy(dev_class, dev);


	/**
	 * class_destroy() - destroys a struct class structure
	 * @cls: pointer to the struct class that is to be destroyed
	 *
	 * Note, the pointer to be destroyed must have been created with a call
	 * to class_create().
	 */
	class_destroy(dev_class);


	/**
	 * cdev_del() - remove a cdev from the system
	 * @p: the cdev structure to be removed
	 *
	 * cdev_del() removes @p from the system, possibly freeing the structure
	 * itself.
	 *
	 * NOTE: This guarantees that cdev device will no longer be able to be
	 * opened, however any cdevs already open will remain and their fops will
	 * still be callable even after cdev_del returns.
	 */
	cdev_del(&hello_chardev);


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

	printk(KERN_INFO "CHARDEV: character device unregistered\n");
	printk(KERN_INFO "CHARDEV: READY.\n");
}
