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

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/buffer.h>
#include <linux/iio/events.h>
#include <linux/iio/kfifo_buf.h>

#include "adxl345.h"

#define ADXL345_FIFO_BYPASS	0
#define ADXL345_FIFO_FIFO	1
#define ADXL345_FIFO_STREAM	2

#define ADXL345_DIRS 3

#define ADXL345_INT_NONE		0xff
#define ADXL345_INT1			0
#define ADXL345_INT2			1

#define ADXL345_REG_TAP_AXIS_MSK	GENMASK(2, 0)
#define ADXL345_REG_TAP_SUPPRESS_MSK	BIT(3)
#define ADXL345_REG_TAP_SUPPRESS	BIT(3)
#define ADXL345_REG_ACT_AXIS_MSK	GENMASK(6, 4)
#define ADXL345_REG_ACT_ACDC_MSK	BIT(7)
#define ADXL345_REG_INACT_AXIS_MSK	GENMASK(2, 0)
#define ADXL345_REG_INACT_ACDC_MSK	BIT(3)
#define ADXL345_POWER_CTL_INACT_MSK	(ADXL345_POWER_CTL_AUTO_SLEEP | ADXL345_POWER_CTL_LINK)

#define ADXL345_TAP_Z_EN		BIT(0)
#define ADXL345_TAP_Y_EN		BIT(1)
#define ADXL345_TAP_X_EN		BIT(2)

#define ADXL345_INACT_Z_EN		BIT(0)
#define ADXL345_INACT_Y_EN		BIT(1)
#define ADXL345_INACT_X_EN		BIT(2)

#define ADXL345_ACT_Z_EN		BIT(4)
#define ADXL345_ACT_Y_EN		BIT(5)
#define ADXL345_ACT_X_EN		BIT(6)

#define ADXL345_COUPLING_DC		0
#define ADXL345_COUPLING_AC		1

/* single/double tap */
enum adxl345_tap_type {
	ADXL345_SINGLE_TAP,
	ADXL345_DOUBLE_TAP,
};

static const unsigned int adxl345_tap_int_reg[] = {
	[ADXL345_SINGLE_TAP] = ADXL345_INT_SINGLE_TAP,
	[ADXL345_DOUBLE_TAP] = ADXL345_INT_DOUBLE_TAP,
};

enum adxl345_tap_time_type {
	ADXL345_TAP_TIME_LATENT,
	ADXL345_TAP_TIME_WINDOW,
	ADXL345_TAP_TIME_DUR,
};

static const unsigned int adxl345_tap_time_reg[] = {
	[ADXL345_TAP_TIME_LATENT] = ADXL345_REG_LATENT,
	[ADXL345_TAP_TIME_WINDOW] = ADXL345_REG_WINDOW,
	[ADXL345_TAP_TIME_DUR] = ADXL345_REG_DUR,
};

/* activity/inactivity */
enum adxl345_activity_type {
	ADXL345_ACTIVITY,
	ADXL345_INACTIVITY,
	ADXL345_ACTIVITY_AC,
	ADXL345_INACTIVITY_AC,
	ADXL345_INACTIVITY_FF,
};

static const unsigned int adxl345_act_int_reg[] = {
	[ADXL345_ACTIVITY] = ADXL345_INT_ACTIVITY,
	[ADXL345_INACTIVITY] = ADXL345_INT_INACTIVITY,
	[ADXL345_ACTIVITY_AC] = ADXL345_INT_ACTIVITY,
	[ADXL345_INACTIVITY_AC] = ADXL345_INT_INACTIVITY,
	[ADXL345_INACTIVITY_FF] = ADXL345_INT_FREE_FALL,
};

static const unsigned int adxl345_act_thresh_reg[] = {
	[ADXL345_ACTIVITY] = ADXL345_REG_THRESH_ACT,
	[ADXL345_INACTIVITY] = ADXL345_REG_THRESH_INACT,
	[ADXL345_ACTIVITY_AC] = ADXL345_REG_THRESH_ACT,
	[ADXL345_INACTIVITY_AC] = ADXL345_REG_THRESH_INACT,
	[ADXL345_INACTIVITY_FF] = ADXL345_REG_THRESH_FF,
};

static const unsigned int adxl345_act_acdc_msk[] = {
	[ADXL345_ACTIVITY] = ADXL345_REG_ACT_ACDC_MSK,
	[ADXL345_INACTIVITY] = ADXL345_REG_INACT_ACDC_MSK,
	[ADXL345_ACTIVITY_AC] = ADXL345_REG_ACT_ACDC_MSK,
	[ADXL345_INACTIVITY_AC] = ADXL345_REG_INACT_ACDC_MSK,
	[ADXL345_INACTIVITY_FF] = 0x00,
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
	[ADXL345_ODR_0P10HZ]	= {    0,  97000 },
	[ADXL345_ODR_0P20HZ]	= {    0, 195000 },
	[ADXL345_ODR_0P39HZ]	= {    0, 390000 },
	[ADXL345_ODR_0P78HZ]	= {    0, 781000 },
	[ADXL345_ODR_1P56HZ]	= {    1, 562000 },
	[ADXL345_ODR_3P13HZ]	= {    3, 125000 },
	[ADXL345_ODR_6P25HZ]	= {    6, 250000 },
	[ADXL345_ODR_12P50HZ]	= {   12, 500000 },
	[ADXL345_ODR_25HZ]	= {   25, 0 },
	[ADXL345_ODR_50HZ]	= {   50, 0 },
	[ADXL345_ODR_100HZ]	= {  100, 0 },
	[ADXL345_ODR_200HZ]	= {  200, 0 },
	[ADXL345_ODR_400HZ]	= {  400, 0 },
	[ADXL345_ODR_800HZ]	= {  800, 0 },
	[ADXL345_ODR_1600HZ]	= { 1600, 0 },
	[ADXL345_ODR_3200HZ]	= { 3200, 0 },
};

/*
 * Full resolution frequency table:
 * (g * 2 * 9.80665) / (2^(resolution) - 1)
 *
 * resolution := 13 (full)
 * g := 2|4|8|16
 *
 *  2g at 13bit: 0.004789
 *  4g at 13bit: 0.009578
 *  8g at 13bit: 0.019156
 * 16g at 16bit: 0.038312
 */
