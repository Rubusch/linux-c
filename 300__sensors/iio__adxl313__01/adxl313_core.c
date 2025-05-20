// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADXL313 3-Axis Digital Accelerometer
 *
 * Copyright (c) 2021 Lucas Stankus <lucas.p.stankus@gmail.com>
 *
 * Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL313.pdf
 */

#include <linux/bitfield.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/units.h>

#include <linux/iio/buffer.h>
#include <linux/iio/events.h>
#include <linux/iio/iio.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/sysfs.h>

#include "adxl313.h"

#define ADXL313_INT_NONE			U8_MAX
#define ADXL313_INT1				1
#define ADXL313_INT2				2

#define ADXL313_REG_XYZ_BASE			ADXL313_REG_DATA_AXIS(0)

#define ADXL313_ACT_XYZ_EN			GENMASK(6, 4)
#define ADXL313_INACT_XYZ_EN			GENMASK(2, 0)

/* activity/inactivity */
enum adxl313_activity_type {
	ADXL313_ACTIVITY,
	ADXL313_INACTIVITY,
};

static const unsigned int adxl313_act_int_reg[] = {
	[ADXL313_ACTIVITY] = ADXL313_INT_ACTIVITY,
	[ADXL313_INACTIVITY] = ADXL313_INT_INACTIVITY,
};

static const unsigned int adxl313_act_thresh_reg[] = {
	[ADXL313_ACTIVITY] = ADXL313_REG_THRESH_ACT,
	[ADXL313_INACTIVITY] = ADXL313_REG_THRESH_INACT,
};

static const struct regmap_range adxl312_readable_reg_range[] = {
	regmap_reg_range(ADXL313_REG_DEVID0, ADXL313_REG_DEVID0),
	regmap_reg_range(ADXL313_REG_OFS_AXIS(0), ADXL313_REG_OFS_AXIS(2)),
	regmap_reg_range(ADXL313_REG_THRESH_ACT, ADXL313_REG_ACT_INACT_CTL),
	regmap_reg_range(ADXL313_REG_BW_RATE, ADXL313_REG_FIFO_STATUS),
};

static const struct regmap_range adxl313_readable_reg_range[] = {
	regmap_reg_range(ADXL313_REG_DEVID0, ADXL313_REG_XID),
	regmap_reg_range(ADXL313_REG_SOFT_RESET, ADXL313_REG_SOFT_RESET),
	regmap_reg_range(ADXL313_REG_OFS_AXIS(0), ADXL313_REG_OFS_AXIS(2)),
	regmap_reg_range(ADXL313_REG_THRESH_ACT, ADXL313_REG_ACT_INACT_CTL),
	regmap_reg_range(ADXL313_REG_BW_RATE, ADXL313_REG_FIFO_STATUS),
};

const struct regmap_access_table adxl312_readable_regs_table = {
	.yes_ranges = adxl312_readable_reg_range,
	.n_yes_ranges = ARRAY_SIZE(adxl312_readable_reg_range),
};
EXPORT_SYMBOL_NS_GPL(adxl312_readable_regs_table, IIO_ADXL313);

const struct regmap_access_table adxl313_readable_regs_table = {
	.yes_ranges = adxl313_readable_reg_range,
	.n_yes_ranges = ARRAY_SIZE(adxl313_readable_reg_range),
};
EXPORT_SYMBOL_NS_GPL(adxl313_readable_regs_table, IIO_ADXL313);

const struct regmap_access_table adxl314_readable_regs_table = {
	.yes_ranges = adxl312_readable_reg_range,
	.n_yes_ranges = ARRAY_SIZE(adxl312_readable_reg_range),
};
EXPORT_SYMBOL_NS_GPL(adxl314_readable_regs_table, IIO_ADXL313);

bool adxl313_is_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case ADXL313_REG_DATA_AXIS(0):
	case ADXL313_REG_DATA_AXIS(1):
	case ADXL313_REG_DATA_AXIS(2):
	case ADXL313_REG_DATA_AXIS(3):
	case ADXL313_REG_DATA_AXIS(4):
	case ADXL313_REG_DATA_AXIS(5):
	case ADXL313_REG_FIFO_STATUS:
	case ADXL313_REG_INT_SOURCE:
		return true;
	default:
		return false;
	}
}
EXPORT_SYMBOL_NS_GPL(adxl313_is_volatile_reg, IIO_ADXL313);

