// SPDX-License-Identifier: GPL-2.0+
/*
  Input demo using the ADXL345 Accel click mikroBUS accessory board.

  This implementation is based on the SPI version of Alberto Liberal's
  accelerometer driver.  Extended with annotations, in case modernized
  and re-formatted to my needs.

  The IIO driver for the ADXL345 is upstream:
  https://elixir.bootlin.com/linux/v6.3.13/source/drivers/iio/accel/adxl345_core.c
  https://elixir.bootlin.com/linux/v6.3.13/source/drivers/iio/accel/adxl345_i2c.c
  https://elixir.bootlin.com/linux/v6.3.13/source/drivers/iio/accel/adxl345_spi.c
  https://elixir.bootlin.com/linux/v6.3.13/source/drivers/iio/accel/adxl345.h
*/

#include <linux/input.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>

#define ADXL345_CMD_MULTB	BIT(6)
#define ADXL345_CMD_READ	BIT(7)
#define ADXL345_CMDMASK		0x3f

#define ADXL345_WRITE(reg) (reg & ADXL345_CMDMASK)
#define ADXL345_READ(reg) (ADXL345_CMD_READ | (reg & ADXL345_CMDMASK))
#define ADXL345_READMB(reg) \
	(ADXL345_CMD_READ | ADXL345_CMD_MULTB | (reg & ADXL345_CMDMASK))

/* ADXL345 register map */
#define ADXL345_REG_DEVID		0x00	/* R   Device ID */
#define ADXL345_REG_THRESH_TAP		0x1D	/* R/W Tap threshold */
#define ADXL345_REG_DUR			0x21	/* R/W Tap duration */
#define ADXL345_REG_TAP_AXES		0x2A	/* R/W Axis control for tap/double tap */
#define ADXL345_REG_ACT_TAP_STATUS	0x2B	/* R   Source of tap/double tap */
#define ADXL345_REG_BW_RATE		0x2C	/* R/W Data rate and power mode control */
#define ADXL345_REG_POWER_CTL		0x2D	/* R/W Power saving features control */
#define ADXL345_REG_INT_ENABLE		0x2E	/* R/W Interrupt enable control */
#define ADXL345_REG_INT_MAP		0x2F	/* R/W Interrupt mapping control */
#define ADXL345_REG_INT_SOURCE		0x30	/* R   Source of interrupts */
#define ADXL345_REG_DATA_FORMAT		0x31	/* R/W Data format control */
#define ADXL345_REG_DATAX0		0x32	/* R   X-Axis Data 0 */
#define ADXL345_REG_DATAX1		0x33	/* R   X-Axis Data 1 */
#define ADXL345_REG_DATAY0		0x34	/* R   Y-Axis Data 0 */
#define ADXL345_REG_DATAY1		0x35	/* R   Y-Axis Data 1 */
#define ADXL345_REG_DATAZ0		0x36	/* R   Z-Axis Data 0 */
#define ADXL345_REG_DATAZ1		0x37	/* R   Z-Axis Data 1 */
#define ADXL345_REG_FIFO_CTL		0x38	/* R/W FIFO control */

/* ADXL345_REG_DEVIDs */
#define ID_ADXL345	0xE5

/* INT_ENABLE / INT_MAP / INT_SOURCE bits */
#define SINGLE_TAP	BIT(6)

/* TAP_AXES bits */
#define TAP_X_EN	BIT(2)
#define TAP_Y_EN	BIT(1)
#define TAP_Z_EN	BIT(0)

/* BW_RATE Bits */
#define LOW_POWER	BIT(4)
#define RATE(x)		((x) & 0xF)

/* POWER_CTL bits */
#define PCTL_MEASURE	BIT(3)
#define PCTL_STANDBY	0X00

/* DATA_FORMAT bits */
#define FULL_RES	BIT(3)

/* FIFO_CTL bits */
#define FIFO_MODE(x)	(((x) & 0x3) << 6)
#define FIFO_BYPASS	0
#define FIFO_FIFO	1
#define FIFO_STREAM	2
#define SAMPLES(x)	((x) & 0x1F)

