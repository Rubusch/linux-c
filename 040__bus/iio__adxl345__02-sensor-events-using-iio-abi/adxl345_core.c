// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADXL345 3-Axis Digital Accelerometer IIO core driver
 *
 * Copyright (c) 2017 Eva Rachel Retuya <eraretuya@gmail.com>
 *
 * Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf
 */

#include <linux/bitfield.h>
#include <linux/bitops.h>
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

#include "adxl345.h"

#define ADXL345_REG_DEVID		0x00
#define ADXL345_REG_THRESH_TAP		0x1D
#define ADXL345_REG_OFSX		0x1E
#define ADXL345_REG_OFSY		0x1F
#define ADXL345_REG_OFSZ		0x20
#define ADXL345_REG_OFS_AXIS(index)	(ADXL345_REG_OFSX + (index))

/* Tap duration */
#define ADXL345_REG_DUR			0x21
/* Tap latency */
#define ADXL345_REG_LATENT		0x22
/* Tap window */
#define ADXL345_REG_WINDOW		0x23
/* Activity threshold */
#define ADXL345_REG_THRESH_ACT		0x24
/* Inactivity threshold */
#define ADXL345_REG_THRESH_INACT	0x25
/* Inactivity time */
#define ADXL345_REG_TIME_INACT		0x26
/* Axis enable control for activity and inactivity detection */
#define ADXL345_REG_ACT_INACT_CTRL	0x27
/* Free-fall threshold */
#define ADXL345_REG_THRESH_FF		0x28
/* Free-fall time */
#define ADXL345_REG_TIME_FF		0x29
/* Axis control for single tap or double tap */
#define ADXL345_REG_TAP_AXIS		0x2A
/* Source of single tap or double tap */
#define ADXL345_REG_ACT_TAP_STATUS	0x2B
/* Data rate and power mode control */
#define ADXL345_REG_BW_RATE		0x2C
#define ADXL345_REG_POWER_CTL		0x2D
#define ADXL345_REG_INT_ENABLE		0x2E
#define ADXL345_REG_INT_MAP		0x2F
#define ADXL345_REG_INT_SOURCE		0x30
#define ADXL345_REG_INT_SOURCE_MSK	0xFF
#define ADXL345_REG_XYZ_BASE		0x32
#define ADXL345_REG_DATA_AXIS(index)				\
	(ADXL345_REG_XYZ_BASE + (index) * sizeof(__le16))

#define ADXL345_REG_FIFO_CTL		0x38
#define ADXL345_FIFO_CTL_SAMPLES_MSK	GENMASK(4, 0)
/* 0: INT1, 1: INT2 */
#define ADXL345_FIFO_CTL_TRIGGER_MSK	BIT(5)
#define ADXL345_FIFO_CTL_MODE_MSK	GENMASK(7, 6)
#define ADXL345_REG_FIFO_STATUS	0x39
#define ADXL345_REG_FIFO_STATUS_MSK	0x3F

#define ADXL345_INT_OVERRUN		BIT(0)
#define ADXL345_INT_WATERMARK		BIT(1)
#define ADXL345_INT_FREE_FALL		BIT(2)
#define ADXL345_INT_INACTIVITY		BIT(3)
#define ADXL345_INT_ACTIVITY		BIT(4)
#define ADXL345_INT_DOUBLE_TAP		BIT(5)
#define ADXL345_INT_SINGLE_TAP		BIT(6)
#define ADXL345_INT_DATA_READY		BIT(7)

/*
 * BW_RATE bits - Bandwidth and output data rate. The default value is
 * 0x0A, which translates to a 100 Hz output data rate
 */
#define ADXL345_BW_RATE_MSK		GENMASK(3, 0)
#define ADXL345_BW_LOW_POWER		BIT(4)
#define ADXL345_BASE_RATE_NANO_HZ	97656250LL

#define ADXL345_POWER_CTL_STANDBY	0x00
#define ADXL345_POWER_CTL_WAKEUP	GENMASK(1, 0)
#define ADXL345_POWER_CTL_SLEEP		BIT(2)
#define ADXL345_POWER_CTL_MEASURE	BIT(3)
#define ADXL345_POWER_CTL_AUTO_SLEEP	BIT(4)
#define ADXL345_POWER_CTL_LINK		BIT(5)
#define ADXL345_POWER_CTL_INACT_MSK	ADXL345_POWER_CTL_AUTO_SLEEP | ADXL345_POWER_CTL_LINK

/* Set the g range */
#define ADXL345_DATA_FORMAT_RANGE_MSK	GENMASK(1, 0)
/* Data is left justified */
#define ADXL345_DATA_FORMAT_JUSTIFY	BIT(2)
/* Up to 13-bits resolution */
#define ADXL345_DATA_FORMAT_FULL_RES	BIT(3)
#define ADXL345_DATA_FORMAT_SELF_TEST	BIT(7)
#define ADXL345_DATA_FORMAT_2G		0
#define ADXL345_DATA_FORMAT_4G		1
#define ADXL345_DATA_FORMAT_8G		2
#define ADXL345_DATA_FORMAT_16G		3

#define ADXL345_DEVID			0xE5
#define ADXL345_FIFO_SIZE		32

#define ADXL345_FIFO_BYPASS	0
#define ADXL345_FIFO_FIFO	1
#define ADXL345_FIFO_STREAM	2

#define ADXL345_DIRS 3

#define ADXL345_INT_NONE		0xff
#define ADXL345_INT1			0
#define ADXL345_INT2			1

#define ADXL345_REG_TAP_AXIS_MSK	GENMASK(2, 0)
#define ADXL345_REG_TAP_SUPPRESS_MSK	BIT(3)
#define ADXL345_REG_ACT_AXIS_MSK	GENMASK(6, 4)
#define ADXL345_REG_ACT_ACDC_MSK	BIT(7)
#define ADXL345_REG_INACT_AXIS_MSK	GENMASK(2, 0)
#define ADXL345_REG_INACT_ACDC_MSK	BIT(3)

enum adxl345_axis {
	ADXL345_Z_EN = BIT(0),
	ADXL345_Y_EN = BIT(1),
	ADXL345_X_EN = BIT(2),
	/* Suppress double tap detection if value > tap threshold */
	ADXL345_TAP_SUPPRESS = BIT(3),
};

/* single/double tap */
enum adxl345_tap_type {
	ADXL345_SINGLE_TAP,
	ADXL345_DOUBLE_TAP,
};

static const unsigned int adxl345_tap_int_reg[2] = {
	[ADXL345_SINGLE_TAP] = ADXL345_INT_SINGLE_TAP,
	[ADXL345_DOUBLE_TAP] = ADXL345_INT_DOUBLE_TAP,
};

enum adxl345_tap_time_type {
	ADXL345_TAP_TIME_LATENT,
	ADXL345_TAP_TIME_WINDOW,
	ADXL345_TAP_TIME_DUR,
};

static const unsigned int adxl345_tap_time_reg[3] = {
	[ADXL345_TAP_TIME_LATENT] = ADXL345_REG_LATENT,
	[ADXL345_TAP_TIME_WINDOW] = ADXL345_REG_WINDOW,
	[ADXL345_TAP_TIME_DUR] = ADXL345_REG_DUR,
};

/* activity/inactivity */
enum adxl345_activity_type {
	ADXL345_ACTIVITY,
	ADXL345_INACTIVITY,
};

static const unsigned int adxl345_act_int_reg[2] = {
	[ADXL345_ACTIVITY] = ADXL345_INT_ACTIVITY,
	[ADXL345_INACTIVITY] = ADXL345_INT_INACTIVITY,
};

static const unsigned int adxl345_act_thresh_reg[2] = {
	[ADXL345_ACTIVITY] = ADXL345_REG_THRESH_ACT,
	[ADXL345_INACTIVITY] = ADXL345_REG_THRESH_INACT,
};

static const unsigned int adxl345_act_acdc_msk[2] = {
	[ADXL345_ACTIVITY] = ADXL345_REG_ACT_ACDC_MSK,
	[ADXL345_INACTIVITY] = ADXL345_REG_INACT_ACDC_MSK,
};

