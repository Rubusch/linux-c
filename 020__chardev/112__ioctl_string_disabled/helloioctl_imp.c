// SPDX-License-Identifier: GPL-2.0+
/*
*/

#include "helloioctl.h"

/*
  forwards
*/

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *, unsigned int, unsigned long);

/*
  globals
*/

/*
// is the device open right now? used to prevent concurent access into
// the same device
static int Device_used = 0;  

// message the device will give when asked
static char Message[BUF_SIZ];  

// message is larger than the size of the buffer -> fill in
// device_read
static char* Ptr_message = NULL;  
// */

// content the device will give when asked
int32_t value = 0;

// dev_t object to hold MAJOR and MINOR number of the device
dev_t dev = 0;

//
static struct class *dev_class;

// the character device as a struct object
static struct cdev hello_chardev;

// TODO rm
// a pointer to this is kept in the device table, thus it can't be
// local (static) to init_module unimplemented funcs have NULL
//struct file_operations hello_chardev_fops = {
//	.read = device_read,
//	.write = device_write,
//	.ioctl = device_ioctl,
//	.open = device_open,
//	.release = device_release, // same as close
//};
static struct file_operations hello_chardev_fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
};

/*
  function implementation
*/

/*
  Called whenever a process attempts to open the device file.

  Returns 0 if success
*/
static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "%s(%p)\n", __func__, file);
	/*
//  NB: nowadays can be left empty, formerly something as below
//  happened here (see Salzmann)

	// don't want to talk to 2 processes at the same time
	if (Device_used) return -EBUSY;

	++Device_used;
	Ptr_message = Message;
	try_module_get(THIS_MODULE);
*/
	return SUCCESS;
}

/*
  Called when the device file is closed.

  Returns 0 if success.
*/
static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "%s(%p, %p)\n", __func__, inode, file);
	/*
//  NB: nowadays can be left empty, formerly something as below
//  happened here (see Salzmann)

	// ready for our next caller
	--Device_used;
	module_put(THIS_MODULE);
*/
	return SUCCESS;
}

/*
  Called whenever a process which has already opened the device file,
  attempts to read form it.

  Returns the number of bytes read.
*/
static ssize_t device_read(struct file *file, char __user *buffer,
			   size_t length, loff_t *offset)
{
	// number of bytes actually written to the buffer
	int bytes_read = 0;

	printk(KERN_INFO "%s(%p, %p, %ld)\n", __func__, file, buffer, length);

	/*
//  NB: nowadays can be left empty, formerly something as below
//  happened here (see Salzmann)

	// if we're at the end of the message, return 0 (eof)
	if(0 == *Ptr_message){
		return 0;
	}

	// actually put the data into the buffer
	while(length && *Ptr_message){
		// ATTENTION: data is in the user segment
		put_user(*(++Ptr_message), ++buffer);
		--length;
		++bytes_read;
	}

	printk(KERN_INFO "read %d bytes, %ld left\n", bytes_read, length);
*/
	// read functions are supposed to return the number of bytes actually inserted into the buffer
	return bytes_read;
}

/*
  Called when someone tries to write into our device file.

  Returns the number of input characters used.
*/
static ssize_t device_write(struct file *file, const char __user *buffer,
			    size_t length, loff_t *offset)
{
	//	int idx = 0;
	printk(KERN_INFO "%s(%p, %s, %ld)", __func__, file, buffer, length);

	/*
//  NB: nowadays can be left empty, formerly something as below
//  happened here (see Salzmann)

	for (idx = 0; (idx < length) && (idx < BUF_SIZ); ++idx) {
		get_user(Message[idx], buffer + idx);
	}
	Ptr_message = Message;

	// return the number of the input characters used
	return idx;
/*/
	return 0;
	// */
}

/*
  Called whenever a process tries to do an ioctl on the device.

  @ioctl_cmd: The number (id) of the ioctl operations that was called.
  @ioctl_param: The param given to the ioctl.

  Returns 0 in case of success.
*/
static long device_ioctl(struct file *file, unsigned int ioctl_cmd,
			 unsigned long ioctl_param)
{
	//	int idx = 0;
	//	char *tmp = NULL;
	//	char ch = 0;

	printk(KERN_INFO "%s(%p, %u, %lu)\n", __func__, file, ioctl_cmd,
	       ioctl_param);
	/*
//  NB: nowadays can be left empty, formerly something as below
//  happened here (see Salzmann)
	// according to the ioctl that was called
	switch (ioctl_cmd) {
	case IOCTL_SET_MSG:
		// receive a pointer to a message (user space) and set
		// that to be the device's message get the param given
		// to ioctl by the process
		tmp = (char*) ioctl_param;

		// determine length of the message
		get_user(ch, tmp);
		for(idx = 0; ch && (idx < BUF_SIZ); ++idx, ++tmp){
			get_user(ch, tmp);
		}
		device_write(file, (char*) ioctl_param, idx, 0);
		break;

	case IOCTL_GET_MSG:
		// pass current message to the calling process - the
		// parameter we got is a pointer, fill it
		idx = device_read(file, (char*) ioctl_param, 99, 0);

		// put a zero at the end of the buffer, so it will be
		// properly terminated
		put_user('\0', (char*) ioctl_param + idx);
		break;

	case IOCTL_GET_NTH_BYTE:
		// input (ioctl_param) and output (return value of
		// this function)
		return Message[ioctl_param];
		break;
	}
/*/
	switch (ioctl_cmd) {
	case WR_VALUE:
		if (copy_from_user(&value, (int32_t *)ioctl_param, sizeof(value)))
			return -EFAULT;
		printk(KERN_INFO "%s() value = %d\n", __func__,
		       value); // TODO check value
		break;
	case RD_VALUE:
		if (copy_to_user((int32_t *)ioctl_param, &value, sizeof(value)))
			return -EFAULT;
		break;
	default:
		printk(KERN_ALERT "%s() unknown ioctl_cmd\n", __func__);
		break;
	}
	return SUCCESS;
}