/* FIFO_STATUS Bits */
#define ADXL_X_AXIS	0
#define ADXL_Y_AXIS	1
#define ADXL_Z_AXIS	2

#define ADXL345_GPIO_NAME	"int"

/* Macros to do SPI operations */
#define AC_READ(ac, reg)	((ac)->bops->read((ac)->dev, reg))
#define AC_WRITE(ac, reg, val)	((ac)->bops->write((ac)->dev, reg, val))

struct adxl345_bus_ops {
	u16 bustype;
	int (*read)(struct device *, unsigned char);
	int (*read_block)(struct device *, unsigned char, int, void *);
	int (*write)(struct device *, unsigned char, unsigned char);
};

struct axis_triple {
	int x;
	int y;
	int z;
};

/* struct platform data

   After definition, set initial adxl345 register values.
*/
struct adxl345_platform_data {
	/*
	  low_power_mode:

	  A '0' = Normal operation, and a '1' = Reduced power operation
	  with somewhat higher noise.
	*/
	u8 low_power_mode;

	/*
	  tap_threshold:

	  Hold the threshold value for tap detection/interrupts. The
	  data format is unsigned. The scale factor is 62.5 mg/LSB
	  (i.e. 0xFF = +16 g). A zero value may result in undesirable
	  behavior if Tap/Double Tap is enabled.
	*/
	u8 tap_threshold;

	/*
	  tap_duration:

	  An unsigned time value representing the maximum time that an
	  event must be above the tap_threshold threshold to qualify
	  as a tap event. The scale factor is 625 us/LSB. A zero value
	  will prevent Tap/Double Tap functions from working.
	*/
	u8 tap_duration;

	/*
	  TAP_X/Y/Z Enable: Setting TAP_X, Y, or Z Enable enables X,
	  Y, or Z participation in Tap detection. A '0' excludes the
	  selected axis from participation in Tap detection.

	  Setting the SUPPRESS bit suppresses Double Tap detection if
	  acceleration greater than tap_threshold is present during
	  the tap_latency period, i.e. after the first tap but before
	  the opening of the second tap window.
	*/

#define ADXL_TAP_X_EN	BIT(2)
#define ADXL_TAP_Y_EN	BIT(1)
#define ADXL_TAP_Z_EN	BIT(0)

	u8 tap_axis_control;

	/*
	  data_rate:

	  Selects device bandwidth and output data rate.

	  RATE = 3200 Hz / (2^(15 - x)). Default value is 0x0A, or 100
	  Hz Output Data Rate. An Output Data Rate should be selected
	  that is appropriate for the communication protocol and
	  frequency selected. Selecting too high of an Output Data
	  Rate with a low communication speed will result in samples
	  being discarded.
	*/
	u8 data_rate;

	/*
	  data_range:

	  FULL_RES: When this bit is set, the device is in
	  Full-Resolution Mode, where the output resolution increases
	  with RANGE to maintain a 4 mg/LSB scale factor. When this
	  bit is cleared the device is in 10-bit Mode and RANGE
	  determine the maximum g-Range and scale factor.
	*/

#define ADXL_FULL_RES		BIT(3)
#define ADXL_RANGE_PM_2g	0
#define ADXL_RANGE_PM_4g	1
#define ADXL_RANGE_PM_8g	2
#define ADXL_RANGE_PM_16g	3

	u8 data_range;

	/*
	  A valid BTN or KEY Code; use tap_axis_control to disable
	  event reporting
	*/
	u32 ev_code_tap[3];	/* EV_KEY {X-Axis, Y-Axis, Z-Axis} */

	/*
	  fifo_mode:

	  BYPASS  The FIFO is bypassed

	  FIFO    FIFO collects up to 32 values then stops collecting
	  data

	  STREAM  FIFO holds the last 32 data values. Once full, the
	  FIFO's oldest data is lost as it is replaced with
	  newer data

	  DEFAULT should be FIFO_STREAM
	*/
	u8 fifo_mode;