static const unsigned int adxl345_act_axis_msk[2] = {
	[ADXL345_ACTIVITY] = ADXL345_REG_ACT_AXIS_MSK,
	[ADXL345_INACTIVITY] = ADXL345_REG_INACT_AXIS_MSK,
};

enum adxl345_odr {
	ADXL345_ODR_0P10HZ = 0,
	ADXL345_ODR_0P20HZ,
	ADXL345_ODR_0P39HZ,
	ADXL345_ODR_0P78HZ,
	ADXL345_ODR_1P56HZ,
	ADXL345_ODR_3P13HZ,
	ADXL345_ODR_6P25HZ,
	ADXL345_ODR_12P50HZ,
	ADXL345_ODR_25HZ,
	ADXL345_ODR_50HZ,
	ADXL345_ODR_100HZ,
	ADXL345_ODR_200HZ,
	ADXL345_ODR_400HZ,
	ADXL345_ODR_800HZ,
	ADXL345_ODR_1600HZ,
	ADXL345_ODR_3200HZ,
};

enum adxl345_range {
	ADXL345_2G_RANGE = 0,
	ADXL345_4G_RANGE,
	ADXL345_8G_RANGE,
	ADXL345_16G_RANGE,
};

/* Certain features recommend 12.5 Hz - 400 Hz ODR */
static const int adxl345_odr_tbl[][2] = {
	[ADXL345_ODR_0P10HZ]	= {   0,  97000},
	[ADXL345_ODR_0P20HZ]	= {   0, 195000},
	[ADXL345_ODR_0P39HZ]	= {   0, 390000},
	[ADXL345_ODR_0P78HZ]	= {   0, 781000},
	[ADXL345_ODR_1P56HZ]	= {   1, 562000},
	[ADXL345_ODR_3P13HZ]	= {   3, 125000},
	[ADXL345_ODR_6P25HZ]	= {   6, 250000},
	[ADXL345_ODR_12P50HZ]	= {  12, 500000},
	[ADXL345_ODR_25HZ]	= {  25, 0},
	[ADXL345_ODR_50HZ]	= {  50, 0},
	[ADXL345_ODR_100HZ]	= { 100, 0},
	[ADXL345_ODR_200HZ]	= { 200, 0},
	[ADXL345_ODR_400HZ]	= { 400, 0},
	[ADXL345_ODR_800HZ]	= { 800, 0},
	[ADXL345_ODR_1600HZ]	= {1600, 0},
	[ADXL345_ODR_3200HZ]	= {3200, 0},
};

/*
 * Full resolution frequency table:
 * (g * 2 * 9.80665) / (2^(resolution) - 1) * 100
 *
 * resolution := 13 (full)
 * g := 2|4|8|16
 */ 
static const int adxl345_fullres_range_tbl[][2] = {
	[ADXL345_2G_RANGE]  = {0, 478899},
	[ADXL345_4G_RANGE]  = {0, 957798},
	[ADXL345_8G_RANGE]  = {1, 915595},
	[ADXL345_16G_RANGE] = {3, 831190},
};

/* scaling */
static const int adxl345_range_factor_tbl[] = {
	[ADXL345_2G_RANGE]  = 1,
	[ADXL345_4G_RANGE]  = 2,
	[ADXL345_8G_RANGE]  = 4,
	[ADXL345_16G_RANGE] = 8,
};

struct adxl345_state {
	const struct adxl345_chip_info *info;
	struct regmap *regmap;
	bool fifo_delay; /* delay: delay is needed for SPI */
	int irq;
	u8 intio;
	u8 int_map;
	u8 watermark;
	u8 fifo_mode;

	enum adxl345_odr odr;
	enum adxl345_range range;

	u32 act_axis_ctrl;
	bool act_ac;
	u8 act_threshold;

	u32 inact_axis_ctrl;
	bool inact_ac;
	u8 inact_threshold;
	u8 inact_time_s;

	u32 tap_axis_ctrl;
	u8 tap_threshold;
	u32 tap_duration_us;
	u32 tap_latent_us;
	u32 tap_window_us;
	bool tap_suppressed;

	u8 ff_threshold;
	u32 ff_time_ms;

	__le16 fifo_buf[ADXL345_DIRS * ADXL345_FIFO_SIZE + 1] __aligned(IIO_DMA_MINALIGN);
};

static struct iio_event_spec adxl345_events[] = {
	{
		/* activity */
		.type = IIO_EV_TYPE_THRESH,
		.dir = IIO_EV_DIR_RISING,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE) |
			BIT(IIO_EV_INFO_VALUE),
	},
	{
		/* inactivity */
		.type = IIO_EV_TYPE_THRESH,
		.dir = IIO_EV_DIR_FALLING,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE) |
			BIT(IIO_EV_INFO_VALUE) |
			BIT(IIO_EV_INFO_PERIOD),
	},
	{
		/* single tap */
		.type = IIO_EV_TYPE_GESTURE,
		.dir = IIO_EV_DIR_SINGLETAP,
		.mask_separate = BIT(IIO_EV_INFO_ENABLE),
		.mask_shared_by_type = BIT(IIO_EV_INFO_VALUE) | 
			BIT(IIO_EV_INFO_TIMEOUT),
	},
	{
		/* double tap */
		.type = IIO_EV_TYPE_GESTURE,
		.dir = IIO_EV_DIR_DOUBLETAP,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE) |
			BIT(IIO_EV_INFO_RESET_TIMEOUT) |
			BIT(IIO_EV_INFO_TAP2_MIN_DELAY),
	},
	{
		/* free fall */
		.type = IIO_EV_TYPE_MAG,
		.dir = IIO_EV_DIR_FALLING,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE) | 
			BIT(IIO_EV_INFO_VALUE) |
			BIT(IIO_EV_INFO_PERIOD),
	},
	{
		/* activity, activity - ac bit */
		.type = IIO_EV_TYPE_MAG_REFERENCED,
		.dir = IIO_EV_DIR_RISING,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE),
	},
	{
		/* activity, inactivity - ac bit */
		.type = IIO_EV_TYPE_MAG_REFERENCED,
		.dir = IIO_EV_DIR_FALLING,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE),
	},
};

#define ADXL345_CHANNEL(index, reg, axis) {					\
	.type = IIO_ACCEL,						\
	.modified = 1,							\
	.channel2 = IIO_MOD_##axis,					\
	.address = (reg),						\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |			\
		BIT(IIO_CHAN_INFO_CALIBBIAS),				\
	.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) |		\
		BIT(IIO_CHAN_INFO_SAMP_FREQ),				\
	.info_mask_shared_by_type_available = BIT(IIO_CHAN_INFO_SCALE) | \
		BIT(IIO_CHAN_INFO_SAMP_FREQ),		\
	.scan_index = (index),				\
	.scan_type = {					\
		.sign = 's',				\
		.realbits = 13,				\
		.storagebits = 16,			\
		.endianness = IIO_LE,			\
	},						\
	.event_spec = adxl345_events,				\
	.num_event_specs = ARRAY_SIZE(adxl345_events),	\
}

enum adxl345_chans {
	chan_x, chan_y, chan_z,
};

static const struct iio_chan_spec adxl345_channels[] = {
	ADXL345_CHANNEL(0, chan_x, X),
	ADXL345_CHANNEL(1, chan_y, Y),
	ADXL345_CHANNEL(2, chan_z, Z),
};

static const unsigned long adxl345_scan_masks[] = {
	BIT(chan_x) | BIT(chan_y) | BIT(chan_z),
	0
};

/**
 * adxl345_set_measure_en() - Enable and disable measuring.
 *
 * @st: The device data.
 * @en: Enable measurements, else standby mode.
 *
 * For lowest power operation, standby mode can be used. In standby mode,
 * current consumption is supposed to be reduced to 0.1uA (typical). In this
 * mode no measurements are made. Placing the device into standby mode
 * preserves the contents of FIFO.
 *
 * Return: Returns 0 if successful, or a negative error value.
 */
