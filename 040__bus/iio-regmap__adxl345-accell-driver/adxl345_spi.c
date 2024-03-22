// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADXL345 3-Axis Digital Accelerometer SPI driver
 *
 * Copyright (c) 2017 Eva Rachel Retuya <eraretuya@gmail.com>
 */

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/spi/spi.h>

#include "adxl345.h"

#define ADXL345_MAX_SPI_FREQ_HZ		5000000

static const struct regmap_config adxl345_spi_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	 /* Setting bits 7 and 6 enables multiple-byte read */
	.read_flag_mask = BIT(7) | BIT(6),
};

static int adxl345_spi_setup(struct device *dev, struct regmap *regmap)
{
	struct spi_device *spi = container_of(dev, struct spi_device, dev);

	pr_info("%s(): called\n", __func__);

	if (spi->mode & SPI_3WIRE)
		return regmap_write(regmap, ADXL345_REG_DATA_FORMAT,
			    ADXL345_DATA_FORMAT_SPI);
	return 0;
}

static int adxl345_spi_probe(struct spi_device *spi)
{
	const struct adxl345_chip_info *chip_data;
	struct regmap *regmap;

	pr_info("%s(): called\n", __func__);

	/* Retrieve device name to pass it as driver specific data */
	chip_data = device_get_match_data(&spi->dev);
	if (!chip_data)
		chip_data = spi_get_device_match_data(spi);

	/* Bail out if max_speed_hz exceeds 5 MHz */
	if (spi->max_speed_hz > ADXL345_MAX_SPI_FREQ_HZ)
		return dev_err_probe(&spi->dev, -EINVAL, "SPI CLK, %d Hz exceeds 5 MHz\n",
				     spi->max_speed_hz);

	regmap = devm_regmap_init_spi(spi, &adxl345_spi_regmap_config);
	if (IS_ERR(regmap))
		return dev_err_probe(&spi->dev, PTR_ERR(regmap), "Error initializing regmap\n");

	return adxl345_core_probe(&spi->dev, regmap, chip_data, &adxl345_spi_setup);
}

static const struct spi_device_id adxl345_spi_id[] = {
	{ "adxl345spi", (kernel_ulong_t)&adxl3x5_chip_info[ADXL345] },
	{ "adxl375", (kernel_ulong_t)&adxl3x5_chip_info[ADXL375] },
	{ }
};
MODULE_DEVICE_TABLE(spi, adxl345_spi_id);

static const struct of_device_id adxl345_of_match[] = {
	{ .compatible = "lothars,adxl345spi", .data = &adxl3x5_chip_info[ADXL345] },
	{ .compatible = "adi,adxl375", .data = &adxl3x5_chip_info[ADXL375] },
	{ }
};
MODULE_DEVICE_TABLE(of, adxl345_of_match);

static const struct acpi_device_id adxl345_acpi_match[] = {
	{ "ADS0345", (kernel_ulong_t)&adxl3x5_chip_info[ADXL345] },
	{ }
};
MODULE_DEVICE_TABLE(acpi, adxl345_acpi_match);

static struct spi_driver adxl345_spi_driver = {
	.driver = {
		.name	= "adxl345_spi",
		.of_match_table = adxl345_of_match,
		.acpi_match_table = adxl345_acpi_match,
	},
	.probe		= adxl345_spi_probe,
	.id_table	= adxl345_spi_id,
};
module_spi_driver(adxl345_spi_driver);

MODULE_AUTHOR("Eva Rachel Retuya <eraretuya@gmail.com>");
MODULE_DESCRIPTION("ADXL345 3-Axis Digital Accelerometer SPI driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(IIO_ADXL345);