	/*
	  watermark:

	  The Watermark feature can be used to reduce the interrupt
	  load of the system. The FIFO fills up to the value stored in
	  watermark [1..32] and then generates an interrupt.

	  A '0' disables the watermark feature.
	*/
	u8 watermark;

};
static const struct adxl345_platform_data adxl345_default_init = {
	.tap_threshold = 50,
	.tap_duration = 3,
	//.tap_axis_control = ADXL_TAP_X_EN | ADXL_TAP_Y_EN | ADXL_TAP_Z_EN,
	.tap_axis_control = ADXL_TAP_Z_EN,
	.data_rate = 8,
	.data_range = ADXL_FULL_RES,
	.ev_code_tap = {BTN_TOUCH, BTN_TOUCH, BTN_TOUCH}, /* EV_KEY {x,y,z} */
	//.fifo_mode = ADXL_FIFO_STREAM,
	.fifo_mode = FIFO_BYPASS,
	.watermark = 0,
};

/* struct private data

   create private data structure
*/
struct adxl345 {
	struct gpio_desc *gpio;
	struct device *dev;
	struct input_dev *input;
	struct adxl345_platform_data pdata;
	struct axis_triple saved;
	u8 phys[32];
	int irq;
	u32 model;
	u32 int_mask;
	const struct adxl345_bus_ops *bops;
};

/* get triple

   Get the adxl345 axis data.
*/
static void adxl345_get_triple(struct adxl345 *ac, struct axis_triple *axis)
{
	__le16 buf[3];
	pr_info("%s(): called\n", __func__);

	ac->bops->read_block(ac->dev, ADXL345_REG_DATAX0, ADXL345_REG_DATAZ1 - ADXL345_REG_DATAX0 + 1, buf);

	ac->saved.x = sign_extend32(le16_to_cpu(buf[0]), 12);
	axis->x = ac->saved.x;

	ac->saved.y = sign_extend32(le16_to_cpu(buf[1]), 12);
	axis->y = ac->saved.y;

	ac->saved.z = sign_extend32(le16_to_cpu(buf[2]), 12);
	axis->z = ac->saved.z;
}

/* send key events

   This function is called inside adxl34x_do_tap() in the ISR when
   there is a SINGLE_TAP event. The function check the ACT_TAP_STATUS
   (0x2B) TAP_X, TAP_Y, TAP_Z bits starting for the TAP_X source
   bit. If the axis is involved in the event there is a EV_KEY event
*/
static void
adxl345_send_key_events(struct adxl345 *ac,
			struct adxl345_platform_data *pdata,
			int status,
			int press)
{
	int i;
	pr_info("%s(): called\n", __func__);

	for (i = ADXL_X_AXIS; i <= ADXL_Z_AXIS; i++) {
		if (status & (1 << (ADXL_Z_AXIS - i)))
			input_report_key(ac->input,
					 pdata->ev_code_tap[i], press);
	}
}

/* do tap

   Function called in the ISR when there is a SINGLE_TAP event
*/
static void
adxl345_do_tap(struct adxl345 *ac,
	       struct adxl345_platform_data *pdata,
	       int status)
{
	pr_info("%s(): called\n", __func__);

	adxl345_send_key_events(ac, pdata, status, true);
	input_sync(ac->input);
	adxl345_send_key_events(ac, pdata, status, false);
}

