// SPDX-License-Identifier: GPL-2.0+
/*
  IIO subsystem ADC with hardware triggering module.

  Demo for the industrial I/O API on the DC934a and a separate
  Button-R-Click board. The setup is the following.

        +-----------------------+   +-----------------------+
        | process               |   | process               |
        | cat out_voltage_0_raw |   | cat out_voltage_0_raw |
        +-----------------------+   +-----------------------+
                    |                           A
                    V                           |
      +------------------------------------+    |
      | LTC2422 dual ADC device            |    |
      | /sys/bus/iio/devices/iio:device2   |    |               user space
    +-| - registered on module loading     |----|-----------------------------+
    | | - unregistered on module unloading |    |               kernel space  |
    | +------------------------------------+    |                             |
    |               |                           |                             |
    |               |                           |                             |
    |               |     +-----------------------------------+               |
    |               |     | ltc2422_read_raw()                |               |
    |               |     | - waits for request on            |               |
    |               +---->|   waitqueue, goes to sleep        |               |
    |                     |-----------------------------------|               |
    |                     | - woken up by the IRQ, sends      |<---+          |
    |                     |   ADC data to user process        |    |          |
    | process context     +-----------------------------------+    |          |
    |--------------------------------------------------------------|----------|
    | interrupt context                                            |          |
    |                                           +-------------------------+   |
    |                                           | ltc2422_adc_interrupt() |   |
    |                                           | interrupt handler:      |   |
    |                                           | - wakes up the process  |   |
    |                                           +-------------------------+   |
    |                                                              A          |
    +--------------------------------------------------------------|-INT------+
                                                                   |
                                                       +----------------+
						       | Button-R-Click |
						       +----------------+
*/

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/iio/iio.h>
#include <linux/wait.h>

#define LTC2422_GPIO_NAME "int"

struct adc_data {
	struct gpio_desc *gpio;
	int irq;
	wait_queue_head_t wq_data_available;
	struct spi_device *spi;
	u8 buffer[4];
	bool conversion_done;
	struct mutex lock;
};

/* interrupt handler
 */
static irqreturn_t
ltc2422_adc_interrupt(int irq, void *data)
{
	struct adc_data *st = data;

	st->conversion_done = true;
	wake_up_interruptible(&st->wq_data_available);

	return IRQ_HANDLED;
}

static const struct iio_chan_spec ltc2422_channel[] = {
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.output = 1,
		.channel = 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
};

/* read raw
 */
static int
ltc2422_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan,
		 int *val, int *val2, long m)
{
	struct adc_data *st;
	struct device *dev;
	int ret;

	st = iio_priv(indio_dev);
	dev = &st->spi->dev;
	dev_info(dev, "%s() - called", __func__);

	dev_info(dev, "%s() - press microbus key to start conversion", __func__);

	switch(m) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&st->lock);

		ret = wait_event_interruptible(st->wq_data_available, st->conversion_done);
		if (ret) {
			dev_err(dev, "%s() - failed to request interrupt", __func__);
			mutex_unlock(&st->lock);
			return ret;
		}

		spi_read(st->spi, &st->buffer, 3);

		*val = st->buffer[0] << 16;
		*val |= st->buffer[1] << 8;
		*val |= st->buffer[2];

		st->conversion_done = false;

		mutex_unlock(&st->lock);
		return IIO_VAL_INT;

	default:
		break;
	}

	return -EINVAL;
}

static const struct iio_info ltc2422_info = {
	.read_raw = &ltc2422_read_raw,
};

/* probe
 */
int
ltc2422_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct adc_data *st;
	struct device *dev = &spi->dev;
	// NB: get the id from the driver structure to use the name!!!
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
	spi_set_drvdata(spi, indio_dev);

	// you can also use devm_gpiod_get(dev, LTC2422_GPIO_NAME, GPIOD_IN);
	st->gpio = devm_gpiod_get_index(dev, LTC2422_GPIO_NAME, 0, GPIOD_IN);
	if (IS_ERR(st->gpio)) {
		dev_err(dev, "%s() - devm_gpiod_get_index() failed", __func__);
		return PTR_ERR(st->gpio);
	}

	st->irq = gpiod_to_irq(st->gpio);
	if (st->irq < 0) {
		dev_err(dev, "%s() - gpiod_to_irq() failed", __func__);
		return st->irq;
	}

	dev_info(dev, "%s() - the irq number is '%d;", __func__, st->irq);

	indio_dev->name = id->name;
	indio_dev->dev.parent = &spi->dev;
	indio_dev->channels = ltc2422_channel;
	indio_dev->info = &ltc2422_info;
	indio_dev->num_channels = 1;
	indio_dev->modes = INDIO_DIRECT_MODE;

	init_waitqueue_head(&st->wq_data_available);
	mutex_init(&st->lock);

	ret = devm_request_irq(dev, st->irq, ltc2422_adc_interrupt,
			       IRQF_TRIGGER_FALLING, id->name, st);
	if (ret) {
		dev_err(dev, "%s() - devm_request_irq() failed", __func__);
		return ret;
	}

	ret = devm_iio_device_register(&spi->dev, indio_dev);
	if (0 > ret) {
		dev_err(dev, "%s() - devm_iio_device_register() failed", __func__);
		return ret;
	}

	st->conversion_done = false;

	return 0;
}

/* remove
 */
void
ltc2422_remove(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	dev_info(dev, "%s() - called", __func__);
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
MODULE_DEVICE_TABLE(spi, ltc2422_id);

static struct spi_driver ltc2422_driver = {
	.probe = ltc2422_probe,
	.remove = ltc2422_remove,
	.id_table = ltc2422_id,
	.driver = {
		.name = "ltc2422",
		.of_match_table = ltc2422_dt_ids,
	},
};
module_spi_driver(ltc2422_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("iio demo");