static const int adxl345_fullres_range_tbl[][2] = {
	[ADXL345_2G_RANGE]  = { 0, 4789 },
	[ADXL345_4G_RANGE]  = { 0, 9578 },
	[ADXL345_8G_RANGE]  = { 0, 19156 },
	[ADXL345_16G_RANGE] = { 0, 38312 },
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
	u8 watermark;
	u8 fifo_mode;

	u8 inact_threshold;
	u32 inact_time_ms;

	u32 tap_duration_us;
	u32 tap_latent_us;
	u32 tap_window_us;

	__le16 fifo_buf[ADXL345_DIRS * ADXL345_FIFO_SIZE + 1] __aligned(IIO_DMA_MINALIGN);
};

static struct iio_event_spec adxl345_events[] = {
	{
		/* activity */
		.type = IIO_EV_TYPE_MAG,
		.dir = IIO_EV_DIR_RISING,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE) |
			BIT(IIO_EV_INFO_VALUE),
	},
	{
		/* activity, ac bit set */
		.type = IIO_EV_TYPE_MAG_ADAPTIVE,
		.dir = IIO_EV_DIR_RISING,
		.mask_shared_by_type = BIT(IIO_EV_INFO_ENABLE) |
			BIT(IIO_EV_INFO_VALUE),
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
	.event_spec = adxl345_events,			\
	.num_event_specs = ARRAY_SIZE(adxl345_events),	\
}

enum adxl345_chans {
	chan_x, chan_y, chan_z,
};

static const struct iio_event_spec adxl345_fake_chan_events[] = {
	{
		/* inactivity */
		.type = IIO_EV_TYPE_MAG,
		.dir = IIO_EV_DIR_FALLING,
		.mask_separate = BIT(IIO_EV_INFO_ENABLE),
		.mask_shared_by_type = BIT(IIO_EV_INFO_VALUE) |
			BIT(IIO_EV_INFO_PERIOD),
	},
	{
		/* inactivity, AC bit set */
		.type = IIO_EV_TYPE_MAG_ADAPTIVE,
		.dir = IIO_EV_DIR_FALLING,
		.mask_separate = BIT(IIO_EV_INFO_ENABLE),
		.mask_shared_by_type = BIT(IIO_EV_INFO_VALUE) |
			BIT(IIO_EV_INFO_PERIOD),
	},
};

static const struct iio_chan_spec adxl345_channels[] = {
	ADXL345_CHANNEL(0, chan_x, X),
	ADXL345_CHANNEL(1, chan_y, Y),
	ADXL345_CHANNEL(2, chan_z, Z),
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_X_AND_Y_AND_Z,
		.scan_index = -1, /* Fake channel */
		.event_spec = adxl345_fake_chan_events,
		.num_event_specs = ARRAY_SIZE(adxl345_fake_chan_events),
	},
};

static const unsigned long adxl345_scan_masks[] = {
	BIT(chan_x) | BIT(chan_y) | BIT(chan_z),
	0
};

bool adxl345_is_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case ADXL345_REG_DATA_AXIS(0):
	case ADXL345_REG_DATA_AXIS(1):
	case ADXL345_REG_DATA_AXIS(2):
	case ADXL345_REG_DATA_AXIS(3):
	case ADXL345_REG_DATA_AXIS(4):
	case ADXL345_REG_DATA_AXIS(5):
	case ADXL345_REG_ACT_TAP_STATUS:
	case ADXL345_REG_FIFO_STATUS:
	case ADXL345_REG_INT_SOURCE:
		return true;
	default:
		return false;
	}
}
EXPORT_SYMBOL_NS_GPL(adxl345_is_volatile_reg, IIO_ADXL345);

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
	return regmap_assign_bits(st->regmap, ADXL345_REG_POWER_CTL,
				  ADXL345_POWER_CTL_MEASURE, en);
}

/* act/inact */

static int adxl345_set_inact_threshold(struct adxl345_state *st,
				       unsigned int threshold)
{
	int ret;

	st->inact_threshold = min(U8_MAX, threshold);

	ret = regmap_write(st->regmap,
			   adxl345_act_thresh_reg[ADXL345_INACTIVITY],
			   st->inact_threshold);
	if (ret)
		return ret;

	return regmap_write(st->regmap,
			    adxl345_act_thresh_reg[ADXL345_INACTIVITY_FF],
			    st->inact_threshold);
}

/**
 * adxl345_set_inact_time - Configure inactivity time explicitly or by ODR.
 * @st: The sensor state instance.
 * @val_int: The inactivity time, integer part.
 * @val_fract: The inactivity time, fractional part when val_int is 0.
 *
 * Inactivity time can be configured between 1 and 255 sec. If a val_s of 0.00
 * is configured by a user, then a default inactivity time will be computed.
 *
 * In such case, it should take power consumption into consideration. Thus it
 * shall be shorter for higher frequencies and longer for lower frequencies.
 * Hence, frequencies above 255 Hz shall default to 10 s and frequencies below
 * 10 Hz shall result in 255 s to detect inactivity.
 *
 * The approach simply subtracts the pre-decimal figure of the configured
 * sample frequency from 255 s to compute inactivity time [s]. Sub-Hz are thus
 * ignored in this estimation. The recommended ODRs for various features
 * (activity/inactivity, sleep modes, free fall, etc.) lie between 12.5 Hz and
 * 400 Hz, thus higher or lower frequencies will result in the boundary
 * defaults or need to be explicitly specified via val_s.
 *
 * Return: 0 or error value.
 */
static int adxl345_set_inact_time(struct adxl345_state *st, u32 val_int,
				  u32 val_fract)
{
	unsigned int max_boundary = 255;
	unsigned int min_boundary = 10;
	unsigned int val;
	enum adxl345_odr odr;
	unsigned int regval;
	int ret;

	if (val_int == 0 && val_fract == 0) {
		/* Generated inactivity time based on ODR */
		ret = regmap_read(st->regmap, ADXL345_REG_BW_RATE, &regval);
		if (ret)
			return ret;

		odr = FIELD_GET(ADXL345_BW_RATE_MSK, regval);

		val = (adxl345_odr_tbl[odr][0] > max_boundary)
			? min_boundary : max_boundary -	adxl345_odr_tbl[odr][0];

		st->inact_time_ms = MILLI * val;

		/* Inactivity time in s */
		ret = regmap_write(st->regmap, ADXL345_REG_TIME_INACT, val);
		if (ret)
			return ret;

	} else if (val_int == 0 && val_fract > 0) {
		/* time < 1s, free-fall */

		/*
		 * Datasheet max. value is 255 * 5000 us = 1.275000 seconds.
		 *
		 * Recommended values between 100ms and 350ms (0x14 to 0x46)
		 */
		st->inact_time_ms = DIV_ROUND_UP(val_fract, MILLI);

		ret = regmap_write(st->regmap, ADXL345_REG_TIME_FF,
				   DIV_ROUND_CLOSEST(val_fract, 5));
		if (ret)
			return ret;
	} else if (val_int > 0) {
		/* Time >= 1s, inactivity */
		st->inact_time_ms = MILLI * val_int;

		ret = regmap_write(st->regmap, ADXL345_REG_TIME_INACT, val_int);
		if (ret)
			return ret;
	} else {
		/* Do not support negative or wrong input. */
		return -EINVAL;
	}

	return 0;
}