/* interrupt handler

   Interrupt service routine (isr)
*/
static irqreturn_t
adxl345_irq(int irq, void *handle)
{
	struct adxl345 *ac = handle;
	struct adxl345_platform_data *pdata = &ac->pdata;
	int int_stat, tap_stat;

	pr_info("%s(): called\n", __func__);

	/*
	  ACT_TAP_STATUS should be read before clearing the interrupt.
	  Avoid reading ACT_TAP_STATUS in case TAP detection is
	  disabled. Read the ACT_TAP_STATUS if any of the axis has been
	  enabled.
	*/
	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
		tap_stat = AC_READ(ac, ADXL345_REG_ACT_TAP_STATUS);
	else
		tap_stat = 0;

	/*
	  Read the INT_SOURCE (0x30) register. The interrupt is
	  cleared.
	*/
	int_stat = AC_READ(ac, ADXL345_REG_INT_SOURCE);

	/*
	  If the SINGLE_TAP event has occurred the adxl345_do_tap
	  function is called with the ACT_TAP_STATUS register as an
	  argument.
	*/
	if (int_stat & (SINGLE_TAP)) {
		pr_info("%s(): single tap interrupt has occurred\n", __func__);
		adxl345_do_tap(ac, pdata, tap_stat);
	}

	input_sync(ac->input);

	return IRQ_HANDLED;
}

static ssize_t
adxl345_rate_show(struct device *dev,
		  struct device_attribute *attr,
		  char *buf)
{
	struct adxl345 *ac = dev_get_drvdata(dev);

	pr_info("%s(): called\n", __func__);

	return sprintf(buf, "%u\n", RATE(ac->pdata.data_rate));
}

static ssize_t
adxl345_rate_store(struct device *dev,
		   struct device_attribute *attr,
		   const char *buf, size_t count)
{
	struct adxl345 *ac = dev_get_drvdata(dev);
	u8 val;
	int error;

	pr_info("%s(): called\n", __func__);

	// transform char array to u8 value
	error = kstrtou8(buf, 10, &val);
	if (error)
		return error;

	// if I set ac->pdata.low_power_mode = 1; then lower power
	// mode but higher noise is selected getting LOW_POWER macro,
	// by default ac->pdata.low_power_mode = 0; RATE(val) sets to
	// 0 the 4 upper u8 bits
	ac->pdata.data_rate = RATE(val);
	AC_WRITE(ac, ADXL345_REG_BW_RATE,
		 ac->pdata.data_rate |
		 (ac->pdata.low_power_mode ? LOW_POWER : 0));

	return count;
}
static DEVICE_ATTR(rate, 0664, adxl345_rate_show, adxl345_rate_store);


static ssize_t
adxl345_position_show(struct device *dev,
		      struct device_attribute *attr,
		      char *buf)
{
	struct adxl345 *ac = dev_get_drvdata(dev);
	ssize_t count;

	pr_info("%s(): called\n", __func__);
	count = sprintf(buf, "(%d, %d, %d)\n",
			ac->saved.x, ac->saved.y, ac->saved.z);

	return count;
}
static DEVICE_ATTR(position, S_IRUGO, adxl345_position_show, NULL);

static ssize_t
adxl345_position_read(struct device *dev,
		      struct device_attribute *attr,
		      char *buf)
{
	struct axis_triple axis;
	ssize_t count;
	struct adxl345 *ac = dev_get_drvdata(dev);

	pr_info("%s(): called\n", __func__);
	adxl345_get_triple(ac, &axis);

	count = sprintf(buf, "(%d, %d, %d)\n",
			axis.x, axis.y, axis.z);

	return count;
}
static DEVICE_ATTR(read, S_IRUGO, adxl345_position_read, NULL);

static struct attribute *adxl345_attributes[] = {
	&dev_attr_rate.attr,
	&dev_attr_position.attr,
	&dev_attr_read.attr,
	NULL
};

static const struct attribute_group adxl345_attr_group = {
	.attrs = adxl345_attributes,
};

