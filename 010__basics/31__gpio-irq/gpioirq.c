// SPDX-License-Identifier: GPL-2.0+
/*
  uses the gpio lib, demonstrates IRQ interaction
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>

#define SELECTED_GPIO_LINE  (26 + GPIO_DYNAMIC_BASE)   /* don't do this at home!! */

extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;
static unsigned int irq_number; // holding the pin number of irq controller

static irqreturn_t
gpio_irq_handler(int irq, void *dev_id)
{
	unsigned long flags;
	unsigned long diff = jiffies - old_jiffie;

	if (diff < 20) { // debounce
		return IRQ_HANDLED;
	}
	old_jiffie = jiffies;
	local_irq_save(flags);
	pr_info("%s(): called, as triggered by IRQ\n", __func__);
	local_irq_restore(flags);

	return IRQ_HANDLED;
}

static int
__init mod_init(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);
	if (false == gpio_is_valid(SELECTED_GPIO_LINE)) {
		pr_err("%s(): gpio is invalid\n", __func__);
		return -EIO;
	}

	ret = gpio_request(SELECTED_GPIO_LINE, "SELECTED_GPIO_LINE");
	if (0 > ret) {
		pr_err("%s(): failed to allocate gpio %d\n", __func__, SELECTED_GPIO_LINE);
		return -EIO;
	}

	// prepare for reading on GPIO line
	gpio_direction_input(SELECTED_GPIO_LINE);

	// prepare IRQ when GPIO signals
	irq_number = gpio_to_irq(SELECTED_GPIO_LINE);
	pr_info("%s(): gpio has irq number %d\n", __func__, irq_number);

	// when implementing for a device, prefer the managed variant
	// of the request interrupt -> devm_request_irq()
	ret = request_irq(irq_number,
			  (void*) gpio_irq_handler,
			  IRQF_TRIGGER_RISING,
			  "happy_gpio_irq",
			  NULL);
	if (0 > ret) {
		pr_err("%s(): failed to request IRQ %d\n", __func__, irq_number);
		gpio_free(SELECTED_GPIO_LINE);
		return -EIO;
	}

	pr_info("%s(): gpio mapped to IRQ %d\n", __func__, irq_number);
	return 0;
}

static void
__exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	free_irq(irq_number, NULL); // since we did not use a managed irq line
	gpio_free(SELECTED_GPIO_LINE);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with gpios");