static int adxl313_set_measure_en(struct adxl313_data *data, bool en)
{
	unsigned int val = en ? ADXL313_MEASUREMENT_MODE : ADXL313_MEASUREMENT_STANDBY;

	return regmap_update_bits(data->regmap, ADXL313_REG_POWER_CTL,
				  ADXL313_POWER_CTL_MSK, val);
}

static int adxl312_check_id(struct device *dev,
			    struct adxl313_data *data)
{
	unsigned int regval;
	int ret;

	ret = regmap_read(data->regmap, ADXL313_REG_DEVID0, &regval);
	if (ret)
		return ret;

	if (regval != ADXL313_DEVID0_ADXL312_314)
		dev_warn(dev, "Invalid manufacturer ID: %#02x\n", regval);

	return 0;
}

static int adxl313_check_id(struct device *dev,
			    struct adxl313_data *data)
{
	unsigned int regval;
	int ret;

	ret = regmap_read(data->regmap, ADXL313_REG_DEVID0, &regval);
	if (ret)
		return ret;

	if (regval != ADXL313_DEVID0)
		dev_warn(dev, "Invalid manufacturer ID: 0x%02x\n", regval);

	/* Check DEVID1 and PARTID */
	if (regval == ADXL313_DEVID0) {
		ret = regmap_read(data->regmap, ADXL313_REG_DEVID1, &regval);
		if (ret)
			return ret;

		if (regval != ADXL313_DEVID1)
			dev_warn(dev, "Invalid mems ID: 0x%02x\n", regval);

		ret = regmap_read(data->regmap, ADXL313_REG_PARTID, &regval);
		if (ret)
			return ret;

		if (regval != ADXL313_PARTID)
			dev_warn(dev, "Invalid device ID: 0x%02x\n", regval);
	}

	return 0;
}

const struct adxl313_chip_info adxl31x_chip_info[] = {
	[ADXL312] = {
		.name = "adxl312",
		.type = ADXL312,
		.scale_factor = 28425072,
		.variable_range = true,
		.soft_reset = false,
		.check_id = &adxl312_check_id,
	},
	[ADXL313] = {
		.name = "adxl313",
		.type = ADXL313,
		.scale_factor = 9576806,
		.variable_range = true,
		.soft_reset = true,
		.check_id = &adxl313_check_id,
	},
	[ADXL314] = {
		.name = "adxl314",
		.type = ADXL314,
		.scale_factor = 478858719,
		.variable_range = false,
		.soft_reset = false,
		.check_id = &adxl312_check_id,
	},
};
EXPORT_SYMBOL_NS_GPL(adxl31x_chip_info, IIO_ADXL313);

static const struct regmap_range adxl312_writable_reg_range[] = {
	regmap_reg_range(ADXL313_REG_OFS_AXIS(0), ADXL313_REG_OFS_AXIS(2)),
	regmap_reg_range(ADXL313_REG_THRESH_ACT, ADXL313_REG_ACT_INACT_CTL),
	regmap_reg_range(ADXL313_REG_BW_RATE, ADXL313_REG_INT_MAP),
	regmap_reg_range(ADXL313_REG_DATA_FORMAT, ADXL313_REG_DATA_FORMAT),
	regmap_reg_range(ADXL313_REG_FIFO_CTL, ADXL313_REG_FIFO_CTL),
};

static const struct regmap_range adxl313_writable_reg_range[] = {
	regmap_reg_range(ADXL313_REG_SOFT_RESET, ADXL313_REG_SOFT_RESET),
	regmap_reg_range(ADXL313_REG_OFS_AXIS(0), ADXL313_REG_OFS_AXIS(2)),
	regmap_reg_range(ADXL313_REG_THRESH_ACT, ADXL313_REG_ACT_INACT_CTL),
	regmap_reg_range(ADXL313_REG_BW_RATE, ADXL313_REG_INT_MAP),
	regmap_reg_range(ADXL313_REG_DATA_FORMAT, ADXL313_REG_DATA_FORMAT),
	regmap_reg_range(ADXL313_REG_FIFO_CTL, ADXL313_REG_FIFO_CTL),
};