struct adxl345*
adxl345_probe(struct device *dev, const struct adxl345_bus_ops *bops)
{
	struct adxl345 *ac; // declare our private structure
	struct input_dev *input_dev;
	const struct adxl345_platform_data *pdata;
	int err;
	u8 revid;

	pr_info("%s(): called\n", __func__);

	// allocate private structure
	ac = devm_kzalloc(dev, sizeof(*ac), GFP_KERNEL);
	if (!ac) {
		pr_err("%s(): FAILED to allocate memory\n", __func__);
		err = -ENOMEM;
		goto err_out;
	}

	// allocate the input_dev structure
	input_dev = devm_input_allocate_device(dev);
	if (!input_dev) {
		pr_err("%s(): FAILED to allocate input device\n", __func__);
		err = -ENOMEM;
		goto err_out;
	}

	/* initialize private structure */

	// store the previously initialized platform data in our
	// private structure
	pdata = &adxl345_default_init;
	ac->pdata = *pdata;
	pdata = &ac->pdata;
	ac->input = input_dev;
	ac->dev = dev;

	// store the SPI operations in our private structure
	ac->bops = bops;

	input_dev->name = "ADXL345 accelerometer";
	revid = AC_READ(ac, ADXL345_REG_DEVID);
	pr_info("%s(): DEVID: 0x%02X\n", __func__, revid);
	pr_info("%s(): dev_name(dev) '%s'\n", __func__, dev_name(dev));

	if (revid == 0xE5) {
		pr_info("%s(): ADXL345 is found\n", __func__);
	} else {
		pr_err("%s(): failed to probe %s\n", __func__, input_dev->name);
		err = -ENODEV;
		goto err_out;
	}

	snprintf(ac->phys, sizeof(ac->phys), "%s/input0", dev_name(dev));
	pr_info("%s(): ac->phys '%s'\n", __func__, ac->phys);

	// initialize the input device
	input_dev->phys = ac->phys;
	input_dev->dev.parent = dev;
	input_dev->id.product = ac->model;
	input_dev->id.bustype = bops->bustype;

	// attach the input device and the private structure
	input_set_drvdata(input_dev, ac);

	// set the different event types. EV_KEY type events, with
	// BTN_TOUCH events code when the single tap interrupt is
	// triggered
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(pdata->ev_code_tap[ADXL_X_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Y_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Z_AXIS], input_dev->keybit);

	// check if any of the axis has been enabled and set the
	// interrupt mask In this driver only SINGLE_TAP interrupt
	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
		ac->int_mask |= SINGLE_TAP;

	ac->gpio = devm_gpiod_get_index(dev, ADXL345_GPIO_NAME, 0, GPIOD_IN);
	if (IS_ERR(ac->gpio)) {
		pr_err("%s(): gpio get index failed\n", __func__);
		err = PTR_ERR(ac->gpio); // PTR_ERR return an int from a pointer
		goto err_out;
	}

	ac->irq = gpiod_to_irq(ac->gpio);
	if (ac->irq < 0) {
		pr_err("%s(): gpio get irq failed\n", __func__);
		err = ac->irq;
		goto err_out;
	}
	pr_info("%s(): the IRQ number is: %d\n", __func__, ac->irq);

	// request threaded interrupt
	err = devm_request_threaded_irq(input_dev->dev.parent,
					ac->irq,
					NULL,
					adxl345_irq,
					IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
					dev_name(dev),
					ac);
	if (err)
		goto err_out;

	err = sysfs_create_group(&dev->kobj, &adxl345_attr_group);
	if (err)
		goto err_out;

	// register the input device to the input core
	err = input_register_device(input_dev);
	if (err)
		goto err_remove_attr;


	/* Initialize the ADXL345 registers */


	// set the tap threshold and duration
	AC_WRITE(ac, ADXL345_REG_THRESH_TAP, pdata->tap_threshold);
	AC_WRITE(ac, ADXL345_REG_DUR, pdata->tap_duration);

	// set the axis where the tap will be detected
	AC_WRITE(ac, ADXL345_REG_TAP_AXES, pdata->tap_axis_control);

	// set the data rate and the axis reading power mode, less or
	// higher noise reducing power
	AC_WRITE(ac, ADXL345_REG_BW_RATE, RATE(ac->pdata.data_rate) |
		 (pdata->low_power_mode ? LOW_POWER : 0));

	// 13-bit full resolution right justified
	AC_WRITE(ac, ADXL345_REG_DATA_FORMAT, pdata->data_range);

	// set the FIFO mode, no FIFO by default
	AC_WRITE(ac, ADXL345_REG_FIFO_CTL, FIFO_MODE(pdata->fifo_mode) |
		 SAMPLES(pdata->watermark));

	// map all INTs to INT1 pin
	AC_WRITE(ac, ADXL345_REG_INT_MAP, 0);

	// enables interrupts
	AC_WRITE(ac, ADXL345_REG_INT_ENABLE, ac->int_mask);

	// set RUN mode
	AC_WRITE(ac, ADXL345_REG_POWER_CTL, PCTL_MEASURE);

	return ac;

err_remove_attr:
	sysfs_remove_group(&dev->kobj, &adxl345_attr_group);

err_out:
// returns a pointer to a struct ac or an err pointer
	return ERR_PTR(err);
}

