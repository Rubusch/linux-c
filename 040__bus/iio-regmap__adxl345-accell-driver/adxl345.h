/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ADXL345 3-Axis Digital Accelerometer
 *
 * Copyright (c) 2017 Eva Rachel Retuya <eraretuya@gmail.com>
 */

#ifndef _ADXL345_H_
#define _ADXL345_H_

#define ADXL345_DATA_FORMAT_SPI         BIT(6) /* 1 in the SPI bit sets the device to 3-wire SPI mode,
						  0 sets the device to 4-wire SPI mode. */
enum adxl345_device_type {
	ADXL345	= 1,
	ADXL375 = 2,
};

int adxl345_core_probe(struct device *dev, struct regmap *regmap);

void adxl345_preset_data_format(u8 data_format);

#endif /* _ADXL345_H_ */
