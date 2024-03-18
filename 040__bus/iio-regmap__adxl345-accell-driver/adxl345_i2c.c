// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADXL345 3-Axis Digital Accelerometer I2C driver
 *
 * Copyright (c) 2017 Eva Rachel Retuya <eraretuya@gmail.com>
 *
 * 7-bit I2C slave address: 0x1D (ALT ADDRESS pin tied to VDDIO) or
 * 0x53 (ALT ADDRESS pin grounded)
 */

#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regmap.h>

#include "adxl345.h"

static const struct regmap_config adxl345_i2c_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static int adxl345_i2c_probe(struct i2c_client *client)
{
	const struct adxl345_chip_info *chip_data;
	struct regmap *regmap;

	pr_info("%s(): called\n", __func__);

	/* Retrieve device data, i.e. the name, to pass it to the core */
	chip_data = i2c_get_match_data(client);

	regmap = devm_regmap_init_i2c(client, &adxl345_i2c_regmap_config);
	if (IS_ERR(regmap))
		return dev_err_probe(&client->dev, PTR_ERR(regmap),
				     "Error initializing regmap\n");

	return adxl345_core_probe(&client->dev, regmap, chip_data, NULL);
}

static const struct i2c_device_id adxl345_i2c_id[] = {
	{ "adxl345i2c", (kernel_ulong_t)&adxl3x5_chip_info[ADXL345] },
	{ "adxl375", (kernel_ulong_t)&adxl3x5_chip_info[ADXL375] },
	{ }
};
MODULE_DEVICE_TABLE(i2c, adxl345_i2c_id);

static const struct of_device_id adxl345_of_match[] = {
	{ .compatible = "lothars,adxl345i2c", .data = &adxl3x5_chip_info[ADXL345] },
	{ .compatible = "adi,adxl375", .data = &adxl3x5_chip_info[ADXL375] },
	{ }
};
MODULE_DEVICE_TABLE(of, adxl345_of_match);

static const struct acpi_device_id adxl345_acpi_match[] = {
	{ "ADS0345", (kernel_ulong_t)&adxl3x5_chip_info[ADXL345] },
	{ }
};
MODULE_DEVICE_TABLE(acpi, adxl345_acpi_match);

static struct i2c_driver adxl345_i2c_driver = {
	.driver = {
		.name	= "adxl345_i2c",
		.of_match_table = adxl345_of_match,
		.acpi_match_table = adxl345_acpi_match,
	},
	.probe		= adxl345_i2c_probe,
	.id_table	= adxl345_i2c_id,
};
module_i2c_driver(adxl345_i2c_driver);

MODULE_AUTHOR("Eva Rachel Retuya <eraretuya@gmail.com>");
MODULE_DESCRIPTION("ADXL345 3-Axis Digital Accelerometer I2C driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(IIO_ADXL345);