/**
 * adxl345_is_act_inact_ac() - Verify if AC or DC coupling is currently enabled.
 *
 * @st: The device data.
 * @type: The activity or inactivity type.
 *
 * Given a type of activity / inactivity combined with either AC coupling set or
 * default to DC, this function verifies if the combination is currently
 * configured, hence enabled or not.
 *
 * Return: true if configured coupling matches the provided type, else a negative
 *         error value.
 */
static int adxl345_is_act_inact_ac(struct adxl345_state *st,
				   enum adxl345_activity_type type)
{
	unsigned int regval;
	bool coupling;
	int ret;

	if (type == ADXL345_INACTIVITY_FF)
		return 1;

	ret = regmap_read(st->regmap, ADXL345_REG_ACT_INACT_CTRL, &regval);
	if (ret)
		return ret;

	coupling = adxl345_act_acdc_msk[type] & regval;

	if (type == ADXL345_ACTIVITY || type == ADXL345_INACTIVITY)
		return coupling == ADXL345_COUPLING_DC;
	else
		return coupling == ADXL345_COUPLING_AC;

	return 0;
}

/**
 * adxl345_set_act_inact_ac() - Configure AC coupling or DC coupling.
 *
 * @st: The device data.
 * @type: Provide a type of activity or inactivity.
 *
 * Enables AC coupling or DC coupling depending on the provided type argument.
 * Note: Activity and inactivity can be either AC coupled or DC coupled not
 * both at the same time.
 *
 * Return: 0 if successful, else error value.
 */
static int adxl345_set_act_inact_ac(struct adxl345_state *st,
				    enum adxl345_activity_type type)
{
	unsigned int coupling;

	if (type == ADXL345_ACTIVITY_AC || type == ADXL345_INACTIVITY_AC)
		coupling = ADXL345_COUPLING_AC;
	else
		coupling = ADXL345_COUPLING_DC;

	/*
	 * A setting of false selects dc-coupled operation, and a setting of
	 * true enables ac-coupled operation. In dc-coupled operation, the
	 * current acceleration magnitude is compared directly with
	 * ADXL345_REG_THRESH_ACT and ADXL345_REG_THRESH_INACT to determine
	 * whether activity or inactivity is detected.
	 *
	 * In ac-coupled operation for activity detection, the acceleration
	 * value at the start of activity detection is taken as a reference
	 * value. New samples of acceleration are then compared to this
	 * reference value, and if the magnitude of the difference exceeds the
	 * ADXL345_REG_THRESH_ACT value, the device triggers an activity
	 * interrupt.
	 *
	 * Similarly, in ac-coupled operation for inactivity detection, a
	 * reference value is used for comparison and is updated whenever the
	 * device exceeds the inactivity threshold. After the reference value
	 * is selected, the device compares the magnitude of the difference
	 * between the reference value and the current acceleration with
	 * ADXL345_REG_THRESH_INACT. If the difference is less than the value in
	 * ADXL345_REG_THRESH_INACT for the time in ADXL345_REG_TIME_INACT, the
	 * device is considered inactive and the inactivity interrupt is
	 * triggered. [quoted from p. 24, ADXL345 datasheet Rev. G]
	 *
	 * In a conclusion, the first acceleration snapshot sample which hit the
	 * threshold in a particular direction is always taken as acceleration
	 * reference value to that direction. Since for the hardware activity
	 * and inactivity depend on the x/y/z axis, so do ac and dc coupling.
	 * Note, this sw driver always enables or disables all three x/y/z axis
	 * for detection via act_axis_ctrl and inact_axis_ctrl, respectively.
	 * Where in dc-coupling samples are compared against the thresholds, in
	 * ac-coupling measurement difference to the first acceleration
	 * reference value are compared against the threshold. So, ac-coupling
	 * allows for a bit more dynamic compensation depending on the initial
	 * sample.
	 */
	return regmap_assign_bits(st->regmap, ADXL345_REG_ACT_INACT_CTRL,
				  adxl345_act_acdc_msk[type], coupling);
}

static int adxl345_is_act_inact_en(struct adxl345_state *st,
				   enum adxl345_activity_type type)
{
	unsigned int regval;
	u32 axis_ctrl;
	bool coupling_en, int_en, en;
	int ret;

	ret = regmap_read(st->regmap, ADXL345_REG_ACT_INACT_CTRL, &axis_ctrl);
	if (ret)
		return ret;

	switch (type) {
	case ADXL345_ACTIVITY:
	case ADXL345_ACTIVITY_AC:
		en = FIELD_GET(ADXL345_ACT_X_EN, axis_ctrl) |
			FIELD_GET(ADXL345_ACT_Y_EN, axis_ctrl) |
			FIELD_GET(ADXL345_ACT_Z_EN, axis_ctrl);
		break;
	case ADXL345_INACTIVITY:
	case ADXL345_INACTIVITY_AC:
		en = FIELD_GET(ADXL345_INACT_X_EN, axis_ctrl) |
			FIELD_GET(ADXL345_INACT_Y_EN, axis_ctrl) |
			FIELD_GET(ADXL345_INACT_Z_EN, axis_ctrl);
		break;
	case ADXL345_INACTIVITY_FF:
		en = true;
		break;
	default:
		return -EINVAL;
	}

	if (!en)
		return en;

	/* Check if corresponding interrupts are enabled */
	ret = regmap_read(st->regmap, ADXL345_REG_INT_ENABLE, &regval);
	if (ret)
		return ret;
	int_en = adxl345_act_int_reg[type] & regval;

	/* Check if coupling is enabled */
	ret = adxl345_is_act_inact_ac(st, type);
	if (ret < 0)
		return ret;
	coupling_en = ret;

	return coupling_en && int_en;
}