static int adxl345_set_measure_en(struct adxl345_state *st, bool en)
{
	unsigned int val = en ? ADXL345_POWER_CTL_MEASURE : ADXL345_POWER_CTL_STANDBY;

	return regmap_write(st->regmap, ADXL345_REG_POWER_CTL, val);
}

static void adxl345_powerdown(void *ptr)
{
	struct adxl345_state *st = ptr;

	adxl345_set_measure_en(st, false);
}

/* act/inact */

static int adxl345_write_act_axis(struct adxl345_state *st,
				  enum adxl345_activity_type type, bool en)
{
	int ret;

	/*
	 * The ADXL345 allows for individually enabling/disabling axis for
	 * activity and inactivity detection, respectively. Here both axis are
	 * kept in sync, i.e. an axis will be generally enabled or disabled for
	 * both equally, activity and inactivity detection.
	 */
	if (type == ADXL345_ACTIVITY) {
		st->act_axis_ctrl = en
			? st->act_axis_ctrl | ADXL345_REG_ACT_AXIS_MSK
			: st->act_axis_ctrl & ~ADXL345_REG_ACT_AXIS_MSK;

		ret = regmap_update_bits(st->regmap, ADXL345_REG_ACT_INACT_CTRL,
					 adxl345_act_axis_msk[type],
					 st->act_axis_ctrl);
		if (ret)
			return ret;

	} else {
		st->inact_axis_ctrl = en
			? st->inact_axis_ctrl | ADXL345_REG_INACT_AXIS_MSK
			: st->inact_axis_ctrl & ~ADXL345_REG_INACT_AXIS_MSK;

		ret = regmap_update_bits(st->regmap, ADXL345_REG_ACT_INACT_CTRL,
					 adxl345_act_axis_msk[type],
					 st->inact_axis_ctrl);
		if (ret)
			return ret;
	}
	return 0;
}

static int adxl345_is_act_inact_ac(struct adxl345_state *st,
				   enum adxl345_activity_type type, bool *ac)
{
	if (type == ADXL345_ACTIVITY)
		*ac = st->act_ac;
	else
		*ac = st->inact_ac;

	return 0;
}

static int adxl345_set_act_inact_ac(struct adxl345_state *st,
				    enum adxl345_activity_type type, bool ac)
{
	int ret;

	ret = regmap_update_bits(st->regmap, ADXL345_REG_ACT_INACT_CTRL,
				 adxl345_act_acdc_msk[type], ac);
	if (ret)
		return ret;

	if (type == ADXL345_ACTIVITY)
		st->act_ac = ac;
	else
		st->inact_ac = ac;
			
	return 0;
}

static int adxl345_is_act_inact_en(struct adxl345_state *st,
				   enum adxl345_activity_type type, bool *en)
{
	int ret;
	unsigned int regval;

	ret = regmap_read(st->regmap, ADXL345_REG_INT_ENABLE, &regval);
	if (ret)
		return ret;

	*en = (adxl345_act_int_reg[type] & regval) > 0;

	return 0;
}

static int adxl345_set_act_inact_en(struct adxl345_state *st,
				    enum adxl345_activity_type type, bool cmd_en)
{
	bool axis_en, en;
	long unsigned int autosleep;
	int ret;

	ret = adxl345_write_act_axis(st, type, cmd_en);
	if (ret)
		return ret;

	if (type == ADXL345_ACTIVITY) {
		axis_en = FIELD_GET(ADXL345_REG_ACT_AXIS_MSK, st->act_axis_ctrl) > 0;
		en = axis_en && st->act_threshold > 0;
	} else {
		axis_en = FIELD_GET(ADXL345_REG_INACT_AXIS_MSK, st->inact_axis_ctrl) > 0;
		en = axis_en && st->inact_threshold > 0 &&
			st->inact_time_s > 0;
	}

	en ? __set_bit(ilog2(adxl345_act_int_reg[type]), (unsigned long*) &st->int_map)
		: __clear_bit(ilog2(adxl345_act_int_reg[type]), (unsigned long*) &st->int_map);

	ret = regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, st->int_map);
	if (ret)
		return ret;

	en ? __set_bit(ilog2(ADXL345_POWER_CTL_INACT_MSK), &autosleep)
		: __clear_bit(ilog2(ADXL345_POWER_CTL_INACT_MSK), &autosleep);

	return regmap_update_bits(st->regmap, ADXL345_REG_POWER_CTL,
			ADXL345_POWER_CTL_INACT_MSK, autosleep);
}

static int adxl345_set_act_inact_threshold(struct adxl345_state *st,
		enum adxl345_activity_type type, u8 val)
{
	int ret;

	ret = regmap_write(st->regmap, adxl345_act_thresh_reg[type], val);
	if (ret)
		return ret;

	if (type == ADXL345_ACTIVITY)
		st->act_threshold = val;
	else
		st->inact_threshold = val;

	return 0;
}

/**
 * adxl345_set_inact_time_s - Configure inactivity time explicitely or by ODR.
 * @st: The sensor state instance.
 * @val_s: A desired time value, between 0 and 255.
 *
 * If val_s is 0, a default inactivity time will be computed. It should take
 * power consumption into consideration. Thus it shall be shorter for higher
 * frequencies and longer for lower frequencies. Hence, frequencies above 255 Hz
 * shall default to 10 s and frequencies below 10 Hz shall result in 255 s to
 * detect inactivity.
 *
 * The approach simply subtracts the pre-decimal figure of the configured
 * sample frequency from 255 s to compute inactivity time [s]. Sub-Hz are thus
 * ignored in this estimation. The recommended ODRs for various features
 * (activity/inactivity, sleep modes, free fall, etc.) lie between 12.5 Hz and
 * 400 Hz, thus higher or lower frequencies will result in the boundary
 * defaults or need to be explicitely specified via val_s.
 *
 * Return: 0 or error value.
 */
static int adxl345_set_inact_time_s(struct adxl345_state *st, u32 val_s)
{
	unsigned int max_boundary = 255;
	unsigned int min_boundary = 10;
	unsigned int val = min(val_s, max_boundary);
	int ret;

	if (0 == val)
		val = (adxl345_odr_tbl[st->odr][0] > max_boundary)
			? min_boundary : max_boundary -	adxl345_odr_tbl[st->odr][0];
	
	ret = regmap_write(st->regmap, ADXL345_REG_TIME_INACT, val);
	if (ret)
		return ret;

	st->inact_time_s = val;

	return 0;
}

/* tap */

static int adxl345_write_tap_axis(struct adxl345_state *st,
		enum adxl345_axis axis, bool en)
{
	st->tap_axis_ctrl = FIELD_GET(ADXL345_REG_TAP_AXIS_MSK,
			en ? st->tap_axis_ctrl | axis
			: st->tap_axis_ctrl & ~axis);

	return regmap_update_bits(st->regmap, ADXL345_REG_TAP_AXIS,
			ADXL345_REG_TAP_AXIS_MSK,
			FIELD_PREP(ADXL345_REG_TAP_AXIS_MSK,
				st->tap_axis_ctrl));
}

static int _adxl345_set_tap_int(struct adxl345_state *st,
		enum adxl345_tap_type type, bool state)
{
	bool axis_valid;
	bool singletap_args_valid = false;
	bool doubletap_args_valid = false;
	bool en = false;

	axis_valid = FIELD_GET(ADXL345_REG_TAP_AXIS_MSK, st->tap_axis_ctrl) > 0;

	/*
	 * Note: A value of 0 for threshold and/or dur may result in undesirable
	 *	 behavior if single tap/double tap interrupts are enabled.
	 */
	singletap_args_valid = st->tap_threshold > 0 && st->tap_duration_us > 0;

	if (type == ADXL345_SINGLE_TAP) {
		en = axis_valid && singletap_args_valid;
	} else {
		/* doubletap: Window must be equal or greater than latent! */
		doubletap_args_valid = st->tap_latent_us > 0 &&
			st->tap_window_us > 0 &&
			st->tap_window_us >= st->tap_latent_us;

		en = axis_valid && singletap_args_valid && doubletap_args_valid;
	}