const struct regmap_access_table adxl312_writable_regs_table = {
	.yes_ranges = adxl312_writable_reg_range,
	.n_yes_ranges = ARRAY_SIZE(adxl312_writable_reg_range),
};
EXPORT_SYMBOL_NS_GPL(adxl312_writable_regs_table, IIO_ADXL313);

const struct regmap_access_table adxl313_writable_regs_table = {
	.yes_ranges = adxl313_writable_reg_range,
	.n_yes_ranges = ARRAY_SIZE(adxl313_writable_reg_range),
};
EXPORT_SYMBOL_NS_GPL(adxl313_writable_regs_table, IIO_ADXL313);

const struct regmap_access_table adxl314_writable_regs_table = {
	.yes_ranges = adxl312_writable_reg_range,
	.n_yes_ranges = ARRAY_SIZE(adxl312_writable_reg_range),
};
EXPORT_SYMBOL_NS_GPL(adxl314_writable_regs_table, IIO_ADXL313);

static const int adxl313_odr_freqs[][2] = {
	[0] = { 6, 250000 },
	[1] = { 12, 500000 },
	[2] = { 25, 0 },
	[3] = { 50, 0 },
	[4] = { 100, 0 },
	[5] = { 200, 0 },
	[6] = { 400, 0 },
	[7] = { 800, 0 },
	[8] = { 1600, 0 },
	[9] = { 3200, 0 },
};

#define ADXL313_ACCEL_CHANNEL(index, reg, axis) {			\
	.type = IIO_ACCEL,						\
	.scan_index = (index),						\
	.address = (reg),						\
	.modified = 1,							\
	.channel2 = IIO_MOD_##axis,					\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |			\
			      BIT(IIO_CHAN_INFO_CALIBBIAS),		\
	.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) |		\
				    BIT(IIO_CHAN_INFO_SAMP_FREQ),	\
	.info_mask_shared_by_type_available =				\
		BIT(IIO_CHAN_INFO_SAMP_FREQ),				\
	.scan_type = {							\
		.sign = 's',						\
		.realbits = 13,						\
		.storagebits = 16,					\
		.endianness = IIO_BE,					\
	},								\
}

static const struct iio_event_spec adxl313_fake_chan_events[] = {
	{
		/* activity */
		.type = IIO_EV_TYPE_MAG,
		.dir = IIO_EV_DIR_RISING,
		.mask_separate = BIT(IIO_EV_INFO_ENABLE),
		.mask_shared_by_type = BIT(IIO_EV_INFO_VALUE),
	},
	{
		/* inactivity */
		.type = IIO_EV_TYPE_MAG,
		.dir = IIO_EV_DIR_FALLING,
		.mask_separate = BIT(IIO_EV_INFO_ENABLE),
		.mask_shared_by_type = BIT(IIO_EV_INFO_VALUE) |
			BIT(IIO_EV_INFO_PERIOD),
	},
};

enum adxl313_chans {
	chan_x, chan_y, chan_z,
};

static const struct iio_chan_spec adxl313_channels[] = {
	ADXL313_ACCEL_CHANNEL(0, chan_x, X),
	ADXL313_ACCEL_CHANNEL(1, chan_y, Y),
	ADXL313_ACCEL_CHANNEL(2, chan_z, Z),
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_X_AND_Y_AND_Z,
		.scan_index = -1, /* Fake channel for axis AND'ing */
		.event_spec = adxl313_fake_chan_events,
		.num_event_specs = ARRAY_SIZE(adxl313_fake_chan_events),
	},
};

static const unsigned long adxl313_scan_masks[] = {
	BIT(chan_x) | BIT(chan_y) | BIT(chan_z),
	0
};

static int adxl313_set_odr(struct adxl313_data *data,
			   unsigned int freq1, unsigned int freq2)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(adxl313_odr_freqs); i++) {
		if (adxl313_odr_freqs[i][0] == freq1 &&
		    adxl313_odr_freqs[i][1] == freq2)
			break;
	}

	if (i == ARRAY_SIZE(adxl313_odr_freqs))
		return -EINVAL;

	return regmap_update_bits(data->regmap, ADXL313_REG_BW_RATE,
				  ADXL313_RATE_MSK,
				  FIELD_PREP(ADXL313_RATE_MSK, ADXL313_RATE_BASE + i));
}