static int adxl345_set_act_inact_linkbit(struct adxl345_state *st,
					 enum adxl345_activity_type type,
					 bool en)
{
	int act_en, act_ac_en, inact_en, inact_ac_en;

	act_en = adxl345_is_act_inact_en(st, ADXL345_ACTIVITY);
	if (act_en < 0)
		return act_en;

	act_ac_en = adxl345_is_act_inact_en(st, ADXL345_ACTIVITY_AC);
	if (act_ac_en < 0)
		return act_ac_en;

	act_en = act_en || act_ac_en;

	if (type == ADXL345_INACTIVITY_FF) {
		inact_en = false;
	} else {
		inact_en = adxl345_is_act_inact_en(st, ADXL345_INACTIVITY);
		if (inact_en < 0)
			return inact_en;

		inact_ac_en = adxl345_is_act_inact_en(st, ADXL345_INACTIVITY_AC);
		if (inact_ac_en < 0)
			return inact_ac_en;

		inact_en = inact_en || inact_ac_en;
	}

	en = en && act_en && inact_en;

	return regmap_assign_bits(st->regmap, ADXL345_REG_POWER_CTL,
				  (ADXL345_POWER_CTL_AUTO_SLEEP | ADXL345_POWER_CTL_LINK),
				  en);
}

static int adxl345_set_act_inact_en(struct adxl345_state *st,
				    enum adxl345_activity_type type,
				    bool cmd_en)
{
	bool en;
	unsigned int threshold;
	u32 axis_ctrl;
	int ret;

	/*
	 * In case of turning off, assure turning off a correspondent coupling
	 * event. In case of not matching coupling, simply return.
	 */
	if (!cmd_en) {
		/* Expected positive true if coupling matches coupling type */
		if (adxl345_is_act_inact_ac(st, type) <= 0)
			return 0;
	}

	/* Start configuring the sensor registers */
	ret = adxl345_set_measure_en(st, false);
	if (ret)
		return ret;

	if (type == ADXL345_ACTIVITY || type == ADXL345_ACTIVITY_AC) {
		axis_ctrl = ADXL345_ACT_X_EN | ADXL345_ACT_Y_EN |
				ADXL345_ACT_Z_EN;

		ret = regmap_read(st->regmap, adxl345_act_thresh_reg[type], &threshold);
		if (ret)
			return ret;
	} else {
		axis_ctrl = ADXL345_INACT_X_EN | ADXL345_INACT_Y_EN |
				ADXL345_INACT_Z_EN;

		threshold = st->inact_threshold;
	}

	ret = regmap_assign_bits(st->regmap, ADXL345_REG_ACT_INACT_CTRL,
				 axis_ctrl, cmd_en);
	if (ret)
		return ret;

	ret = adxl345_set_act_inact_ac(st, type);
	if (ret)
		return ret;

	en = false;

	switch (type) {
	case ADXL345_ACTIVITY:
	case ADXL345_ACTIVITY_AC:
		en = FIELD_GET(ADXL345_REG_ACT_AXIS_MSK, axis_ctrl) &&
			threshold;
		break;
	case ADXL345_INACTIVITY:
	case ADXL345_INACTIVITY_AC:
	case ADXL345_INACTIVITY_FF:
		en = FIELD_GET(ADXL345_REG_INACT_AXIS_MSK, axis_ctrl) &&
			threshold && st->inact_time_ms;
		break;
	default:
		return -EINVAL;
	}

	/* Enable interrupts accordingly. */
	ret = regmap_assign_bits(st->regmap, ADXL345_REG_INT_ENABLE,
				 adxl345_act_int_reg[type], cmd_en && en);
	if (ret)
		return ret;

	/* Set sleep and link bit when ACT and INACT are enabled. */
	ret = adxl345_set_act_inact_linkbit(st, type, en);
	if (ret)
		return ret;

	return adxl345_set_measure_en(st, true);
}

/* tap */

static int _adxl345_set_tap_int(struct adxl345_state *st,
				enum adxl345_tap_type type, bool state)
{
	unsigned int int_map = 0x00;
	unsigned int tap_threshold;
	bool axis_valid;
	bool singletap_args_valid = false;
	bool doubletap_args_valid = false;
	bool en = false;
	u32 axis_ctrl;
	int ret;

	ret = regmap_read(st->regmap, ADXL345_REG_TAP_AXIS, &axis_ctrl);
	if (ret)
		return ret;

	axis_valid = FIELD_GET(ADXL345_REG_TAP_AXIS_MSK, axis_ctrl) > 0;

	ret = regmap_read(st->regmap, ADXL345_REG_THRESH_TAP, &tap_threshold);
	if (ret)
		return ret;

	/*
	 * Note: A value of 0 for threshold and/or dur may result in undesirable
	 *	 behavior if single tap/double tap interrupts are enabled.
	 */
	singletap_args_valid = tap_threshold > 0 && st->tap_duration_us > 0;

	if (type == ADXL345_SINGLE_TAP) {
		en = axis_valid && singletap_args_valid;
	} else {
		/* doubletap: Window must be equal or greater than latent! */
		doubletap_args_valid = st->tap_latent_us > 0 &&
			st->tap_window_us > 0 &&
			st->tap_window_us >= st->tap_latent_us;

		en = axis_valid && singletap_args_valid && doubletap_args_valid;
	}

	if (state && en)
		int_map |= adxl345_tap_int_reg[type];

	return regmap_update_bits(st->regmap, ADXL345_REG_INT_ENABLE,
				  adxl345_tap_int_reg[type], int_map);
}

static int adxl345_is_tap_en(struct adxl345_state *st,
			     enum iio_modifier axis,
			     enum adxl345_tap_type type, bool *en)
{
	unsigned int regval;
	u32 axis_ctrl;
	int ret;

	ret = regmap_read(st->regmap, ADXL345_REG_TAP_AXIS, &axis_ctrl);
	if (ret)
		return ret;

	/* Verify if axis is enabled for the tap detection. */
	switch (axis) {
	case IIO_MOD_X:
		*en = FIELD_GET(ADXL345_TAP_X_EN, axis_ctrl);
		break;
	case IIO_MOD_Y:
		*en = FIELD_GET(ADXL345_TAP_Y_EN, axis_ctrl);
		break;
	case IIO_MOD_Z:
		*en = FIELD_GET(ADXL345_TAP_Z_EN, axis_ctrl);
		break;
	default:
		*en = false;
		return -EINVAL;
	}

	if (*en) {
		/*
		 * If axis allow for tap detection, verify if the interrupt is
		 * enabled for tap detection.
		 */
		ret = regmap_read(st->regmap, ADXL345_REG_INT_ENABLE, &regval);
		if (ret)
			return ret;

		*en = adxl345_tap_int_reg[type] & regval;
	}