	if (state && en) {
		__set_bit(ilog2(adxl345_tap_int_reg[type]),
			  (unsigned long*) &st->int_map);
	} else {
		__clear_bit(ilog2(adxl345_tap_int_reg[type]),
			    (unsigned long*) &st->int_map);
	}

	return regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, st->int_map);
}

static int adxl345_is_tap_en(struct adxl345_state *st,
			     enum adxl345_tap_type type, bool *en)
{
	int ret;
	unsigned int regval;

	ret = regmap_read(st->regmap, ADXL345_REG_INT_ENABLE, &regval);
	if (ret)
		return ret;

	*en = (adxl345_tap_int_reg[type] & regval) > 0;

	return 0;
}

static int adxl345_set_singletap_en(struct adxl345_state *st,
				    enum adxl345_axis axis, bool en)
{
	int ret;

	ret = adxl345_write_tap_axis(st, axis, en);
	if (ret)
		return ret;

	return _adxl345_set_tap_int(st, ADXL345_SINGLE_TAP, en);
}

static int adxl345_set_doubletap_en(struct adxl345_state *st, bool en)
{
	return _adxl345_set_tap_int(st, ADXL345_DOUBLE_TAP, en);
}

static int adxl345_is_suppressed_en(struct adxl345_state *st, bool *en)
{
	*en = st->tap_suppressed;

	return 0;
}

static int adxl345_set_suppressed_en(struct adxl345_state *st, bool en)
{
	long unsigned int regval;
	int ret;

	en ? __set_bit(ilog2(ADXL345_TAP_SUPPRESS), &regval)
		: __clear_bit(ilog2(ADXL345_TAP_SUPPRESS), &regval);

	ret = regmap_update_bits(st->regmap, ADXL345_REG_TAP_AXIS,
				  ADXL345_REG_TAP_SUPPRESS_MSK, regval);
	if (ret)
		return ret;

	st->tap_suppressed = en;

	return 0;
}

static int adxl345_set_tap_threshold(struct adxl345_state *st, u8 val)
{
	int ret;

	ret = regmap_write(st->regmap, ADXL345_REG_THRESH_TAP, min(val, 0xFF));
	if (ret)
		return ret;

	st->tap_threshold = val;

	return 0;
}

static int _adxl345_set_tap_time(struct adxl345_state *st,
				 enum adxl345_tap_time_type type, u32 val_us)
{
	unsigned int regval;

	switch (type) {
	case ADXL345_TAP_TIME_WINDOW:
		st->tap_window_us = val_us;
		break;
	case ADXL345_TAP_TIME_LATENT:
		st->tap_latent_us = val_us;
		break;
	case ADXL345_TAP_TIME_DUR:
		st->tap_duration_us = val_us;
		break;
	}

	/*
	 * The scale factor is 1250us / LSB for tap_window_us and tap_latent_us.
	 * For tap_duration_us the scale factor is 625us / LSB.
	 */
	if (type == ADXL345_TAP_TIME_DUR)
		regval = DIV_ROUND_CLOSEST(val_us, 625);
	else
		regval = DIV_ROUND_CLOSEST(val_us, 1250);

	return regmap_write(st->regmap, adxl345_tap_time_reg[type], regval);
}

static int adxl345_set_tap_duration(struct adxl345_state *st, u32 val_int,
				    u32 val_fract_us)
{
	/*
	 * Max value is 255 * 625 us = 0.159375 seconds
	 *
	 * Note: the scaling is similar to the scaling in the ADXL380
	 */
	if (val_int || val_fract_us > 159375)
		return -EINVAL;

	return _adxl345_set_tap_time(st, ADXL345_TAP_TIME_DUR, val_fract_us);
}

static int adxl345_set_tap_window(struct adxl345_state *st, u32 val_int,
				  u32 val_fract_us)
{
	/*
	 * Max value is 255 * 1250 us = 0.318750 seconds
	 *
	 * Note: the scaling is similar to the scaling in the ADXL380
	 */
	if (val_int || val_fract_us > 318750)
		return -EINVAL;

	return _adxl345_set_tap_time(st, ADXL345_TAP_TIME_WINDOW, val_fract_us);
}

static int adxl345_set_tap_latent(struct adxl345_state *st, u32 val_int,
				  u32 val_fract_us)
{
	/*
	 * Max value is 255 * 1250 us = 0.318750 seconds
	 *
	 * Note: the scaling is similar to the scaling in the ADXL380
	 */
	if (val_int || val_fract_us > 318750)
		return -EINVAL;

	return _adxl345_set_tap_time(st, ADXL345_TAP_TIME_LATENT, val_fract_us);
}

/* ff */

static int adxl345_is_ff_en(struct adxl345_state *st, bool *en)
{
	int ret;
	unsigned int regval;

	ret = regmap_read(st->regmap, ADXL345_REG_INT_ENABLE, &regval);
	if (ret)
		return ret;

	*en = FIELD_GET(ADXL345_INT_FREE_FALL, st->int_map) > 0;

	return 0;
}

static int adxl345_set_ff_en(struct adxl345_state *st, bool cmd_en)
{
	bool en = cmd_en && st->ff_threshold > 0 && st->ff_time_ms > 0;

	en ? __set_bit(ilog2(ADXL345_INT_FREE_FALL), (unsigned long*) &st->int_map)
		: __clear_bit(ilog2(ADXL345_INT_FREE_FALL), (unsigned long*) &st->int_map);

	return regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, st->int_map);
}

static int adxl345_set_ff_threshold(struct adxl345_state *st, u8 val)
{
	int ret;

	ret = regmap_write(st->regmap, ADXL345_REG_THRESH_FF, val);
	if (ret)
		return ret;

	st->ff_threshold = val;

	return 0;
}

static int adxl345_set_ff_time(struct adxl345_state *st, u32 val_int,
			       u32 val_fract_us)
{
	unsigned int regval;
	int val_ms;

	/*
	 * max value is 255 * 5000 us = 1.275000 seconds
	 *
	 * Note: the scaling is similar to the scaling in the ADXL380
	 */
	if (1000000 * val_int + val_fract_us > 1275000)
		return -EINVAL;

	val_ms = val_int * 1000 + DIV_ROUND_UP(val_fract_us, 1000);
	st->ff_time_ms = val_ms;

	regval = DIV_ROUND_CLOSEST(val_ms, 5);

	/* Values between 100ms and 350ms (0x14 to 0x46) are recommended. */
	return regmap_write(st->regmap, ADXL345_REG_TIME_FF, min(regval, 0xff));
}

static int adxl345_find_odr(struct adxl345_state *st, int val,
			    int val2, enum adxl345_odr *odr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(adxl345_odr_tbl); i++)
		if (val == adxl345_odr_tbl[i][0] &&
		    val2 == adxl345_odr_tbl[i][1])
			break;

	if (i == ARRAY_SIZE(adxl345_odr_tbl))
		return -EINVAL;

	*odr = i;

	return 0;
}

static int adxl345_set_odr(struct adxl345_state *st, enum adxl345_odr odr)
{
	int ret;

	ret = regmap_update_bits(st->regmap, ADXL345_REG_BW_RATE,
				 ADXL345_BW_RATE_MSK,
				 FIELD_PREP(ADXL345_BW_RATE_MSK, odr));
	if (ret)
		return ret;

	st->odr = odr;

	/* update inactivity time by ODR */
	ret = adxl345_set_inact_time_s(st, 0);
	if (ret)
		return ret;

	return 0;
}

static int adxl345_find_range(struct adxl345_state *st, int val, int val2,
			      enum adxl345_range *range)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(adxl345_fullres_range_tbl); i++)
		if (val == adxl345_fullres_range_tbl[i][0] &&
		    val2 == adxl345_fullres_range_tbl[i][1])
			break;

	if (i == ARRAY_SIZE(adxl345_fullres_range_tbl))
		return -EINVAL;

	*range = i;

	return 0;
}

