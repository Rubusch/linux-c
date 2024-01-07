// SPDX-License-Identifier: GPL-2.0+
/*
  IIO subsystem ADC module for the LTC2422 dual ADC

  Demo for the industrial I/O API, the iio as an SPI interaction.
  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018
*/

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

struct ltc2422_state {
	struct spi_device *spi;
	u8 buffer[4];
};

static const struct iio_chan_spec ltc2422_channel[] = {
	{
		.type		= IIO_VOLTAGE,
		.indexed	= 1,
		.output		= 1,
		.channel	= 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	}
};

static int
ltc2422_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long m)
{
	struct device *dev = indio_dev->dev.parent;
	struct ltc2422_state *st = iio_priv(indio_dev);
	int ret;

	dev_info(dev, "%s() - called", __func__);
	switch(m) {
	case IIO_CHAN_INFO_RAW:
		ret = spi_read(st->spi, &st->buffer, 3);
		if (0 > ret)
			return ret;

		*val = st->buffer[0] << 16;
		*val |= st->buffer[1] << 8;
		*val |= st->buffer[2];

		dev_info(&st->spi->dev, "the value is %x", *val);

		return IIO_VAL_INT;

	default:
		return -EINVAL;
	}
}

static const struct iio_info ltc2422_info = {
	.read_raw = &ltc2422_read_raw,
};

static int
ltc2422_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ltc2422_state *st;
	struct device *dev = &spi->dev;
	const struct spi_device_id *id = spi_get_device_id(spi);
	int ret;

	dev_info(dev, "%s() - called", __func__);
	indio_dev = devm_iio_device_alloc(dev, sizeof(*st));
	if (!indio_dev) {
		dev_err(dev, "%s() - devm_iio_device_alloc() failed", __func__);
		return -ENOMEM;
	}

	st = iio_priv(indio_dev);
	st->spi = spi;

	indio_dev->dev.parent = &spi->dev;
	indio_dev->channels = ltc2422_channel;
	indio_dev->info = &ltc2422_info;
	indio_dev->name = id->name;
	indio_dev->num_channels = 1;
	indio_dev->modes = INDIO_DIRECT_MODE;

	ret = devm_iio_device_register(dev, indio_dev);
	if (0 > ret) {
		dev_err(dev, "%s() - devm_iio_device_register() failed", __func__);
		return ret;
	}

	return 0;
}

static const struct of_device_id ltc2422_dt_ids[] = {
	{ .compatible = "lothars,ltc2422", },
	{ },
};
MODULE_DEVICE_TABLE(of, ltc2422_dt_ids);

static const struct spi_device_id ltc2422_id[] = {
	{ .name = "ltc2422", },
	{ },
};
MODULE_DEVICE_TABLE(SPI, ltc2422_id);

static struct spi_driver ltc2422_driver = {
	.probe		= ltc2422_probe,
	.id_table	= ltc2422_id,
	.driver = {
		.name = "ltc2422",
		.of_match_table = ltc2422_dt_ids,
	},
};
module_spi_driver(ltc2422_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("iio demo");
