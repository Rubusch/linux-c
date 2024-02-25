// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define PLATFORM_DRIVER_NAME "lothars-iio-dummy"

#define IIO_VOLTAGE_CHANNEL(num)					\
	{								\
		.type = IIO_VOLTAGE,					\
		.indexed = 1,						\
		.channel = (num),					\
		.address = (num),					\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),           \
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE)    \
	}

struct my_private_data {
	int aaa;
	int bbb;
	struct mutex lock;
};

static const unsigned long iiodrv_scan_masks[] = {0xf, 0};

static int iiodrv_read_raw(struct iio_dev *indio_dev,
			 struct iio_chan_spec const *channel, int *val,
			 int *val2, long mask)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static int iiodrv_write_raw(struct iio_dev *indio_dev,
			  struct iio_chan_spec const *chan,
			  int val, int val2, long mask)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static const struct iio_chan_spec iiodrv_channels[] = {
	IIO_VOLTAGE_CHANNEL(0),
	IIO_VOLTAGE_CHANNEL(1),
	IIO_VOLTAGE_CHANNEL(2),
	IIO_VOLTAGE_CHANNEL(3),
};

static const struct of_device_id iiodrv_ids[] = {
	{ .compatible = "lothars,iio-dummy", },
	{ },
};

static const struct iio_info iiodrv_iio_info = {
	.read_raw = iiodrv_read_raw,
	.write_raw = iiodrv_write_raw,
};

static int pdrv_probe (struct platform_device *pdev)
{
	struct iio_dev *indio_dev;
	struct my_private_data *data;

	pr_info("%s(): called\n", __func__);
	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*data));
	if (!indio_dev) {
		dev_err(&pdev->dev, "%s(): iio allocation failed!\n", __func__);
		return -ENOMEM;
	}

	data = iio_priv(indio_dev);
	mutex_init(&data->lock);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->info = &iiodrv_iio_info;
	indio_dev->name = KBUILD_MODNAME;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = iiodrv_channels;
	indio_dev->num_channels = ARRAY_SIZE(iiodrv_channels);
	indio_dev->available_scan_masks = iiodrv_scan_masks;

	platform_set_drvdata(pdev, indio_dev);

	return devm_iio_device_register(&pdev->dev, indio_dev);
}

static int pdrv_remove(struct platform_device *pdev)
{
	pr_info("%s(): called\n", __func__);

	return 0;
}

static struct platform_driver pdrv_driver = {
	.probe = pdrv_probe,
	.remove = pdrv_remove,
	.driver = {
		.name = PLATFORM_DRIVER_NAME,
		.of_match_table = of_match_ptr(iiodrv_ids),
	},
};
module_platform_driver(pdrv_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with iio devices.");