static int adxl345_set_range(struct adxl345_state *st, enum adxl345_range range)
{
	int ret;

	ret = regmap_update_bits(st->regmap, ADXL345_REG_DATA_FORMAT,
				 ADXL345_DATA_FORMAT_RANGE_MSK,
				 FIELD_PREP(ADXL345_DATA_FORMAT_RANGE_MSK, range));
	if (ret)
		return ret;

	st->act_threshold = st->act_threshold
		* adxl345_range_factor_tbl[st->range]
		/ adxl345_range_factor_tbl[range];
	st->act_threshold = min(255, max(1, st->inact_threshold));

	st->inact_threshold = st->inact_threshold
		* adxl345_range_factor_tbl[st->range]
		/ adxl345_range_factor_tbl[range];
	st->inact_threshold = min(255, max(1, st->inact_threshold));

	ret = adxl345_set_act_inact_threshold(st, ADXL345_ACTIVITY, st->act_threshold);
	if (ret)
		return ret;

	ret = adxl345_set_act_inact_threshold(st, ADXL345_INACTIVITY, st->inact_threshold);
	if (ret)
		return ret;

	st->range = range;

	return 0;
}

static int adxl345_read_avail(struct iio_dev *indio_dev,
			      struct iio_chan_spec const *chan,
			      const int **vals, int *type,
			      int *length, long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_SCALE:
		*vals = (int *)adxl345_fullres_range_tbl;
		*type = IIO_VAL_INT_PLUS_MICRO;
		*length = ARRAY_SIZE(adxl345_fullres_range_tbl) * 2;
		return IIO_AVAIL_LIST;
	case IIO_CHAN_INFO_SAMP_FREQ:
		*vals = (int *)adxl345_odr_tbl;
		*type = IIO_VAL_INT_PLUS_MICRO;
		*length = ARRAY_SIZE(adxl345_odr_tbl) * 2;
		return IIO_AVAIL_LIST;
	}

	return -EINVAL;
}

static int adxl345_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	__le16 accel;
	unsigned int regval;
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		/*
		 * Data is stored in adjacent registers:
		 * ADXL345_REG_DATA(X0/Y0/Z0) contain the least significant byte
		 * and ADXL345_REG_DATA(X0/Y0/Z0) + 1 the most significant byte
		 */
		ret = regmap_bulk_read(st->regmap,
				       ADXL345_REG_DATA_AXIS(chan->address),
				       &accel, sizeof(accel));
		if (ret < 0)
			return ret;

		*val = sign_extend32(le16_to_cpu(accel), 12);
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		*val = adxl345_fullres_range_tbl[st->range][0];
		*val2 = adxl345_fullres_range_tbl[st->range][1];
		return IIO_VAL_INT_PLUS_MICRO;
	case IIO_CHAN_INFO_CALIBBIAS:
		ret = regmap_read(st->regmap,
				  ADXL345_REG_OFS_AXIS(chan->address), &regval);
		if (ret)
			return ret;
		/*
		 * 8-bit resolution at +/- 2g, that is 4x accel data scale
		 * factor
		 */
		*val = sign_extend32(regval, 7) * 4;
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SAMP_FREQ:
		*val = adxl345_odr_tbl[st->odr][0];
		*val2 = adxl345_odr_tbl[st->odr][1];
		return IIO_VAL_INT_PLUS_MICRO;
	}

	return -EINVAL;
}

static int adxl345_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val, int val2, long mask)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	enum adxl345_range range;
	enum adxl345_odr odr;
	int ret;

	ret = adxl345_set_measure_en(st, false);
	if (ret)
 		return ret;

	switch (mask) {
	case IIO_CHAN_INFO_CALIBBIAS:
		/*
		 * 8-bit resolution at +/- 2g, that is 4x accel data scale
		 * factor.
		 */
		ret = regmap_write(st->regmap,
				   ADXL345_REG_OFS_AXIS(chan->address), val / 4);
		break;
	case IIO_CHAN_INFO_SAMP_FREQ:
		ret = adxl345_find_odr(st, val, val2, &odr);
		if (ret)
			return ret;

		ret = adxl345_set_odr(st, odr);
		break;
	case IIO_CHAN_INFO_SCALE:
		ret = adxl345_find_range(st, val, val2,	&range);
		if (ret)
			return ret;
		ret = adxl345_set_range(st, range);
		break;
	default:
		return -EINVAL;
	}

	if (ret)
		return ret;

	return adxl345_set_measure_en(st, true);
}

static int adxl345_read_event_config(struct iio_dev *indio_dev,
				     const struct iio_chan_spec *chan,
				     enum iio_event_type type,
				     enum iio_event_direction dir)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	bool int_en;
	bool axis_en;
	int ret = -EFAULT;

	switch (type) {
	case IIO_EV_TYPE_THRESH:
		switch (dir) {
		case IIO_EV_DIR_RISING:
			ret = adxl345_is_act_inact_en(st, ADXL345_ACTIVITY, &int_en);
			if (ret)
				return ret;
			return int_en;
		case IIO_EV_DIR_FALLING:
			ret = adxl345_is_act_inact_en(st, ADXL345_INACTIVITY, &int_en);
			if (ret)
				return ret;
			return int_en;
		default:
			return -EINVAL;
		}
	case IIO_EV_TYPE_GESTURE:
		switch (chan->channel2) {
		case IIO_MOD_X:
			axis_en = FIELD_GET(ADXL345_X_EN, st->tap_axis_ctrl);
			break;
		case IIO_MOD_Y:
			axis_en = FIELD_GET(ADXL345_Y_EN, st->tap_axis_ctrl);
			break;
		case IIO_MOD_Z:
			axis_en = FIELD_GET(ADXL345_Z_EN, st->tap_axis_ctrl);
			break;
		default:
			axis_en = ADXL345_TAP_SUPPRESS;
		}

		switch (dir) {
		case IIO_EV_DIR_SINGLETAP:
			ret = adxl345_is_tap_en(st, ADXL345_SINGLE_TAP, &int_en);
			if (ret)
				return ret;
			return int_en && axis_en;
		case IIO_EV_DIR_DOUBLETAP:
			ret = adxl345_is_tap_en(st, ADXL345_DOUBLE_TAP, &int_en);
			if (ret)
				return ret;
			return int_en;
		default:
			return -EINVAL;
		}
	case IIO_EV_TYPE_MAG:
		ret = adxl345_is_ff_en(st, &int_en);
		if (ret)
			return ret;
		return int_en;
	case IIO_EV_TYPE_MAG_REFERENCED:
		switch (dir) {
		case IIO_EV_DIR_RISING:
			ret = adxl345_is_act_inact_ac(st, ADXL345_ACTIVITY, &int_en);
			if (ret)
				return ret;
			return int_en;
		case IIO_EV_DIR_FALLING:
			ret = adxl345_is_act_inact_ac(st, ADXL345_INACTIVITY, &int_en);
			if (ret)
				return ret;
			return int_en;
		default:
			return -EINVAL;
		}
	default:
		return -EINVAL;
	}

	return ret;
}