static int adxl313_read_axis(struct adxl313_data *data,
			     struct iio_chan_spec const *chan)
{
	int ret;

	mutex_lock(&data->lock);

	ret = regmap_bulk_read(data->regmap,
			       ADXL313_REG_DATA_AXIS(chan->address),
			       &data->transf_buf, sizeof(data->transf_buf));
	if (ret)
		goto unlock_ret;

	ret = le16_to_cpu(data->transf_buf);

unlock_ret:
	mutex_unlock(&data->lock);
	return ret;
}

static int adxl313_read_freq_avail(struct iio_dev *indio_dev,
				   struct iio_chan_spec const *chan,
				   const int **vals, int *type, int *length,
				   long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_SAMP_FREQ:
		*vals = (const int *)adxl313_odr_freqs;
		*length = ARRAY_SIZE(adxl313_odr_freqs) * 2;
		*type = IIO_VAL_INT_PLUS_MICRO;
		return IIO_AVAIL_LIST;
	default:
		return -EINVAL;
	}
}

static int adxl313_set_inact_time_s(struct adxl313_data *data,
				    unsigned int val_s)
{
	unsigned int max_boundary = 255;
	unsigned int val = min(val_s, max_boundary);

	return regmap_write(data->regmap, ADXL313_REG_TIME_INACT, val);
}

static int adxl313_is_act_inact_en(struct adxl313_data *data,
				   enum adxl313_activity_type type)
{
	unsigned int axis_ctrl;
	unsigned int regval;
	int axis_en, int_en, ret;

	ret = regmap_read(data->regmap, ADXL313_REG_ACT_INACT_CTL, &axis_ctrl);
	if (ret)
		return ret;

	/* Check if axis for activity are enabled */
	if (type == ADXL313_ACTIVITY)
		axis_en = FIELD_GET(ADXL313_ACT_XYZ_EN, axis_ctrl);
	else
		axis_en = FIELD_GET(ADXL313_INACT_XYZ_EN, axis_ctrl);

	/* The axis are enabled, now check if specific interrupt is enabled */
	ret = regmap_read(data->regmap, ADXL313_REG_INT_ENABLE, &regval);
	if (ret)
		return ret;

	int_en = adxl313_act_int_reg[type] & regval;

	return axis_en && int_en;
}

static int adxl313_set_act_inact_en(struct adxl313_data *data,
				    enum adxl313_activity_type type,
				    bool cmd_en)
{
	unsigned int axis_ctrl;
	unsigned int threshold;
	unsigned int inact_time_s;
	bool en;
	int ret;

	if (type == ADXL313_ACTIVITY)
		axis_ctrl = ADXL313_ACT_XYZ_EN;
	else
		axis_ctrl = ADXL313_INACT_XYZ_EN;

	ret = regmap_update_bits(data->regmap,
				 ADXL313_REG_ACT_INACT_CTL,
				 axis_ctrl,
				 cmd_en ? 0xff : 0x00);
	if (ret)
		return ret;

	ret = regmap_read(data->regmap, adxl313_act_thresh_reg[type], &threshold);
	if (ret)
		return ret;

	en = cmd_en && threshold;
	if (type == ADXL313_INACTIVITY) {
		ret = regmap_read(data->regmap, ADXL313_REG_TIME_INACT, &inact_time_s);
		if (ret)
			return ret;

		en = en && inact_time_s;
	}

	return regmap_update_bits(data->regmap, ADXL313_REG_INT_ENABLE,
				  adxl313_act_int_reg[type],
				  en ? adxl313_act_int_reg[type] : 0);
}

static int adxl313_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
	struct adxl313_data *data = iio_priv(indio_dev);
	unsigned int regval;
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = adxl313_read_axis(data, chan);
		if (ret < 0)
			return ret;

		*val = sign_extend32(ret, chan->scan_type.realbits - 1);
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		*val = 0;

		*val2 = data->chip_info->scale_factor;

		return IIO_VAL_INT_PLUS_NANO;
	case IIO_CHAN_INFO_CALIBBIAS:
		ret = regmap_read(data->regmap,
				  ADXL313_REG_OFS_AXIS(chan->address), &regval);
		if (ret)
			return ret;

		/*
		 * 8-bit resolution at minimum range, that is 4x accel data scale
		 * factor at full resolution
		 */
		*val = sign_extend32(regval, 7) * 4;
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SAMP_FREQ:
		ret = regmap_read(data->regmap, ADXL313_REG_BW_RATE, &regval);
		if (ret)
			return ret;

		ret = FIELD_GET(ADXL313_RATE_MSK, regval) - ADXL313_RATE_BASE;
		*val = adxl313_odr_freqs[ret][0];
		*val2 = adxl313_odr_freqs[ret][1];
		return IIO_VAL_INT_PLUS_MICRO;
	default:
		return -EINVAL;
	}
}

