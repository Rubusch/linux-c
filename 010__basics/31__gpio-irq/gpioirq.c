// SPDX-License-Identifier: GPL-2.0+
/*
  uses the gpio lib, demonstrates IRQ interaction
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>

// de-bouncing
extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;
#define GPIO_25_IN  (25)
static unsigned int irq_number; // holding the pin number of irq controller

static irqreturn_t
gpio_irq_handler(int irq, void *dev_id)
{
	unsigned long flags;
	unsigned long diff = jiffies - old_jiffie;
	if (diff < 20)
	{ // debounce
		return IRQ_HANDLED;
	}
	old_jiffie = jiffies;

	local_irq_save(flags);
	pr_info("%s(): called, as triggered by IRQ", __func__);
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static int
__init mod_init(void)
{
	int ret;
	pr_info("%s(): called", __func__);

	if (false == gpio_is_valid(GPIO_25_IN)) {
		pr_err("%s(): gpio is invalid", __func__);
		return -EIO;
	}

	ret = gpio_request(GPIO_25_IN, "GPIO_25_IN");
	if (0 > ret) {
		pr_err("%s(): failed to allocate gpio %d", __func__, GPIO_25_IN);
		return -EIO;
	}

	gpio_direction_input(GPIO_25_IN);

	irq_number = gpio_to_irq(GPIO_25_IN);
	pr_info("%s(): gpio has irq number %d", __func__, irq_number);

	// when implementing for a device, prefer
	// the managed variant of the request interrupt
	// ->   devm_request_irq()
	ret = request_irq(irq_number,
			  (void*) gpio_irq_handler,
			  IRQF_TRIGGER_RISING,
			  "lothars_gpio_irq",
			  NULL);
	if (0 > ret) {
		pr_err("%s(): failed to request IRQ %d", __func__, irq_number);
		gpio_free(GPIO_25_IN);
		return -EIO;
	}

	pr_info("%s(): gpio mapped to IRQ %d", __func__, irq_number);
	return 0;
}

static void
__exit mod_exit(void)
{
	pr_info("%s(): called", __func__);
	free_irq(irq_number, NULL);
	gpio_free(GPIO_25_IN);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with gpios");
