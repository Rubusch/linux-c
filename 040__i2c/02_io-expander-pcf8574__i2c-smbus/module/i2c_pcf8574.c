/*
  I2C Client Demo: PCF8574

  Demonstrates the communication via smbus routines
  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018

  VERIFIED:
  linux v6.3/aarch64
*/

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

/* private device structure */
struct ioexp_dev {
	struct i2c_client *client;
	struct miscdevice ioexp_miscdevice;
	char name[8];
};

/* user is reading data from /dev/lothars_device */
static ssize_t
ioexp_read_file(struct file *file, char __user *userbuf, size_t count, loff_t *ppos)
{
	int expval, size;
	char buf[3];
	struct ioexp_dev *ioexp;

	// obtain ioexp device from file->private_data
	ioexp = container_of(file->private_data,
			     struct ioexp_dev,
			     ioexp_miscdevice);

	// store io expander input to expval int variable
	expval = i2c_smbus_read_byte(ioexp->client);
	if (0 > expval)
		return -EFAULT;

	// converts expval int value into a char string
	// for instance
	// 255 int (4 bytes) = FF (2 bytes) + '\0' (1 byte) string
	size = sprintf(buf, "%02x", expval); // size is 2

	// replace NULL by \n; it is not needded to have a char array
	// ended with \0
	buf[size] = '\n';

	// send size+1 to include the \n character
	if (*ppos == 0) {
		if (copy_to_user(userbuf, buf, size+1)) {
			pr_warn("failed to return led_value to user space\n");
			return -EFAULT;
		}
		*ppos += 1;
		return size+1;
	}
	return 0;
}

// writing from the terminal command line to /dev/lothars_device, \n is added
static ssize_t ioexp_write_file(struct file *file, const char __user *userbuf, size_t count, loff_t *ppos)
{
	int ret = -1;
	unsigned long val;
	char buf[4];
	struct ioexp_dev *ioexp;
	struct i2c_client *client;

	ioexp = container_of(file->private_data,
			     struct ioexp_dev,
			     ioexp_miscdevice);

	client = ioexp->client;
	dev_info(&ioexp->client->dev,
		 "ioexp_write_file() started, entered on '%s'\n", ioexp->name);

	// convert "char __user*" to kernel pointer: copy_from_user() => buf
	if (copy_from_user(buf, userbuf, count)) {
		dev_err(&ioexp->client->dev, "ioexp_write_file() bad copied value\n");
		return -EFAULT;
	}
	buf[count-1] = '\0'; // replace \n with \0

	// convert the string to an unsigned long [string is supposed
	// to be a number] => val
	ret = kstrtoul(buf, 0, &val);
	if (ret) {
		return -EINVAL;
	}

	dev_info(&ioexp->client->dev,
		 "ioexp_write_file() convert str to unsigned long, the value is %lu [0x%02lX]\n",
		 val, val);

	// generally prefer the '..._smbus_...()' functions for i2c
	// initiate an smbus write byte process with val
	ret = i2c_smbus_write_byte(ioexp->client, val);
	udelay(500);

	if (0 > ret) {
		dev_warn(&client->dev,
			 "!!!ioexp_write_file() the device is not found!!!\n");
	}

	dev_info(&client->dev,
		 "ioexp_write_file() we have written %d characters\n", ret);

	dev_info(&client->dev, "ioexp_write_file() exited on %s\n", ioexp->name);
	dev_info(&client->dev, "ioexp_write_file() done\n");

	return count;
}

// fops - needed for initialization of the miscdevice
static const struct file_operations ioexp_fops = {
	.owner = THIS_MODULE,
	.read = ioexp_read_file,
	.write = ioexp_write_file,
};

// the probe() function is called twice
static int ioexp_probe(struct i2c_client* client)
{
	static int counter = 0;
	struct ioexp_dev *ioexp;

	dev_info(&client->dev, "ioexp_probe() started\n");

	// allocate new private structure
	// use devm_kzalloc() inside *_probe() as device scope alloc
	ioexp = devm_kzalloc(&client->dev, sizeof(struct ioexp_dev), GFP_KERNEL);
	if (!ioexp)
		return -ENOMEM;

	// store pointer to the device-structure in bus device context
	i2c_set_clientdata(client, ioexp);

	// set address (base): pcf8574

	// preamble
        // PCF8574:  0b0100 -> (0x40 >>1): 0x20
	//
	// initial msg:  |s|0|1|0| |0|a2|a1|a0|
	//   rw: where "write" == 0
	//   set the address by the three jumpers,
	// e.g. set all to "-" [0]
	//   adress: 0|0|0     -> 0x20
	//
	// command: enable p0... will be or'ed
	// ...
	//
	client->addr = 0x20;

	// store pointer to I2C client device in the private structure
	ioexp->client = client;

	// setup /dev/ioexp00 and /dev/ioexp01 as miscdevice
	sprintf(ioexp->name, "ioexp%02d", counter++);
	dev_info(&client->dev, "ioexp_probe() is entered on '%s'\n", ioexp->name);
	ioexp->ioexp_miscdevice.name = ioexp->name;
	ioexp->ioexp_miscdevice.minor = MISC_DYNAMIC_MINOR;
	ioexp->ioexp_miscdevice.fops = &ioexp_fops;

	// register misc device
	return misc_register(&ioexp->ioexp_miscdevice);
}

static void ioexp_remove(struct i2c_client *client)
{
	struct ioexp_dev* ioexp;

	dev_info(&client->dev, "ioexp_remove() started\n");

	// get device structure from bus device context
	ioexp = i2c_get_clientdata(client);

	dev_info(&client->dev, "ioexp_remove() is entered on %s\n", ioexp->name);

	// deregister misc device
	misc_deregister(&ioexp->ioexp_miscdevice);

	dev_info(&client->dev, "ioexp_remove() is exited on %s\n", ioexp->name);
	dev_info(&client->dev, "ioexp_remove() done\n");
}

// device tree registering / handling
static const struct of_device_id ioexp_dt_ids[] = {
	{ .compatible = "lothars,ioexp", },
	{ }
};
MODULE_DEVICE_TABLE(of, ioexp_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
	{ .name = "ioexp", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

static struct i2c_driver ioexp_driver = {
	.probe = ioexp_probe,
	.remove = ioexp_remove,
	.id_table = i2c_ids,
	.driver = {
		.name = "ioexp",
		.owner = THIS_MODULE,
		.of_match_table = ioexp_dt_ids,
	},
};

module_i2c_driver(ioexp_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("i2c smbus demo on the pcf8574");