static int adxl313_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val, int val2, long mask)
{
	struct adxl313_data *data = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_CALIBBIAS:
		/*
		 * 8-bit resolution at minimum range, that is 4x accel data scale
		 * factor at full resolution
		 */
		if (clamp_val(val, -128 * 4, 127 * 4) != val)
			return -EINVAL;

		return regmap_write(data->regmap,
				    ADXL313_REG_OFS_AXIS(chan->address),
				    val / 4);
	case IIO_CHAN_INFO_SAMP_FREQ:
		return adxl313_set_odr(data, val, val2);
	default:
		return -EINVAL;
	}
}

static int adxl313_read_event_config(struct iio_dev *indio_dev,
				     const struct iio_chan_spec *chan,
				     enum iio_event_type type,
				     enum iio_event_direction dir)
{
	struct adxl313_data *data = iio_priv(indio_dev);

	if (type != IIO_EV_TYPE_MAG)
		return -EINVAL;

	switch (dir) {
	case IIO_EV_DIR_RISING:
		return adxl313_is_act_inact_en(data, ADXL313_ACTIVITY);
	case  IIO_EV_DIR_FALLING:
		return adxl313_is_act_inact_en(data, ADXL313_INACTIVITY);
	default:
		return -EINVAL;
	}
}

static int adxl313_write_event_config(struct iio_dev *indio_dev,
				      const struct iio_chan_spec *chan,
				      enum iio_event_type type,
				      enum iio_event_direction dir,
				      int state)
{
	struct adxl313_data *data = iio_priv(indio_dev);

	if (type != IIO_EV_TYPE_MAG)
		return -EINVAL;

	switch (dir) {
	case IIO_EV_DIR_RISING:
		return adxl313_set_act_inact_en(data, ADXL313_ACTIVITY, state);
	case IIO_EV_DIR_FALLING:
		return adxl313_set_act_inact_en(data, ADXL313_INACTIVITY, state);
	default:
		return -EINVAL;
	}
}