static int adxl345_write_event_config(struct iio_dev *indio_dev,
				      const struct iio_chan_spec *chan,
				      enum iio_event_type type,
				      enum iio_event_direction dir,
				      int state)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	enum adxl345_axis axis;

	switch (type) {
	case IIO_EV_TYPE_THRESH:
		switch (dir) {
		case IIO_EV_DIR_RISING:
			return adxl345_set_act_inact_en(st, ADXL345_ACTIVITY, state);
		case IIO_EV_DIR_FALLING:
			return adxl345_set_act_inact_en(st, ADXL345_INACTIVITY, state);
		default:
			return -EINVAL;
		}
	case IIO_EV_TYPE_GESTURE:
		switch (chan->channel2) {
		case IIO_MOD_X:
			axis = ADXL345_X_EN;
			break;
		case IIO_MOD_Y:
			axis = ADXL345_Y_EN;
			break;
		case IIO_MOD_Z:
			axis = ADXL345_Z_EN;
			break;
		default:
			axis = ADXL345_TAP_SUPPRESS;
		}

		switch (dir) {
		case IIO_EV_DIR_SINGLETAP:
			return adxl345_set_singletap_en(st, axis, state);
		case IIO_EV_DIR_DOUBLETAP:
			return adxl345_set_doubletap_en(st, state);
		default:
			return -EINVAL;
		}
	case IIO_EV_TYPE_MAG:
		return adxl345_set_ff_en(st, state);
	case IIO_EV_TYPE_MAG_REFERENCED:
		switch (dir) {
		case IIO_EV_DIR_RISING:
			return adxl345_set_act_inact_ac(st, ADXL345_ACTIVITY, state);
		case IIO_EV_DIR_FALLING:
			return adxl345_set_act_inact_ac(st, ADXL345_INACTIVITY, state);
		default:
			return -EINVAL;
		}

	default:
		return -EINVAL;
	}

	return -EINVAL;
}

static int adxl345_read_event_value(struct iio_dev *indio_dev, const struct iio_chan_spec *chan,
				    enum iio_event_type type, enum iio_event_direction dir,
				    enum iio_event_info info, int *val, int *val2)
{
	struct adxl345_state *st = iio_priv(indio_dev);

	switch (type) {
	case IIO_EV_TYPE_THRESH:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			switch (dir) {
			case IIO_EV_DIR_RISING:
				*val = st->act_threshold;
				return IIO_VAL_INT;
			case IIO_EV_DIR_FALLING:
				*val = st->inact_threshold;
				return IIO_VAL_INT;
			default:
				return -EINVAL;
			}
		case IIO_EV_INFO_PERIOD:
			*val = st->inact_time_s;
			return IIO_VAL_INT;
		default:
			return -EINVAL;
		}		
	case IIO_EV_TYPE_GESTURE:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			/*
			 * The scale factor is 62.5mg/LSB (i.e. 0xFF = 16g) but
			 * not applied here.
			 */
			*val = sign_extend32(st->tap_threshold, 7);
			return IIO_VAL_INT;
		case IIO_EV_INFO_TIMEOUT:
			*val = st->tap_duration_us;
			*val2 = 1000000;
			return IIO_VAL_FRACTIONAL;
		case IIO_EV_INFO_RESET_TIMEOUT:
			*val = st->tap_window_us;
			*val2 = 1000000;
			return IIO_VAL_FRACTIONAL;
		case IIO_EV_INFO_TAP2_MIN_DELAY:
			*val = st->tap_latent_us;
			*val2 = 1000000;
			return IIO_VAL_FRACTIONAL;
		default:
			return -EINVAL;
		}
	case IIO_EV_TYPE_MAG:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			*val = st->ff_threshold;
			return IIO_VAL_INT;
		case IIO_EV_INFO_PERIOD:
			*val = st->ff_time_ms;
			*val2 = 1000;
			return IIO_VAL_FRACTIONAL;
		default:
			return -EINVAL;
		}
	default:
		return -EINVAL;
	}
	return -EINVAL;
}

static int adxl345_write_event_value(struct iio_dev *indio_dev,
				     const struct iio_chan_spec *chan,
				     enum iio_event_type type,
				     enum iio_event_direction dir,
				     enum iio_event_info info,
				     int val, int val2)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	int ret;

	ret = adxl345_set_measure_en(st, false);
	if (ret)
		return ret;

	switch (type) {
	case IIO_EV_TYPE_THRESH:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			switch (dir) {
			case IIO_EV_DIR_RISING:
				ret = adxl345_set_act_inact_threshold(st, ADXL345_ACTIVITY, val);
				break;
			case IIO_EV_DIR_FALLING:
				ret = adxl345_set_act_inact_threshold(st, ADXL345_INACTIVITY, val);
				break;
			default:
				ret = -EINVAL;
			}
			break;
		case IIO_EV_INFO_PERIOD:
			ret = adxl345_set_inact_time_s(st, val);
			break;
		default:
			ret = -EINVAL;
		}
		break;
	case IIO_EV_TYPE_GESTURE:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			ret = adxl345_set_tap_threshold(st, val);
			break;
		case IIO_EV_INFO_TIMEOUT:
			ret = adxl345_set_tap_duration(st, val, val2);
			break;
		case IIO_EV_INFO_RESET_TIMEOUT:
			ret = adxl345_set_tap_window(st, val, val2);
			break;
		case IIO_EV_INFO_TAP2_MIN_DELAY:
			ret = adxl345_set_tap_latent(st, val, val2);
			break;
		default:
			ret = -EINVAL;
		}
		break;
	case IIO_EV_TYPE_MAG:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			ret = adxl345_set_ff_threshold(st, val);
			break;
		case IIO_EV_INFO_PERIOD:
			ret = adxl345_set_ff_time(st, val, val2);
			break;
		default:
			ret = -EINVAL;
		}
		break;
	default:
		ret = -EINVAL;
	}

	if (ret)
		return ret; /* measurement stays off */

	return adxl345_set_measure_en(st, true);
}

static int adxl345_reg_access(struct iio_dev *indio_dev, unsigned int reg,
			      unsigned int writeval, unsigned int *readval)
{
	struct adxl345_state *st = iio_priv(indio_dev);

	if (readval)
		return regmap_read(st->regmap, reg, readval);
	return regmap_write(st->regmap, reg, writeval);
}

static int adxl345_set_watermark(struct iio_dev *indio_dev, unsigned int value)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	unsigned int fifo_mask = 0x1F;
	int ret;

	value = min(value, ADXL345_FIFO_SIZE - 1);

	ret = regmap_update_bits(st->regmap, ADXL345_REG_FIFO_CTL, fifo_mask, value);
	if (ret)
		return ret;

	st->watermark = value;
	st->int_map |= ADXL345_INT_WATERMARK;

	return 0;
}

static int adxl345_write_raw_get_fmt(struct iio_dev *indio_dev,
				     struct iio_chan_spec const *chan,
				     long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_CALIBBIAS:
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		return IIO_VAL_INT_PLUS_MICRO;
	case IIO_CHAN_INFO_SAMP_FREQ:
		return IIO_VAL_INT_PLUS_MICRO;
	default:
		return -EINVAL;
	}
}

static ssize_t in_accel_gesture_doubletap_suppressed_en_show(struct device *dev,
						     struct device_attribute *attr,
						     char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct adxl345_state *st = iio_priv(indio_dev);
	bool en;
	int val, ret;

	ret = adxl345_is_suppressed_en(st, &en);
	if (ret)
		return ret;
	val = en ? 1 : 0;

	return iio_format_value(buf, IIO_VAL_INT, 1, &val);
}

static ssize_t in_accel_gesture_doubletap_suppressed_en_store(struct device *dev,
						      struct device_attribute *attr,
						      const char *buf, size_t len)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct adxl345_state *st = iio_priv(indio_dev);
	int val, ret;

	ret = kstrtoint(buf, 0, &val);
	if (ret)
		return ret;

	ret = adxl345_set_measure_en(st, false);
	if (ret)
		return ret;

	ret = adxl345_set_suppressed_en(st, val > 0);
	if (ret)
		return ret;

	ret =  adxl345_set_measure_en(st, true);
	if (ret)
		return ret;

	return len;
}
static IIO_DEVICE_ATTR_RW(in_accel_gesture_doubletap_suppressed_en, 0);

static struct attribute *adxl345_event_attrs[] = {
	&iio_dev_attr_in_accel_gesture_doubletap_suppressed_en.dev_attr.attr,
	NULL
};

static const struct attribute_group adxl345_event_attrs_group = {
	.attrs = adxl345_event_attrs,
};