/*
  spi read

  Write the address of the register and read the value of it.
*/
static int adxl345_spi_read(struct device *dev, unsigned char reg)
{
	struct spi_device *spi = to_spi_device(dev);
	u8 cmd;

	pr_info("%s(): called\n", __func__);
	cmd = ADXL345_READ(reg);

	return spi_w8r8(spi, cmd);
}

/*
  spi write

  Write 2 bytes, the address of the register and the value.
*/
static int
adxl345_spi_write(struct device *dev, unsigned char reg, unsigned char val)
{
	struct spi_device *spi = to_spi_device(dev);
	u8 buf[2];

	pr_info("%s(): called\n", __func__);
	buf[0] = ADXL345_WRITE(reg);
	buf[1] = val;

	return spi_write(spi, buf, sizeof(buf));
}

/* spi read block

   Read multiple registers
*/
static int adxl345_spi_read_block(struct device *dev,
				  unsigned char reg, int count,
				  void *buf)
{
	struct spi_device *spi = to_spi_device(dev);
	ssize_t status;

	pr_info("%s(): called\n", __func__);

	// add MB flags to the reading
	reg = ADXL345_READMB(reg);

	// write byte stored in reg (address with MB) read count bytes
	// (from successive addresses) and stores them to buf
	status = spi_write_then_read(spi, &reg, 1, buf, count);

	return (status < 0) ? status : 0;
}

static const struct adxl345_bus_ops adxl345_spi_bops = {
	.bustype	= BUS_SPI,
	.write		= adxl345_spi_write,
	.read		= adxl345_spi_read,
	.read_block	= adxl345_spi_read_block,
};

static int adxl345_spi_probe(struct spi_device *spi)
{
	struct adxl345 *ac;

	pr_info("%s(): called\n", __func__);

	// send the spi operations
	ac = adxl345_probe(&spi->dev, &adxl345_spi_bops);
	if (IS_ERR(ac))
		return PTR_ERR(ac);

	// attach the SPI device to the private structure
	spi_set_drvdata(spi, ac);

	return 0;
}

static void adxl345_spi_remove(struct spi_device *spi)
{
	struct adxl345 *ac = spi_get_drvdata(spi);

	pr_info("%s(): called\n", __func__);

	sysfs_remove_group(&ac->dev->kobj, &adxl345_attr_group);
	input_unregister_device(ac->input);
	AC_WRITE(ac, ADXL345_REG_POWER_CTL, PCTL_STANDBY);
}

static const struct of_device_id adxl345_dt_ids[] = {
	{ .compatible = "lothars,adxl345", },
	{ }
};
MODULE_DEVICE_TABLE(of, adxl345_dt_ids);

static const struct spi_device_id adxl345_id[] = {
	{ .name = "adxl345", },
	{ }
};
MODULE_DEVICE_TABLE(spi, adxl345_id);

static struct spi_driver adxl345_driver = {
	.driver = {
		.name = "adxl345",
		.of_match_table = adxl345_dt_ids,
	},
	.probe = adxl345_spi_probe,
	.remove = adxl345_spi_remove,
	.id_table = adxl345_id,
};
module_spi_driver(adxl345_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Input layer demo using spi");