static int adxl313_read_event_value(struct iio_dev *indio_dev,
				    const struct iio_chan_spec *chan,
				    enum iio_event_type type,
				    enum iio_event_direction dir,
				    enum iio_event_info info,
				    int *val, int *val2)
{
	struct adxl313_data *data = iio_priv(indio_dev);
	unsigned int act_threshold;
	unsigned int inact_threshold;
	unsigned int inact_time_s;
	int ret;

	/* Measurement stays enabled, reading from regmap cache */

	if (type != IIO_EV_TYPE_MAG)
		return -EINVAL;

	switch (info) {
	case IIO_EV_INFO_VALUE:
		switch (dir) {
		case IIO_EV_DIR_RISING:
			ret = regmap_read(data->regmap,
					  adxl313_act_thresh_reg[ADXL313_ACTIVITY],
					  &act_threshold);
			if (ret)
				return ret;
			*val = act_threshold * 15625;
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		case IIO_EV_DIR_FALLING:
			ret = regmap_read(data->regmap,
					  adxl313_act_thresh_reg[ADXL313_INACTIVITY],
					  &inact_threshold);
			if (ret)
				return ret;
			*val = inact_threshold * 15625;
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		default:
			return -EINVAL;
		}
	case IIO_EV_INFO_PERIOD:
		ret = regmap_read(data->regmap,
				  ADXL313_REG_TIME_INACT,
				  &inact_time_s);
		if (ret)
			return ret;
		*val = inact_time_s;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int adxl313_write_event_value(struct iio_dev *indio_dev,
				     const struct iio_chan_spec *chan,
				     enum iio_event_type type,
				     enum iio_event_direction dir,
				     enum iio_event_info info,
				     int val, int val2)
{
	struct adxl313_data *data = iio_priv(indio_dev);
	unsigned int regval;
	int ret;

	ret = adxl313_set_measure_en(data, false);
	if (ret)
		return ret;

	if (type != IIO_EV_TYPE_MAG)
		return -EINVAL;

	switch (info) {
	case IIO_EV_INFO_VALUE:
		/* The scale factor is 15.625 mg/LSB */
		regval = DIV_ROUND_CLOSEST(MICRO * val + val2, 15625);
		switch (dir) {
		case IIO_EV_DIR_RISING:
			ret = regmap_write(data->regmap,
					   adxl313_act_thresh_reg[ADXL313_ACTIVITY],
					   regval);
			if (ret)
				return ret;
			return adxl313_set_measure_en(data, true);
		case IIO_EV_DIR_FALLING:
			ret = regmap_write(data->regmap,
					   adxl313_act_thresh_reg[ADXL313_INACTIVITY],
					   regval);
			if (ret)
				return ret;
			return adxl313_set_measure_en(data, true);
		default:
			return -EINVAL;
		}
	case IIO_EV_INFO_PERIOD:
		ret = adxl313_set_inact_time_s(data, val);
		if (ret)
			return ret;
		return adxl313_set_measure_en(data, true);
	default:
		return -EINVAL;
	}
}

static int adxl313_set_watermark(struct iio_dev *indio_dev, unsigned int value)
{
	struct adxl313_data *data = iio_priv(indio_dev);
	const unsigned int fifo_mask = 0x1f, watermark_mask  = 0x02;
	int ret;

	value = min(value, ADXL313_FIFO_SIZE - 1);

	ret = regmap_update_bits(data->regmap, ADXL313_REG_FIFO_CTL,
				 fifo_mask, value);
	if (ret)
		return ret;

	data->watermark = value;

	return regmap_update_bits(data->regmap, ADXL313_REG_INT_ENABLE,
				  watermark_mask, ADXL313_INT_WATERMARK);
}

static int adxl313_get_samples(struct adxl313_data *data)
{
	unsigned int regval = 0;
	int ret;

	ret = regmap_read(data->regmap, ADXL313_REG_FIFO_STATUS, &regval);
	if (ret)
		return ret;

	return FIELD_GET(ADXL313_REG_FIFO_STATUS_ENTRIES_MSK, regval);
}

static int adxl313_set_fifo(struct adxl313_data *data)
{
	unsigned int int_line;
	int ret;

	ret = adxl313_set_measure_en(data, false);
	if (ret)
		return ret;

	ret = regmap_read(data->regmap, ADXL313_REG_INT_MAP, &int_line);
	if (ret)
		return ret;

	ret = regmap_write(data->regmap, ADXL313_REG_FIFO_CTL,
			   FIELD_PREP(ADXL313_REG_FIFO_CTL_SAMPLES_MSK,	data->watermark) |
			   FIELD_PREP(ADXL313_REG_FIFO_CTL_MODE_MSK, data->fifo_mode));

	return adxl313_set_measure_en(data, true);
}

static int adxl313_fifo_transfer(struct adxl313_data *data, int samples)
{
	size_t count;
	int i;
	int ret;

	count = sizeof(data->fifo_buf[0]) * ADXL313_NUM_AXIS;
	for (i = 0; i < samples; i++) {
		ret = regmap_bulk_read(data->regmap, ADXL313_REG_XYZ_BASE,
				       data->fifo_buf + (i * count / 2), count);
		if (ret)
			return ret;
	}
	return 0;
}

static void adxl313_fifo_reset(struct adxl313_data *data)
{
	int regval;
	int samples;

	adxl313_set_measure_en(data, false);

	/* clear samples */
	samples = adxl313_get_samples(data);
	if (samples)
		adxl313_fifo_transfer(data, samples);

	/* clear interrupt register */
	regmap_read(data->regmap, ADXL313_REG_INT_SOURCE, &regval);

	adxl313_set_measure_en(data, true);
}

static int adxl313_buffer_postenable(struct iio_dev *indio_dev)
{
	struct adxl313_data *data = iio_priv(indio_dev);

	data->fifo_mode = ADXL313_FIFO_STREAM;
	return adxl313_set_fifo(data);
}

static int adxl313_buffer_predisable(struct iio_dev *indio_dev)
{
	struct adxl313_data *data = iio_priv(indio_dev);
	int ret;

	data->fifo_mode = ADXL313_FIFO_BYPASS;
	ret = adxl313_set_fifo(data);
	if (ret)
		return ret;

	return regmap_write(data->regmap, ADXL313_REG_INT_ENABLE, 0);
}

static const struct iio_buffer_setup_ops adxl313_buffer_ops = {
	.postenable = adxl313_buffer_postenable,
	.predisable = adxl313_buffer_predisable,
};

static int adxl313_fifo_push(struct iio_dev *indio_dev, int samples)
{
	struct adxl313_data *data = iio_priv(indio_dev);
	int i, ret;

	if (samples <= 0)
		return -EINVAL;

	ret = adxl313_fifo_transfer(data, samples);
	if (ret)
		return ret;

	for (i = 0; i  < ADXL313_NUM_AXIS * samples; i += ADXL313_NUM_AXIS)
		iio_push_to_buffers(indio_dev, &data->fifo_buf[i]);

	return 0;
}

static int adxl313_push_event(struct iio_dev *indio_dev, int int_stat)
{
	s64 ts = iio_get_time_ns(indio_dev);
	struct adxl313_data *data =  iio_priv(indio_dev);
	int samples;
	int ret = -ENOENT;

	if (FIELD_GET(ADXL313_INT_ACTIVITY, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							IIO_MOD_X_AND_Y_AND_Z,
							IIO_EV_TYPE_MAG,
							IIO_EV_DIR_RISING),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL313_INT_INACTIVITY, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							IIO_MOD_X_AND_Y_AND_Z,
							IIO_EV_TYPE_MAG,
							IIO_EV_DIR_FALLING),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL313_INT_WATERMARK, int_stat)) {
		samples = adxl313_get_samples(data);
		if (samples < 0)
			return -EINVAL;

		ret = adxl313_fifo_push(indio_dev, samples);
	}

	return ret;
}