static int adxl345_set_fifo(struct adxl345_state *st)
{
	int ret;

	/* FIFO should only be configured while in standby mode */
	ret = adxl345_set_measure_en(st, false);
	if (ret < 0)
		return ret;

	ret = regmap_write(st->regmap, ADXL345_REG_FIFO_CTL,
			   FIELD_PREP(ADXL345_FIFO_CTL_SAMPLES_MSK,
				      st->watermark) |
			   FIELD_PREP(ADXL345_FIFO_CTL_TRIGGER_MSK,
				      st->intio) |
			   FIELD_PREP(ADXL345_FIFO_CTL_MODE_MSK,
				      st->fifo_mode));
	if (ret < 0)
		return ret;

	return adxl345_set_measure_en(st, true);
}

/**
 * adxl345_get_samples() - Read number of FIFO entries.
 * @st: The initialized state instance of this driver.
 *
 * The sensor does not support treating any axis individually, or exclude them
 * from measuring.
 *
 * Return: negative error, or value.
 */
static int adxl345_get_samples(struct adxl345_state *st)
{
	unsigned int regval = 0;
	int ret;

	ret = regmap_read(st->regmap, ADXL345_REG_FIFO_STATUS, &regval);
	if (ret < 0)
		return ret;

	return FIELD_GET(ADXL345_REG_FIFO_STATUS_MSK, regval);
}

/**
 * adxl345_fifo_transfer() - Read samples number of elements.
 * @st: The instance of the state object of this sensor.
 * @samples: The number of lines in the FIFO referred to as fifo_entry.
 *
 * It is recommended that a multiple-byte read of all registers be performed to
 * prevent a change in data between reads of sequential registers. That is to
 * read out the data registers X0, X1, Y0, Y1, Z0, Z1, i.e. 6 bytes at once.
 *
 * Return: 0 or error value.
 */
static int adxl345_fifo_transfer(struct adxl345_state *st, int samples)
{
	size_t count;
	int i, ret = 0;

	/* count is the 3x the fifo_buf element size, hence 6B */
	count = sizeof(st->fifo_buf[0]) * ADXL345_DIRS;
	for (i = 0; i < samples; i++) {
		/* read 3x 2 byte elements from base address into next fifo_buf position */
		ret = regmap_bulk_read(st->regmap, ADXL345_REG_XYZ_BASE,
				       st->fifo_buf + (i * count / 2), count);
		if (ret < 0)
			return ret;

		/*
		 * To ensure that the FIFO has completely popped, there must be at least 5
		 * us between the end of reading the data registers, signified by the
		 * transition to register 0x38 from 0x37 or the CS pin going high, and the
		 * start of new reads of the FIFO or reading the FIFO_STATUS register. For
		 * SPI operation at 1.5 MHz or lower, the register addressing portion of the
		 * transmission is sufficient delay to ensure the FIFO has completely
		 * popped. It is necessary for SPI operation greater than 1.5 MHz to
		 * de-assert the CS pin to ensure a total of 5 us, which is at most 3.4 us
		 * at 5 MHz operation.
		 */
		if (st->fifo_delay && samples > 1)
			udelay(3);
	}
	return ret;
}

/**
 * adxl345_fifo_reset() - Empty the FIFO in error condition.
 * @st: The instance to the state object of the sensor.
 *
 * Read all elements of the FIFO. Reading the interrupt source register
 * resets the sensor.
 */
static void adxl345_fifo_reset(struct adxl345_state *st)
{
	int regval;
	int samples;

	adxl345_set_measure_en(st, false);

	samples = adxl345_get_samples(st);
	if (samples > 0)
		adxl345_fifo_transfer(st, samples);

	regmap_read(st->regmap, ADXL345_REG_INT_SOURCE, &regval);

	adxl345_set_measure_en(st, true);
}

static int adxl345_buffer_postenable(struct iio_dev *indio_dev)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	int ret;

	ret = regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, st->int_map);
	if (ret < 0)
		return ret;

	st->fifo_mode = ADXL345_FIFO_STREAM;
	return adxl345_set_fifo(st);
}

static int adxl345_buffer_predisable(struct iio_dev *indio_dev)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	int ret;

	st->fifo_mode = ADXL345_FIFO_BYPASS;
	ret = adxl345_set_fifo(st);
	if (ret < 0)
		return ret;

	st->int_map = 0x00;
	return regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, st->int_map);
}

static const struct iio_buffer_setup_ops adxl345_buffer_ops = {
	.postenable = adxl345_buffer_postenable,
	.predisable = adxl345_buffer_predisable,
};

static int adxl345_get_status(struct adxl345_state *st, unsigned int *int_stat,
			      enum iio_modifier *act_tap_dir)
{
	unsigned int regval;
	bool check_tap_stat;
	bool check_act_stat;

	*act_tap_dir = IIO_NO_MOD;
	check_tap_stat = FIELD_GET(ADXL345_REG_TAP_AXIS_MSK, st->tap_axis_ctrl) > 0;
	check_act_stat = FIELD_GET(ADXL345_REG_ACT_AXIS_MSK, st->act_axis_ctrl) > 0;

	if (check_tap_stat || check_act_stat) {
		/* ACT_TAP_STATUS should be read before clearing the interrupt */
		if (regmap_read(st->regmap, ADXL345_REG_ACT_TAP_STATUS, &regval))
			return -EINVAL;

		if ((FIELD_GET(ADXL345_Z_EN, regval >> 4)
				| FIELD_GET(ADXL345_Z_EN, regval)) > 0)
			*act_tap_dir = IIO_MOD_Z;
		else if ((FIELD_GET(ADXL345_Y_EN, regval >> 4)
				| FIELD_GET(ADXL345_Y_EN, regval)) > 0)
			*act_tap_dir = IIO_MOD_Y;
		else if ((FIELD_GET(ADXL345_X_EN, regval >> 4)
				| FIELD_GET(ADXL345_X_EN, regval)) > 0)
			*act_tap_dir = IIO_MOD_X;
	}

	return regmap_read(st->regmap, ADXL345_REG_INT_SOURCE, int_stat);
}

static int adxl345_fifo_push(struct iio_dev *indio_dev,
			     int samples)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	int i, ret;

	if (samples <= 0)
		return -EINVAL;

	ret = adxl345_fifo_transfer(st, samples);
	if (ret)
		return ret;

	for (i = 0; i < ADXL345_DIRS * samples; i += ADXL345_DIRS)
		iio_push_to_buffers(indio_dev, &st->fifo_buf[i]);

	return 0;
}

static int adxl345_push_event(struct iio_dev *indio_dev, int int_stat,
			      enum iio_modifier act_tap_dir)
{
	s64 ts = iio_get_time_ns(indio_dev);
	int ret;

	if (FIELD_GET(ADXL345_INT_SINGLE_TAP, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							act_tap_dir,
							IIO_EV_TYPE_GESTURE,
							IIO_EV_DIR_SINGLETAP),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_DOUBLE_TAP, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							act_tap_dir,
							IIO_EV_TYPE_GESTURE,
							IIO_EV_DIR_DOUBLETAP),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_ACTIVITY, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							act_tap_dir,
							IIO_EV_TYPE_THRESH,
							IIO_EV_DIR_RISING),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_INACTIVITY, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							IIO_MOD_X_OR_Y_OR_Z,
							IIO_EV_TYPE_THRESH,
							IIO_EV_DIR_FALLING),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_FREE_FALL, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							IIO_MOD_X_OR_Y_OR_Z,
							IIO_EV_TYPE_MAG,
							IIO_EV_DIR_FALLING),
				     ts);
		if (ret)
			return ret;
	}

	return -ENOENT;
}

/**
 * adxl345_irq_handler() - Handle irqs of the ADXL345.
 * @irq: The irq being handled.
 * @p: The struct iio_device pointer for the device.
 *
 * Return: The interrupt was handled.
 */
