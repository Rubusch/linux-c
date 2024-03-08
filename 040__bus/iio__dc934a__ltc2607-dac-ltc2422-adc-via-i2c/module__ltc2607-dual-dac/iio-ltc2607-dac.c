// SPDX-License-Identifier: GPL-2.0+
/*
  Demo for the industrial I/O API.

      +-----------------------+
  +-->| struct ltc2607_device |<----+
  |   +-----------------------+     |
  |   |      +--------+       |     |
  |   |      | client |---------------------------------------------------+
  |   |      +--------+       |     |                                     |
  |   +-----------------------+     |                                     |
  |                                 |                                     |
  |                                 |                                     |
  |                                 |                                     |
  |   +-----------------------+     |        +-----------------------+    |
  +---| struct iio_dev        |     |        | struct i2c_client     |<---+
      +-----------------------+     |        +-----------------------+
      |   +---------------+   |     |        |                       |
      |   | device        |   |     |        |                       |
      |   +---------------+   |     |        |   +---------------+   |
      |   | parent        |--------------------->| device        |   |
      |   +---------------+   |     |        |   +---------------+   |
      |   | void*         |---------+------------| void*         |   |
      |   |   driver_data |   |              |   |   driver_data |   |
      |   +---------------+   |              |   +---------------+   |
      +-----------------------+              +-----------------------+
*/

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>

#define LTC2607_DRV_NAME "ltc2607"

struct ltc2607_device {
	struct i2c_client *client;
	char name[8];
};

/*
  An iio device channel is a representation of a data channel. An iio
  device can have one or more channels. The iio channel definitions
  will generate the following data channel access attributes for
  iio:device0 and iio:device1

  /sys/bus/iio/devices/iio:device0/out_voltage0_raw
  /sys/bus/iio/devices/iio:device0/out_voltage1_raw
  /sys/bus/iio/devices/iio:device0/out_voltage2_raw

  /sys/bus/iio/devices/iio:device1/out_voltage0_raw
  /sys/bus/iio/devices/iio:device1/out_voltage1_raw
  /sys/bus/iio/devices/iio:device1/out_voltage2_raw
 */
static const struct iio_chan_spec ltc2607_channel[] = {
	{
		.type		= IIO_VOLTAGE,
		.indexed	= 1,
		.output		= 1,
		.channel	= 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type		= IIO_VOLTAGE,
		.indexed	= 1,
		.output		= 1,
		.channel	= 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type		= IIO_VOLTAGE,
		.indexed	= 1,
		.output		= 1,
		.channel	= 2,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	}
};

/* set value

   initialize the iio_device structure
 */
static int
ltc2607_set_value(struct iio_dev* indio_dev, int val, int channel)
{
	struct ltc2607_device *data = iio_priv(indio_dev);
	struct device *dev = indio_dev->dev.parent;
	u8 outbuf[sizeof(ltc2607_channel)/sizeof(*ltc2607_channel)];
	int chan;
	int ret;

	dev_info(dev, "%s() - called\n", __func__);
	dev_info(dev, "%s() - val '%d', channel '%d'\n", __func__, val, channel);

	chan = (2 == channel) ? 0x0f : channel;
	if (BIT(16) <= val || 0 > val)
		return -EINVAL;

	dev_info(dev, "%s() - chan '%02x' [0x00: DACa, 0x01: DACb, 0x0f: both DACs]\n", __func__, chan);

	outbuf[0] = 0x30 | chan;       // write and update DAC: 0x30
	dev_info(dev, "%s() - outbuf[0] '%02x'\n", __func__, outbuf[0]);

	outbuf[1] = (val >> 8) & 0xff; // MSB byte of dac_code
	dev_info(dev, "%s() - outbuf[1] '%02x'\n", __func__, outbuf[1]);

	outbuf[2] = val & 0xff;        // LSB byte of dac_code
	dev_info(dev, "%s() - outbuf[2] '%02x'\n", __func__, outbuf[2]);

	ret = i2c_master_send(data->client, outbuf, indio_dev->num_channels);
	if (0 > ret)
		return ret;
	else if (ret != indio_dev->num_channels)
		return -EIO;

	// else
	return 0;
}

/* write raw

   The function contains a switch(mask) that will set different tasks
   depending on the received parameter values. If the received
   info_mask value is [IIO_CHAN_INFO_RAW] = "raw", the
   ltc2607_set_value() function is called, recovering the private data
   through the iio_priv() function. Once the private info is
   recovered, the i2c device address will be retrieved from the i2c
   client pointer variable (data->client). This data->client pointer
   variable is the first parameter of the i2c_master_send() function,
   which is used to communicate with the Analog Devices DAC by writing
   each channel in an independent way, or in a simultaneous mode.

   Use the command value 0x30 to write and update the DAC device. The
   DAC values will range from 0 to 0xffff. For example, if the DAC
   value is set to 0xFFFF, then the DACa output is closer to 5V
   (VREF). Depending of the DAC address, the data will be written to
   DACa (0x00), DACb (0x01), or both DACs (0x0f). In the next table
   you can see all the commands and addresses in their respective
   descriptions.

   Output voltage = VREF x DAC value / 65535

   Taken from the data sheet, the commands and addresses in the I2C
   communication to the LTC2607 DAC.

   Write word protocol for the LTC2607:
   <S><slave address><W><A><1st data byte><A><2nd data byte><A><3rd data byte><A><P>
                           \---------input word---------------------------------/

   input word
   |C3|C2|C1|C0|A3|A2|A1|A0| |D15|D14|D13|D12|D11|D10|D9|D8| |D7|D6|D5|D4|D3|D2|D1|D0|

   C3-C0: command
   A3-A0: address (channel)

   command            |
   ----+----+----+----+--------------------------------
    c3 | c2 | c1 | c0 |
   ----+----+----+----+--------------------------------
    0  | 0  | 0  | 0  | write to input register
    0  | 0  | 0  | 1  | update (power up) DAC register
    0  | 0  | 1  | 1  | write to and update (power up) <---- we use 0x30 here
    0  | 1  | 0  | 0  | power down
    1  | 1  | 1  | 1  | no operation
   ----+----+----+----+--------------------------------
   address            |
    a3 | a2 | a1 | a0 |
   ----+----+----+----+--------------------------------
    0  | 0  | 0  | 0  | DAC a
    0  | 0  | 0  | 1  | DAC b
    1  | 1  | 1  | 1  | all DACs <------------------- we use "all" 0x0f here
 */