static irqreturn_t adxl313_irq_handler(int irq, void *p)
{
	struct iio_dev *indio_dev = p;
	struct adxl313_data *data = iio_priv(indio_dev);
	int int_stat;

	if (regmap_read(data->regmap, ADXL313_REG_INT_SOURCE, &int_stat))
		return IRQ_NONE;

	if (adxl313_push_event(indio_dev, int_stat))
		goto err;

	if (FIELD_GET(ADXL313_INT_OVERRUN, int_stat))
		goto err;

	return IRQ_HANDLED;
err:
	adxl313_fifo_reset(data);

	return IRQ_HANDLED;
}

static int adxl313_reg_access(struct iio_dev *indio_dev, unsigned int reg,
			      unsigned int writeval, unsigned int *readval)
{
	struct adxl313_data *data = iio_priv(indio_dev);

	if (readval)
		return regmap_read(data->regmap, reg, readval);
	return regmap_write(data->regmap, reg, writeval);
}

static const struct iio_info adxl313_info = {
	.read_raw	= adxl313_read_raw,
	.write_raw	= adxl313_write_raw,
	.read_event_config = adxl313_read_event_config,
	.write_event_config = adxl313_write_event_config,
	.read_event_value = adxl313_read_event_value,
	.write_event_value = adxl313_write_event_value,
	.read_avail	= adxl313_read_freq_avail,
	.hwfifo_set_watermark = adxl313_set_watermark,
	.debugfs_reg_access = &adxl313_reg_access,
};

static int adxl313_setup(struct device *dev, struct adxl313_data *data,
			 int (*setup)(struct device *, struct regmap *))
{
	int ret;

	/*
	 * If sw reset available, ensures the device is in a consistent
	 * state after start up
	 */
	if (data->chip_info->soft_reset) {
		ret = regmap_write(data->regmap, ADXL313_REG_SOFT_RESET,
				   ADXL313_SOFT_RESET);
		if (ret)
			return ret;
	}

	if (setup) {
		ret = setup(dev, data->regmap);
		if (ret)
			return ret;
	}

	ret = data->chip_info->check_id(dev, data);
	if (ret)
		return ret;

	/* Sets the range to maximum, full resolution, if applicable */
	if (data->chip_info->variable_range) {
		ret = regmap_update_bits(data->regmap, ADXL313_REG_DATA_FORMAT,
					 ADXL313_RANGE_MSK,
					 FIELD_PREP(ADXL313_RANGE_MSK, ADXL313_RANGE_MAX));
		if (ret)
			return ret;

		/* Enables full resolution */
		ret = regmap_update_bits(data->regmap, ADXL313_REG_DATA_FORMAT,
					 ADXL313_FULL_RES, ADXL313_FULL_RES);
		if (ret)
			return ret;
	}

	/* Enables measurement mode */
	return adxl313_set_measure_en(data, true);
}

