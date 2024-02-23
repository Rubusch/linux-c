// SPDX-License-Identifier: GPL-2.0+
/*
  Input demo using the ADXL345 Accel click mikroBUS accessory board.

  The original version based on input-polldev.h, where
  linux/input-polldev.h does not exist anymore. The below
  implementation of a device driver thus, is fundamentally re-written
  and based on contemporary API.

  This is the I2C version.
*/


#include <linux/module.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/platform_device.h>


struct ioaccel_dev {
	struct input_dev* input;
	struct i2c_client *i2c_client;
};

/*
  set the measure bit (bit 3) in the POWER_CTL register, address 0x2d
*/
#define CMD_POWER_CTL 0x2D
#define CMDARG_PCTL_MEASURE (1 << 3)
#define OUT_X_MSB_REGISTER 0x33
#define IOACCEL_POLL_INTERVAL 50

/* poll function

   this function will be called every 50ms to read the OUT_X_MSB
   register (0x33) of the ADXL345 accelerometer by using the i2c/smbus
   read byte data function
 */
static void
ioaccel_poll(struct input_dev* input)
{
	struct ioaccel_dev *ioaccel = input_get_drvdata(input);
	int val = i2c_smbus_read_byte_data(
		ioaccel->i2c_client, OUT_X_MSB_REGISTER);

	input_event(input, EV_KEY, KEY_1,
		    (((0xC0 < val) && (0xff > val)) ? 1 : 0) );

	input_sync(input);
}

/* probe function
 */
static int
ioaccel_probe(struct i2c_client* client)
{
	struct device *dev = &client->dev;
	struct input_dev *input;
	struct ioaccel_dev *ioaccel;
	int error;

	dev_info(dev, "%s() - called", __func__);

	// allocate private structure for new device
	ioaccel = devm_kzalloc(dev, sizeof(*ioaccel), GFP_KERNEL);
	if (!ioaccel) {
		dev_err(dev, "%s() - ioaccel allocation failed", __func__);
		return -ENOMEM;
	}

	// allocate input device
	input = devm_input_allocate_device(dev);
	if (!input) {
		dev_err(dev, "%s() - input allocation failed", __func__);
		return -ENOMEM;
	}

	// init the private structure ioaccel
	ioaccel->input = input;
	ioaccel->i2c_client = client;

	// associate client with private structure ioaccel
	i2c_set_clientdata(client, ioaccel);

	error = i2c_smbus_write_byte_data(client, CMD_POWER_CTL, CMDARG_PCTL_MEASURE);
	if (error) {
		dev_warn(dev, "%s() - smbus write error %d\n", __func__, error);
		return error;
	}

	// register the ioaccel struct at the input device
	// (private driver data)
	input_set_drvdata(input, ioaccel);

	// init input device
	input->dev.parent = dev;
	input->name = "IOACCEL keyboard";
	input->phys = "ioaccel/input0";
	input->id.bustype = BUS_I2C;

	// set event types
	set_bit(EV_KEY, input->evbit);
	set_bit(KEY_1, input->keybit);

	/*
	   register the device, now the device is global until
	   unregistered
	*/

	// setup polling device
	error = input_setup_polling(input, ioaccel_poll);
	if (error)
		return error;

	// setup poll interval
	input_set_poll_interval(input, IOACCEL_POLL_INTERVAL);

	// register input device
	error = input_register_device(input);
	if (error)
		return error;

	return 0;
}

static const struct of_device_id ioaccel_dt_ids[] = {
	{ .compatible = "lothars,adxl345", },
	{},
};
MODULE_DEVICE_TABLE(of, ioaccel_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
	{ "adxl345", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

static struct i2c_driver ioaccel_driver = {
	.driver = {
		.name = "adxl345",
		.of_match_table = ioaccel_dt_ids,
	},
	.probe = ioaccel_probe,
	.id_table = i2c_ids,
};
module_i2c_driver(ioaccel_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Input layer demo");