static irqreturn_t adxl345_irq_handler(int irq, void *p)
{
	struct iio_dev *indio_dev = p;
	struct adxl345_state *st = iio_priv(indio_dev);
	int int_stat;
	enum iio_modifier act_tap_dir;
	int samples;

	if (adxl345_get_status(st, &int_stat, &act_tap_dir))
		return IRQ_NONE;

	if (adxl345_push_event(indio_dev, int_stat, act_tap_dir) == 0)
		return IRQ_HANDLED;

	if (FIELD_GET(ADXL345_INT_WATERMARK, int_stat)) {
		samples = adxl345_get_samples(st);
		if (samples < 0)
			goto err;

		if (adxl345_fifo_push(indio_dev, samples) < 0)
			goto err;
	}

	if (FIELD_GET(ADXL345_INT_OVERRUN, int_stat))
		goto err;

	return IRQ_HANDLED;

err:
	adxl345_fifo_reset(st);

	return IRQ_HANDLED;
}

static const struct iio_info adxl345_info = {
	.event_attrs	= &adxl345_event_attrs_group,
	.read_raw	= adxl345_read_raw,
	.write_raw	= adxl345_write_raw,
	.read_avail	= adxl345_read_avail,
	.write_raw_get_fmt	= adxl345_write_raw_get_fmt,
	.read_event_config = adxl345_read_event_config,
	.write_event_config = adxl345_write_event_config,
	.read_event_value = adxl345_read_event_value,
	.write_event_value = adxl345_write_event_value,
	.debugfs_reg_access = &adxl345_reg_access,
	.hwfifo_set_watermark = adxl345_set_watermark,
};

/**
 * adxl345_core_probe() - Probe and setup for the accelerometer.
 * @dev:	Driver model representation of the device
 * @regmap:	Regmap instance for the device
 * @fifo_delay_default: Using FIFO with SPI needs delay
 * @setup:	Setup routine to be executed right before the standard device
 *		setup
 *
 * For SPI operation greater than 1.6 MHz, it is necessary to deassert the CS
 * pin to ensure a total delay of 5 us; otherwise, the delay is not sufficient.
 * The total delay necessary for 5 MHz operation is at most 3.4 us. This is not
 * a concern when using I2C mode because the communication rate is low enough
 * to ensure a sufficient delay between FIFO reads.
 * Ref: "Retrieving Data from FIFO", p. 21 of 36, Data Sheet ADXL345 Rev. G
 *
 * Return: 0 on success, negative errno on error
 */
int adxl345_core_probe(struct device *dev, struct regmap *regmap,
		       bool fifo_delay_default,
		       int (*setup)(struct device*, struct regmap*))
{
	struct adxl345_state *st;
	struct iio_dev *indio_dev;
	u32 regval;
	unsigned int data_format_mask = (ADXL345_DATA_FORMAT_RANGE_MSK |
					 ADXL345_DATA_FORMAT_JUSTIFY |
					 ADXL345_DATA_FORMAT_FULL_RES |
					 ADXL345_DATA_FORMAT_SELF_TEST);
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);
	st->regmap = regmap;
	st->info = device_get_match_data(dev);
	if (!st->info)
		return -ENODEV;
	st->fifo_delay = fifo_delay_default;

	/*
	 * If the feature is enabled, scan all axis for activity and or
	 * inactivity, and set activity and inactivity to the same ac / dc
	 * setup.
	 */
	st->act_axis_ctrl = ADXL345_REG_ACT_AXIS_MSK;
	st->inact_axis_ctrl = ADXL345_REG_INACT_AXIS_MSK;
	st->inact_ac = 0;			/*    0 [dc]              */
	st->act_ac = 0;
	st->int_map = 0x00;			/* reset interrupts */

	/* Init with reasonable values */
	st->tap_threshold = 48;			/*   48 [0x30] -> ~3g     */
	st->tap_duration_us = 16;		/*   16 [0x10] -> .010    */
	st->tap_window_us = 64;			/*   64 [0x40] -> .080    */
	st->tap_latent_us = 16;			/*   16 [0x10] -> .020    */

	st->ff_threshold = 8;			/*    8 [0x08]            */
	st->ff_time_ms = 32;			/*   32 [0x20] -> 0.16    */

	indio_dev->name = st->info->name;
	indio_dev->info = &adxl345_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = adxl345_channels;
	indio_dev->num_channels = ARRAY_SIZE(adxl345_channels);
	indio_dev->available_scan_masks = adxl345_scan_masks;

	/*
	 * Using I2C at 100kHz would limit the maximum ODR to 200Hz, operation
	 * at an output rate above the recommended maximum may result in
	 * undesired behavior.
	 */
	ret = adxl345_set_odr(st, ADXL345_ODR_200HZ);
	if (ret)
		return ret;
	ret = adxl345_set_range(st, ADXL345_16G_RANGE);
	if (ret)
		return ret;

	/* Reset interrupts at start up */
	ret = regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, st->int_map);
	if (ret)
		return ret;

	if (setup) {
		/* Perform optional initial bus specific configuration */
		ret = setup(dev, st->regmap);
		if (ret)
			return ret;

		/* Enable full-resolution mode */
		ret = regmap_update_bits(st->regmap, ADXL345_REG_DATA_FORMAT,
					 data_format_mask,
					 ADXL345_DATA_FORMAT_FULL_RES);
		if (ret)
			return dev_err_probe(dev, ret,
					     "Failed to set data range\n");

	} else {
		/* Enable full-resolution mode (init all data_format bits) */
		ret = regmap_write(st->regmap, ADXL345_REG_DATA_FORMAT,
				   ADXL345_DATA_FORMAT_FULL_RES);
		if (ret)
			return dev_err_probe(dev, ret,
					     "Failed to set data range\n");
	}

	ret = regmap_read(st->regmap, ADXL345_REG_DEVID, &regval);
	if (ret < 0)
		return dev_err_probe(dev, ret, "Error reading device ID\n");

	if (regval != ADXL345_DEVID)
		return dev_err_probe(dev, -ENODEV, "Invalid device ID: %x, expected %x\n",
				     regval, ADXL345_DEVID);

	/* Enable measurement mode */
	ret = adxl345_set_measure_en(st, true);
	if (ret < 0)
		return dev_err_probe(dev, ret, "Failed to enable measurement mode\n");

	ret = devm_add_action_or_reset(dev, adxl345_powerdown, st);
	if (ret < 0)
		return ret;

	st->intio = ADXL345_INT1;
	st->irq = fwnode_irq_get_byname(dev_fwnode(dev), "INT1");
	if (st->irq < 0) {
		st->intio = ADXL345_INT2;
		st->irq = fwnode_irq_get_byname(dev_fwnode(dev), "INT2");
		if (st->irq < 0)
			st->intio = ADXL345_INT_NONE;
	}

	if (st->intio != ADXL345_INT_NONE) {
		/*
		 * Any bits set to 0 in the INT map register send their respective
		 * interrupts to the INT1 pin, whereas bits set to 1 send their respective
		 * interrupts to the INT2 pin. The intio shall convert this accordingly.
		 */
		regval = st->intio ? ADXL345_REG_INT_SOURCE_MSK
			: ~ADXL345_REG_INT_SOURCE_MSK;

		ret = regmap_write(st->regmap, ADXL345_REG_INT_MAP, regval);
		if (ret)
			return ret;

		/* FIFO_STREAM mode is going to be activated later */
		ret = devm_iio_kfifo_buffer_setup(dev, indio_dev, &adxl345_buffer_ops);
		if (ret)
			return ret;

		ret = devm_request_threaded_irq(dev, st->irq, NULL,
						&adxl345_irq_handler,
						IRQF_SHARED | IRQF_ONESHOT,
						indio_dev->name, indio_dev);
		if (ret)
			return ret;
	} else {
		ret = regmap_write(st->regmap, ADXL345_REG_FIFO_CTL,
				   FIELD_PREP(ADXL345_FIFO_CTL_MODE_MSK,
					      ADXL345_FIFO_BYPASS));
		if (ret < 0)
			return ret;
	}

	return devm_iio_device_register(dev, indio_dev);
}
EXPORT_SYMBOL_NS_GPL(adxl345_core_probe, IIO_ADXL345);

MODULE_AUTHOR("Eva Rachel Retuya <eraretuya@gmail.com>");
MODULE_DESCRIPTION("ADXL345 3-Axis Digital Accelerometer core driver");
MODULE_LICENSE("GPL v2");