/**
 * adxl313_core_probe() - probe and setup for adxl313 accelerometer
 * @dev:	Driver model representation of the device
 * @regmap:	Register map of the device
 * @chip_info:	Structure containing device specific data
 * @setup:	Setup routine to be executed right before the standard device
 *		setup, can also be set to NULL if not required
 *
 * Return: 0 on success, negative errno on error cases
 */
int adxl313_core_probe(struct device *dev,
		       struct regmap *regmap,
		       const struct adxl313_chip_info *chip_info,
		       int (*setup)(struct device *, struct regmap *))
{
	struct adxl313_data *data;
	struct iio_dev *indio_dev;
	unsigned int regval;
	u8 int_line;
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	data->regmap = regmap;
	data->chip_info = chip_info;

	mutex_init(&data->lock);

	indio_dev->name = chip_info->name;
	indio_dev->info = &adxl313_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = adxl313_channels;
	indio_dev->num_channels = ARRAY_SIZE(adxl313_channels);
	indio_dev->available_scan_masks = adxl313_scan_masks;

	ret = adxl313_setup(dev, data, setup);
	if (ret) {
		dev_err(dev, "ADXL313 setup failed\n");
		return ret;
	}

	int_line = ADXL313_INT1;
	data->irq = fwnode_irq_get_byname(dev_fwnode(dev), "INT1");
	if (data->irq < 0) {
		int_line = ADXL313_INT2;
		data->irq = fwnode_irq_get_byname(dev_fwnode(dev), "INT2");
		if (data->irq < 0)
			int_line = ADXL313_INT_NONE;
	}

	if (int_line == ADXL313_INT1 || int_line == ADXL313_INT2) {
		/* FIFO_STREAM mode */
		regval = int_line == ADXL313_INT2 ? 0xff : 0;
		ret = regmap_write(data->regmap, ADXL313_REG_INT_MAP, regval);
		if (ret)
			return ret;

		/*
		 * Reset or configure the registers with reasonable default
		 * values. As having 0 in most cases may result in undesirable
		 * behavior if the interrupts are enabled.
		 */
		ret = regmap_write(data->regmap, ADXL313_REG_ACT_INACT_CTL, 0x00);
		if (ret)
			return ret;

		ret = regmap_write(data->regmap, ADXL313_REG_TIME_INACT, 5);
		if (ret)
			return ret;

		ret = regmap_write(data->regmap, ADXL313_REG_THRESH_INACT, 0x4f);
		if (ret)
			return ret;

		ret = regmap_write(data->regmap, ADXL313_REG_THRESH_ACT, 0x52);
		if (ret)
			return ret;

		ret  = devm_iio_kfifo_buffer_setup(dev, indio_dev,
						   &adxl313_buffer_ops);
		if (ret)
			return ret;

		ret = devm_request_threaded_irq(dev, data->irq, NULL,
						&adxl313_irq_handler,
						IRQF_SHARED | IRQF_ONESHOT,
						indio_dev->name, indio_dev);
		if (ret)
			return ret;
	} else {
		/* FIFO_BYPASSED mode */
		ret = regmap_write(data->regmap, ADXL313_REG_FIFO_CTL,
				   FIELD_PREP(ADXL313_REG_FIFO_CTL_MODE_MSK,
					      ADXL313_FIFO_BYPASS));
		if (ret)
			return ret;
	}

	return devm_iio_device_register(dev, indio_dev);
}
EXPORT_SYMBOL_NS_GPL(adxl313_core_probe, IIO_ADXL313);

MODULE_AUTHOR("Lucas Stankus <lucas.p.stankus@gmail.com>");
MODULE_DESCRIPTION("ADXL313 3-Axis Digital Accelerometer core driver");
MODULE_LICENSE("GPL v2");