/*
  linux init & clean up
*/

int init_hello_ioctl(void)
{
	/*
	int ret = 0;

	// register the character device
	if (0 > (ret = register_chrdev(MAJOR_NUM, HELLO_DEVICE_NAME, &hello_chardev_fops))) {
		printk(KERN_ALERT "sorry, failed registering the char dev with %d\n", ret);
		return ret;
	}
/*/
	/**
	 * alloc_chrdev_region() - register a range of char device numbers
	 * @dev: output parameter for first assigned number
	 * @baseminor: first of the requested range of minor numbers
	 * @count: the number of minor numbers required
	 * @name: the name of the associated device or driver
	 *
	 * Allocates a range of char device numbers.  The major number will be
	 * chosen dynamically, and returned (along with the first minor number)
	 * in @dev.  Returns zero or a negative error code.
	 */
	if (0 >
	    alloc_chrdev_region(&dev, MAJOR_NUM, 1, HELLO_DEVICE_FILENAME)) {
		printk(KERN_ALERT "device allocation failed!\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "%s() - major = %d, minor = %d\n", __func__,
	       MAJOR(dev), MINOR(dev));

	/**
	 * cdev_init() - initialize a cdev structure
	 * @cdev: the structure to initialize
	 * @fops: the file_operations for this device
	 *
	 * Initializes @cdev, remembering @fops, making it ready to add to the
	 * system with cdev_add().
	 */
	cdev_init(&hello_chardev, &hello_chardev_fops);

	/**
	 * cdev_add() - add a char device to the system
	 * @p: the cdev structure for the device
	 * @dev: the first device number for which this device is responsible
	 * @count: the number of consecutive minor numbers corresponding to this
	 *         device
	 *
	 * cdev_add() adds the device represented by @p to the system, making it
	 * live immediately.  A negative error code is returned on failure.
	 */
	if (0 > cdev_add(&hello_chardev, dev, 1)) {
		printk(KERN_ALERT
		       "%s() adding char device to the system failed\n",
		       __func__);
		goto err_cdev;
	}

	/**
	 * class_create - create a struct class structure
	 * @owner: pointer to the module that is to "own" this struct class
	 * @name: pointer to a string for the name of this class.
	 * @key: the lock_class_key for this class; used by mutex lock debugging
	 *
	 * This is used to create a struct class pointer that can then be used
	 * in calls to device_create().
	 *
	 * Returns &struct class pointer on success, or ERR_PTR() on error.
	 *
	 * Note, the pointer created here is to be destroyed when finished by
	 * making a call to class_destroy().
	 */
	dev_class = class_create(THIS_MODULE, HELLO_CLASS_NAME);
	if (NULL == dev_class) {
		printk(KERN_ALERT
		       "%s() creating a struct class structure failed\n",
		       __func__);
		goto err_class;
	}

	/**
	 * device_create() - creates a device and registers it with sysfs
	 * @class: pointer to the struct class that this device should be registered to
	 * @parent: pointer to the parent struct device of this new device, if any
	 * @devt: the dev_t for the char device to be added
	 * @drvdata: the data to be added to the device for callbacks
	 * @fmt: string for the device's name
	 *
	 * This function can be used by char device classes.  A struct device
	 * will be created in sysfs, registered to the specified class.
	 *
	 * A "dev" file will be created, showing the dev_t for the device, if
	 * the dev_t is not 0,0.
	 * If a pointer to a parent struct device is passed in, the newly created
	 * struct device will be a child of that device in sysfs.
	 * The pointer to the struct device will be returned from the call.
	 * Any further sysfs files that might be required can be created using this
	 * pointer.
	 *
	 * Returns &struct device pointer on success, or ERR_PTR() on error.
	 *
	 * Note: the struct class passed to this function must have previously
	 * been created with a call to class_create().
	 */
	if (NULL ==
	    device_create(dev_class, NULL, dev, NULL, HELLO_DEVICE_NAME)) {
		printk(KERN_ALERT "%s() \n", __func__);
		goto err_device;
	}
	printk(KERN_INFO "%s() device driver init - OK\n", __func__);
	// */

	printk(KERN_INFO "If you want to talk to the device driver,\n");
	printk(KERN_INFO "you'll have to create a device file, do a:\n");
	printk(KERN_INFO "$ sudo mknod %s c %d 0\n", HELLO_DEVICE_FILENAME,
	       MAJOR_NUM);
	printk(KERN_INFO "the device file name is important, because\n");
	printk(KERN_INFO "the ioctl program assumes that's the\n");
	printk(KERN_INFO "file you'll use.\n");

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

	//	unregister_chrdev(MAJOR_NUM, HELLO_DEVICE_NAME);

	printk(KERN_INFO "character device unregistered\n");
	printk(KERN_INFO "READY.\n");
}