	return 0;
}

static int adxl345_set_singletap_en(struct adxl345_state *st,
				    enum iio_modifier axis, bool en)
{
	int ret;
	u32 axis_ctrl;

	switch (axis) {
	case IIO_MOD_X:
		axis_ctrl = ADXL345_TAP_X_EN;
		break;
	case IIO_MOD_Y:
		axis_ctrl = ADXL345_TAP_Y_EN;
		break;
	case IIO_MOD_Z:
		axis_ctrl = ADXL345_TAP_Z_EN;
		break;
	default:
		return -EINVAL;
	}

	if (en)
		ret = regmap_set_bits(st->regmap, ADXL345_REG_TAP_AXIS,
				      axis_ctrl);
	else
		ret = regmap_clear_bits(st->regmap, ADXL345_REG_TAP_AXIS,
					axis_ctrl);
	if (ret)
		return ret;

	return _adxl345_set_tap_int(st, ADXL345_SINGLE_TAP, en);
}

static int adxl345_set_doubletap_en(struct adxl345_state *st, bool en)
{
	int ret;

	/*
	 * Generally suppress detection of spikes during the latency period as
	 * double taps here, this is fully optional for double tap detection
	 */
	ret = regmap_update_bits(st->regmap, ADXL345_REG_TAP_AXIS,
				 ADXL345_REG_TAP_SUPPRESS_MSK,
				 en ? ADXL345_REG_TAP_SUPPRESS : 0x00);
	if (ret)
		return ret;

	return _adxl345_set_tap_int(st, ADXL345_DOUBLE_TAP, en);
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

static int adxl345_find_odr(struct adxl345_state *st, int val,
			    int val2, enum adxl345_odr *odr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(adxl345_odr_tbl); i++) {
		if (val == adxl345_odr_tbl[i][0] &&
		    val2 == adxl345_odr_tbl[i][1]) {
			*odr = i;
			return 0;
		}
	}

	return -EINVAL;
}

static int adxl345_set_odr(struct adxl345_state *st, enum adxl345_odr odr)
{
	int ret;

	ret = regmap_update_bits(st->regmap, ADXL345_REG_BW_RATE,
				 ADXL345_BW_RATE_MSK,
				 FIELD_PREP(ADXL345_BW_RATE_MSK, odr));
	if (ret)
		return ret;

	/* update inactivity time by ODR */
	return adxl345_set_inact_time(st, 0, 0);
}

static int adxl345_find_range(struct adxl345_state *st, int val, int val2,
			      enum adxl345_range *range)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(adxl345_fullres_range_tbl); i++) {
		if (val == adxl345_fullres_range_tbl[i][0] &&
		    val2 == adxl345_fullres_range_tbl[i][1]) {
			*range = i;
			return 0;
		}
	}

	return -EINVAL;
}

static int adxl345_set_range(struct adxl345_state *st, enum adxl345_range range)
{
	unsigned int act_threshold, inact_threshold;
	unsigned int range_old;
	unsigned int regval;
	int ret;

	ret = regmap_read(st->regmap, ADXL345_REG_DATA_FORMAT, &regval);
	if (ret)
		return ret;
	range_old = FIELD_GET(ADXL345_DATA_FORMAT_RANGE, regval);

	ret = regmap_read(st->regmap,
			  adxl345_act_thresh_reg[ADXL345_ACTIVITY],
			  &act_threshold);
	if (ret)
		return ret;

	ret = regmap_update_bits(st->regmap, ADXL345_REG_DATA_FORMAT,
				 ADXL345_DATA_FORMAT_RANGE,
				 FIELD_PREP(ADXL345_DATA_FORMAT_RANGE, range));
	if (ret)
		return ret;

	act_threshold = act_threshold * adxl345_range_factor_tbl[range_old]
		/ adxl345_range_factor_tbl[range];
	act_threshold = min(U8_MAX, max(1, act_threshold));

	inact_threshold = st->inact_threshold;
	inact_threshold = inact_threshold * adxl345_range_factor_tbl[range_old]
		/ adxl345_range_factor_tbl[range];
	inact_threshold = min(U8_MAX, max(1, inact_threshold));

	ret = regmap_write(st->regmap, adxl345_act_thresh_reg[ADXL345_ACTIVITY],
			   act_threshold);
	if (ret)
		return ret;

	return adxl345_set_inact_threshold(st, inact_threshold);
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
	enum adxl345_odr odr;
	enum adxl345_range range;
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
		if (ret)
			return ret;

		*val = sign_extend32(le16_to_cpu(accel), 12);
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		ret = regmap_read(st->regmap, ADXL345_REG_DATA_FORMAT, &regval);
		if (ret)
			return ret;
		range = FIELD_GET(ADXL345_DATA_FORMAT_RANGE, regval);
		*val = adxl345_fullres_range_tbl[range][0];
		*val2 = adxl345_fullres_range_tbl[range][1];
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
		ret = regmap_read(st->regmap, ADXL345_REG_BW_RATE, &regval);
		if (ret)
			return ret;
		odr = FIELD_GET(ADXL345_BW_RATE_MSK, regval);
		*val = adxl345_odr_tbl[odr][0];
		*val2 = adxl345_odr_tbl[odr][1];
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
		 * factor
		 */
		ret = regmap_write(st->regmap,
				   ADXL345_REG_OFS_AXIS(chan->address),
				   val / 4);
		if (ret)
			return ret;
		break;
	case IIO_CHAN_INFO_SAMP_FREQ:
		ret = adxl345_find_odr(st, val, val2, &odr);
		if (ret)
			return ret;

		ret = adxl345_set_odr(st, odr);
		if (ret)
			return ret;
		break;
	case IIO_CHAN_INFO_SCALE:
		ret = adxl345_find_range(st, val, val2,	&range);
		if (ret)
			return ret;

		ret = adxl345_set_range(st, range);
		if (ret)
			return ret;
		break;
	default:
		return -EINVAL;
	}

	return adxl345_set_measure_en(st, true);
}

static int adxl345_read_mag_config(struct adxl345_state *st,
				   const struct iio_chan_spec *chan,
				   enum iio_event_direction dir,
				   enum adxl345_activity_type type_act,
				   enum adxl345_activity_type type_inact)
{
	switch (dir) {
	case IIO_EV_DIR_RISING:
		return adxl345_is_act_inact_en(st, type_act);
	case IIO_EV_DIR_FALLING:
		return adxl345_is_act_inact_en(st, type_inact);
	default:
		return -EINVAL;
	}
}