static int
ltc2607_write_raw(struct iio_dev* indio_dev, struct iio_chan_spec const *chan, int val, int val2, long mask)
{
	int ret;
	struct device *dev = indio_dev->dev.parent;

	dev_info(dev, "%s() - called\n", __func__);
	switch(mask) {
	case IIO_CHAN_INFO_RAW:
		dev_info(dev,
			 "%s() - case IIO_CHAN_INFO_RAW: val '%d', chan->channel '%d'\n",
			 __func__, val, chan->channel);
		ret = ltc2607_set_value(indio_dev, val, chan->channel);
		return ret;
	default:
		dev_info(dev, "%s() - case default\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static const struct iio_info ltc2607_info = {
	.write_raw = ltc2607_write_raw,
};

/*
 */
static int
ltc2607_probe(struct i2c_client *client)
{
	static int counter = 0;
	struct iio_dev *indio_dev;
	struct ltc2607_device *data;
	const struct i2c_device_id *id = i2c_client_get_device_id(client);
	struct device *dev = &client->dev;
	u8 inbuf[3];
	u8 command_byte;
	int ret;

	dev_info(dev, "%s() - called\n", __func__);

	// allocate the struct iio_dev
	indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
	if (!indio_dev) {
		dev_err(dev, "%s() - alloc indio_dev failed\n",
			__func__);
		return -ENOMEM;
	}

	/* iio/i2c - initialization (1/8)

	  To be able to access the private data structure in other
	  parts of the driver you need to attach it to the iio_dev
	  structure using the iio_priv() function. You will retrieve
	  the pointer "data" to the private structure using the same
	  function iio_priv()
	*/
	data = iio_priv(indio_dev);

	/* iio/i2c - init (2/8)

	  Keep pointer to the i2c device, needed for exchanging data
	  with the LTC2607 device.
	 */
	i2c_set_clientdata(client, data);
	data->client = client;

	/* iio/i2c - init (3/8)

	   Create a different name for each device attached to the
	   DT. In the driver two DAC names will be created, one for
	   each i2c address. Store the names in each private
	   structure. The probe() function will be called twice, once
	   per DT LTC2607 node found
	*/
	sprintf(data->name, "DAC%02d", counter++);
	dev_info(dev, "%s() - was called from %s\n",
		 __func__, data->name);

	/* iio/i2c - init (4/8)

	   Store the name in the iio device
	*/
	indio_dev->name = id->name;

	/* iio/i2c - init (5/8)

	   Keep pointers between physical devices (devices as handled
	   by the physical bus, I2C in this case) and logical devices
	*/
	indio_dev->dev.parent = &client->dev;

	/* iio/i2c - init (6/8)

	   Store the address of the iio_info structure which contains
	   a pointer variable to the IIO raw writing callback
	*/
	indio_dev->info = &ltc2607_info;

	/* iio/i2c - init (7/8)

	   Store address of the iio_chan_spec structure which stores
	   each channel info for the LTC2607 dual DAC
	*/
	indio_dev->channels = ltc2607_channel;

	/* iio/i2c - init (8/8)

	   Set number of the channels of the device: 3
	   (the number of the elements in the array ltc2607_channel, top)
	*/
	indio_dev->num_channels = (sizeof(ltc2607_channel)/sizeof(*ltc2607_channel));
	indio_dev->modes = INDIO_DIRECT_MODE;

	command_byte = 0x30 | 0x00; // write and update register with value 0xff
	inbuf[0] = command_byte;
	inbuf[1] = 0xff;
	inbuf[2] = 0xff;

        // write DAC value
	ret = i2c_master_send(client, inbuf, indio_dev->num_channels);
	if (0 > ret) {
		dev_err(dev, "%s() - i2c_master_send() failed\n",
			__func__);
		return ret;
	}
	dev_info(dev, "%s() - the DAC answer is '%x'\n",
		 __func__, ret);

	ret = devm_iio_device_register(dev, indio_dev);
	if (ret) {
		dev_err(dev, "%s() devm_iio_device_register() failed\n",
			__func__);
		return ret;
	}

	dev_info(dev, "%s() - ltc2607 DAC registered\n",
		 __func__);

	return 0;
}

static void
ltc2607_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "%s() - called\n",
		 __func__);
}

static const struct of_device_id dac_dt_ids[] = {
	{ .compatible = "lothars,ltc2607", },
	{},
};
MODULE_DEVICE_TABLE(of, dac_dt_ids);

static const struct i2c_device_id ltc2607_id[] = {
	{ "ltc2607", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, ltc2607_id);

static struct i2c_driver ltc2607_driver = {
	.driver = {
		.name = LTC2607_DRV_NAME,
		.of_match_table = dac_dt_ids,
	},
	.probe = ltc2607_probe,
	.remove = ltc2607_remove,
	.id_table = ltc2607_id,
};
module_i2c_driver(ltc2607_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("iio demo");