static int adxl345_write_mag_config(struct adxl345_state *st,
				    const struct iio_chan_spec *chan,
				    enum iio_event_direction dir,
				    enum iio_event_type type,
				    enum adxl345_activity_type type_act,
				    enum adxl345_activity_type type_inact,
				    bool state)
{
	switch (dir) {
	case IIO_EV_DIR_RISING:
		return adxl345_set_act_inact_en(st, type_act, state);
	case IIO_EV_DIR_FALLING:
		return adxl345_set_act_inact_en(st, type_inact, state);
	default:
		return -EINVAL;
	}
}

static int adxl345_read_event_config(struct iio_dev *indio_dev,
				     const struct iio_chan_spec *chan,
				     enum iio_event_type type,
				     enum iio_event_direction dir)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	bool int_en;
	int ret;

	switch (type) {
	case IIO_EV_TYPE_MAG:
		return adxl345_read_mag_config(st, chan, dir,
					       ADXL345_ACTIVITY,
					       ADXL345_INACTIVITY);
	case IIO_EV_TYPE_MAG_ADAPTIVE:
		return adxl345_read_mag_config(st, chan, dir,
					       ADXL345_ACTIVITY_AC,
					       ADXL345_INACTIVITY_AC);
	case IIO_EV_TYPE_GESTURE:
		switch (dir) {
		case IIO_EV_DIR_SINGLETAP:
			ret = adxl345_is_tap_en(st, chan->channel2,
						ADXL345_SINGLE_TAP, &int_en);
			if (ret)
				return ret;
			return int_en;
		case IIO_EV_DIR_DOUBLETAP:
			ret = adxl345_is_tap_en(st, chan->channel2,
						ADXL345_DOUBLE_TAP, &int_en);
			if (ret)
				return ret;
			return int_en;
		default:
			return -EINVAL;
		}
	default:
		return -EINVAL;
	}
}

static int adxl345_write_event_config(struct iio_dev *indio_dev,
				      const struct iio_chan_spec *chan,
				      enum iio_event_type type,
				      enum iio_event_direction dir,
				      int state)
{
	struct adxl345_state *st = iio_priv(indio_dev);

	switch (type) {
	case IIO_EV_TYPE_MAG:
		return adxl345_write_mag_config(st, chan, dir, type,
						ADXL345_ACTIVITY,
						ADXL345_INACTIVITY,
						state);
	case IIO_EV_TYPE_MAG_ADAPTIVE:
		return adxl345_write_mag_config(st, chan, dir, type,
						ADXL345_ACTIVITY_AC,
						ADXL345_INACTIVITY_AC,
						state);
	case IIO_EV_TYPE_GESTURE:
		switch (dir) {
		case IIO_EV_DIR_SINGLETAP:
			return adxl345_set_singletap_en(st, chan->channel2, state);
		case IIO_EV_DIR_DOUBLETAP:
			return adxl345_set_doubletap_en(st, state);
		default:
			return -EINVAL;
		}
	default:
		return -EINVAL;
	}
}

static int adxl345_read_mag_value(struct adxl345_state *st,
				  enum adxl345_activity_type type_act,
				  enum adxl345_activity_type type_inact, /* TODO needed */
				  enum iio_event_direction dir,
				  enum iio_event_info info,
				  int *val, int *val2)
{
	unsigned int act_threshold;
	int ret;

	switch (info) {
	case IIO_EV_INFO_VALUE:
		switch (dir) {
		case IIO_EV_DIR_RISING:
			ret = regmap_read(st->regmap,
					  adxl345_act_thresh_reg[type_act],
					  &act_threshold);
			if (ret)
				return ret;
			*val = 62500 * act_threshold;
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		case IIO_EV_DIR_FALLING:
			*val = 62500 * st->inact_threshold;
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		default:
			return -EINVAL;
		}
	case IIO_EV_INFO_PERIOD:
		*val = st->inact_time_ms;
		*val2 = MILLI;
		return IIO_VAL_FRACTIONAL;
	default:
		return -EINVAL;
	}
}

static int adxl345_write_mag_value(struct adxl345_state *st,
				   enum adxl345_activity_type type_act,
				   enum adxl345_activity_type type_inact,
				   enum iio_event_direction dir,
				   enum iio_event_info info,
				   int val, int val2)
{
	switch (info) {
	case IIO_EV_INFO_VALUE:
		/*
		 * Scaling factor 62.5mg/LSB, i.e. ~16g corresponds to 0xff
		 */
		val = DIV_ROUND_CLOSEST(val * MICRO + val2, 62500);
		switch (dir) {
		case IIO_EV_DIR_RISING:
			return regmap_write(st->regmap,
					    adxl345_act_thresh_reg[type_act],
					    val);
		case IIO_EV_DIR_FALLING:
			return adxl345_set_inact_threshold(st, val);
		default:
			return -EINVAL;
		}
	case IIO_EV_INFO_PERIOD:
		return adxl345_set_inact_time(st, val, val2);
	default:
		return -EINVAL;
	}
}

static int adxl345_read_event_value(struct iio_dev *indio_dev,
				    const struct iio_chan_spec *chan,
				    enum iio_event_type type,
				    enum iio_event_direction dir,
				    enum iio_event_info info,
				    int *val, int *val2)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	unsigned int tap_threshold;
	int ret;

	switch (type) {
	case IIO_EV_TYPE_MAG:
		return adxl345_read_mag_value(st, ADXL345_ACTIVITY,
					      ADXL345_INACTIVITY, dir, info,
					      val, val2);
	case IIO_EV_TYPE_MAG_ADAPTIVE:
		return adxl345_read_mag_value(st, ADXL345_ACTIVITY_AC,
					      ADXL345_INACTIVITY_AC, dir, info,
					      val, val2);
	case IIO_EV_TYPE_GESTURE:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			/*
			 * Scale factor is 62.5mg/LSB i.e. 0xff = 16g
			 */
			ret = regmap_read(st->regmap, ADXL345_REG_THRESH_TAP,
					  &tap_threshold);
			if (ret)
				return ret;
			*val = 62500 * sign_extend32(tap_threshold, 7);
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		case IIO_EV_INFO_TIMEOUT:
			*val = st->tap_duration_us;
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		case IIO_EV_INFO_RESET_TIMEOUT:
			*val = st->tap_window_us;
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		case IIO_EV_INFO_TAP2_MIN_DELAY:
			*val = st->tap_latent_us;
			*val2 = MICRO;
			return IIO_VAL_FRACTIONAL;
		default:
			return -EINVAL;
		}
	default:
		return -EINVAL;
	}
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
	case IIO_EV_TYPE_MAG:
		ret = adxl345_write_mag_value(st,
					      ADXL345_ACTIVITY,
					      ADXL345_INACTIVITY,
					      dir, info, val, val2);
		if (ret)
			return ret;
		break;
	case IIO_EV_TYPE_MAG_ADAPTIVE:
		ret = adxl345_write_mag_value(st,
					      ADXL345_ACTIVITY_AC,
					      ADXL345_INACTIVITY_AC,
					      dir, info, val, val2);
		if (ret)
			return ret;
		break;
	case IIO_EV_TYPE_GESTURE:
		switch (info) {
		case IIO_EV_INFO_VALUE:
			val = DIV_ROUND_CLOSEST(val * MICRO + val2, 62500);
			ret = regmap_write(st->regmap, ADXL345_REG_THRESH_TAP,
					   min(val, 0xFF));
			if (ret)
				return ret;
			break;
		case IIO_EV_INFO_TIMEOUT:
			ret = adxl345_set_tap_duration(st, val, val2);
			if (ret)
				return ret;
			break;
		case IIO_EV_INFO_RESET_TIMEOUT:
			ret = adxl345_set_tap_window(st, val, val2);
			if (ret)
				return ret;
			break;
		case IIO_EV_INFO_TAP2_MIN_DELAY:
			ret = adxl345_set_tap_latent(st, val, val2);
			if (ret)
				return ret;
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

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
	const unsigned int fifo_mask = 0x1F, watermark_mask = 0x02;
	int ret;

	value = min(value, ADXL345_FIFO_SIZE - 1);

	ret = regmap_update_bits(st->regmap, ADXL345_REG_FIFO_CTL, fifo_mask, value);
	if (ret)
		return ret;

	st->watermark = value;
	return regmap_update_bits(st->regmap, ADXL345_REG_INT_ENABLE,
				  watermark_mask, ADXL345_INT_WATERMARK);
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

static void adxl345_powerdown(void *ptr)
{
	struct adxl345_state *st = ptr;

	adxl345_set_measure_en(st, false);
}

static int adxl345_set_fifo(struct adxl345_state *st)
{
	unsigned int intio;
	int ret;

	/* FIFO should only be configured while in standby mode */
	ret = adxl345_set_measure_en(st, false);
	if (ret)
		return ret;

	ret = regmap_read(st->regmap, ADXL345_REG_INT_MAP, &intio);
	if (ret)
		return ret;

	ret = regmap_write(st->regmap, ADXL345_REG_FIFO_CTL,
			   FIELD_PREP(ADXL345_FIFO_CTL_SAMPLES_MSK,
				      st->watermark) |
			   FIELD_PREP(ADXL345_FIFO_CTL_TRIGGER_MSK, intio) |
			   FIELD_PREP(ADXL345_FIFO_CTL_MODE_MSK,
				      st->fifo_mode));
	if (ret)
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
	if (ret)
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
	int i, ret = 0;

	for (i = 0; i < samples; i++) {
		/* read 3x 2 byte elements from base address into next fifo_buf position */
		ret = regmap_bulk_read(st->regmap, ADXL345_REG_XYZ_BASE,
				       st->fifo_buf + (i * ADXL345_DIRS),
				       2 * ADXL345_DIRS);
		if (ret)
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

	st->fifo_mode = ADXL345_FIFO_STREAM;
	return adxl345_set_fifo(st);
}

static int adxl345_buffer_predisable(struct iio_dev *indio_dev)
{
	struct adxl345_state *st = iio_priv(indio_dev);
	int ret;

	st->fifo_mode = ADXL345_FIFO_BYPASS;
	ret = adxl345_set_fifo(st);
	if (ret)
		return ret;

	return regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, 0x00);
}

static const struct iio_buffer_setup_ops adxl345_buffer_ops = {
	.postenable = adxl345_buffer_postenable,
	.predisable = adxl345_buffer_predisable,
};

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
			      enum iio_modifier tap_dir,
			      enum iio_modifier act_dir)
{
	s64 ts = iio_get_time_ns(indio_dev);
	struct adxl345_state *st = iio_priv(indio_dev);
	unsigned int regval;
	int samples;
	int ret = -ENOENT;

	if (FIELD_GET(ADXL345_INT_SINGLE_TAP, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0, tap_dir,
							IIO_EV_TYPE_GESTURE,
							IIO_EV_DIR_SINGLETAP),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_DOUBLE_TAP, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0, tap_dir,
							IIO_EV_TYPE_GESTURE,
							IIO_EV_DIR_DOUBLETAP),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_ACTIVITY, int_stat)) {
		ret = regmap_read(st->regmap, ADXL345_REG_ACT_INACT_CTRL, &regval);
		if (ret)
			return ret;

		if (FIELD_GET(ADXL345_REG_ACT_ACDC_MSK, regval)) {
			/* AC coupled */
			ret = iio_push_event(indio_dev,
					     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0, act_dir,
								IIO_EV_TYPE_MAG_ADAPTIVE,
								IIO_EV_DIR_RISING),
					     ts);

		} else {
			/* DC coupled, relying on THRESH */
			ret = iio_push_event(indio_dev,
					     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0, act_dir,
								IIO_EV_TYPE_MAG,
								IIO_EV_DIR_RISING),
					     ts);
		}
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_INACTIVITY, int_stat)) {
		ret = regmap_read(st->regmap, ADXL345_REG_ACT_INACT_CTRL, &regval);
		if (ret)
			return ret;

		if (FIELD_GET(ADXL345_REG_INACT_ACDC_MSK, regval)) {
			/* AC coupled */
			ret = iio_push_event(indio_dev,
					     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
								IIO_MOD_X_AND_Y_AND_Z,
								IIO_EV_TYPE_MAG_ADAPTIVE,
								IIO_EV_DIR_FALLING),
					     ts);
		} else {
			/* DC coupled, relying on THRESH */
			ret = iio_push_event(indio_dev,
					     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
								IIO_MOD_X_AND_Y_AND_Z,
								IIO_EV_TYPE_MAG,
								IIO_EV_DIR_FALLING),
					     ts);
		}
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_FREE_FALL, int_stat)) {
		ret = iio_push_event(indio_dev,
				     IIO_MOD_EVENT_CODE(IIO_ACCEL, 0,
							IIO_MOD_X_AND_Y_AND_Z,
							IIO_EV_TYPE_MAG,
							IIO_EV_DIR_FALLING),
				     ts);
		if (ret)
			return ret;
	}

	if (FIELD_GET(ADXL345_INT_WATERMARK, int_stat)) {
		samples = adxl345_get_samples(st);
		if (samples < 0)
			return -EINVAL;

		if (adxl345_fifo_push(indio_dev, samples) < 0)
			return -EINVAL;

		ret = 0;
	}

	return ret;
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
	unsigned int regval;
	enum iio_modifier tap_dir = IIO_NO_MOD;
	enum iio_modifier act_dir = IIO_NO_MOD;
	u32 axis_ctrl;
	int int_stat;
	int ret;

	ret = regmap_read(st->regmap, ADXL345_REG_TAP_AXIS, &axis_ctrl);
	if (ret)
		return IRQ_NONE;

	if (FIELD_GET(ADXL345_REG_TAP_AXIS_MSK, axis_ctrl) ||
	    FIELD_GET(ADXL345_REG_ACT_AXIS_MSK, axis_ctrl)) {
		ret = regmap_read(st->regmap, ADXL345_REG_ACT_TAP_STATUS, &regval);
		if (ret)
			return IRQ_NONE;

		if (FIELD_GET(ADXL345_TAP_Z_EN, regval))
			tap_dir = IIO_MOD_Z;
		else if (FIELD_GET(ADXL345_TAP_Y_EN, regval))
			tap_dir = IIO_MOD_Y;
		else if (FIELD_GET(ADXL345_TAP_X_EN, regval))
			tap_dir = IIO_MOD_X;

		if (FIELD_GET(ADXL345_ACT_Z_EN, regval))
			act_dir = IIO_MOD_Z;
		else if (FIELD_GET(ADXL345_ACT_Y_EN, regval))
			act_dir = IIO_MOD_Y;
		else if (FIELD_GET(ADXL345_ACT_X_EN, regval))
			act_dir = IIO_MOD_X;
	}

	if (regmap_read(st->regmap, ADXL345_REG_INT_SOURCE, &int_stat))
		return IRQ_NONE;

	if (adxl345_push_event(indio_dev, int_stat, tap_dir, act_dir))
		goto err;

	if (FIELD_GET(ADXL345_INT_OVERRUN, int_stat))
		goto err;

	return IRQ_HANDLED;

err:
	adxl345_fifo_reset(st);

	return IRQ_HANDLED;
}

static const struct iio_info adxl345_info = {
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
	u8 intio = ADXL345_INT1;
	unsigned int data_format_mask = (ADXL345_DATA_FORMAT_RANGE |
					 ADXL345_DATA_FORMAT_JUSTIFY |
					 ADXL345_DATA_FORMAT_FULL_RES |
					 ADXL345_DATA_FORMAT_SELF_TEST);
	unsigned int tap_threshold;
	int irq;
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

	/* Init with reasonable values */
	tap_threshold = 48;			/*   48 [0x30] -> ~3g     */
	st->tap_duration_us = 16;		/*   16 [0x10] -> .010    */
	st->tap_window_us = 64;			/*   64 [0x40] -> .080    */
	st->tap_latent_us = 16;			/*   16 [0x10] -> .020    */

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
	ret = regmap_write(st->regmap, ADXL345_REG_INT_ENABLE, 0x00);
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
	if (ret)
		return dev_err_probe(dev, ret, "Error reading device ID\n");

	if (regval != ADXL345_DEVID)
		return dev_err_probe(dev, -ENODEV, "Invalid device ID: %x, expected %x\n",
				     regval, ADXL345_DEVID);

	/* Enable measurement mode */
	ret = adxl345_set_measure_en(st, true);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to enable measurement mode\n");

	ret = devm_add_action_or_reset(dev, adxl345_powerdown, st);
	if (ret)
		return ret;

	irq = fwnode_irq_get_byname(dev_fwnode(dev), "INT1");
	if (irq < 0) {
		intio = ADXL345_INT2;
		irq = fwnode_irq_get_byname(dev_fwnode(dev), "INT2");
		if (irq < 0)
			intio = ADXL345_INT_NONE;
	}

	if (intio != ADXL345_INT_NONE) {
		/*
		 * Any bits set to 0 in the INT map register send their respective
		 * interrupts to the INT1 pin, whereas bits set to 1 send their respective
		 * interrupts to the INT2 pin. The intio shall convert this accordingly.
		 */
		ret = regmap_assign_bits(st->regmap, ADXL345_REG_INT_MAP,
					 U8_MAX, intio);
		if (ret)
			return ret;

		/*
		 * Initialization with reasonable values to simplify operation
		 * of the sensor. The default values are partly taken from the
		 * older input driver for the ADXL345, and partly based on
		 * recommendations in the datasheet.
		 */
		ret = regmap_write(st->regmap, ADXL345_REG_ACT_INACT_CTRL, 0);
		if (ret)
			return ret;

		ret = regmap_write(st->regmap, ADXL345_REG_THRESH_ACT, 6);
		if (ret)
			return ret;

		ret = adxl345_set_inact_threshold(st, 4);
		if (ret)
			return ret;

		ret = regmap_write(st->regmap, ADXL345_REG_THRESH_TAP, tap_threshold);
		if (ret)
			return ret;

		/* FIFO_STREAM mode is going to be activated later */
		ret = devm_iio_kfifo_buffer_setup(dev, indio_dev, &adxl345_buffer_ops);
		if (ret)
			return ret;

		ret = devm_request_threaded_irq(dev, irq, NULL,
						&adxl345_irq_handler,
						IRQF_SHARED | IRQF_ONESHOT,
						indio_dev->name, indio_dev);
		if (ret)
			return ret;
	} else {
		ret = regmap_write(st->regmap, ADXL345_REG_FIFO_CTL,
				   FIELD_PREP(ADXL345_FIFO_CTL_MODE_MSK,
					      ADXL345_FIFO_BYPASS));
		if (ret)
			return ret;
	}

	return devm_iio_device_register(dev, indio_dev);
}
EXPORT_SYMBOL_NS_GPL(adxl345_core_probe, IIO_ADXL345);

MODULE_AUTHOR("Eva Rachel Retuya <eraretuya@gmail.com>");
MODULE_DESCRIPTION("ADXL345 3-Axis Digital Accelerometer core driver");
MODULE_LICENSE("GPL v2");
