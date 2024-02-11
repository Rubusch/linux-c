// SPDX-License-Identifier: GPL-2.0+
/*
*/
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>

#include "gpio_irq_poll.h"

unsigned int irq_number;
unsigned long old_jiffie;
static int irq_ready = 0;
static wait_queue_head_t waitqueue;

static irqreturn_t
gpiodev_irq_handler(int irq, void *dev_id)
{
	unsigned long diff = jiffies - old_jiffie;
	pr_info("%s(): called by irq\n", __func__);
	if (diff < 20) { // debounce
		return IRQ_HANDLED;
	}
	old_jiffie = jiffies;

	irq_ready = 1;
	wake_up(&waitqueue);
	return IRQ_HANDLED;
}

static unsigned int
gpiodev_poll(struct file *file, poll_table *wait)
{
	pr_info("%s(): called\n", __func__);
	poll_wait(file, &waitqueue, wait);
	if (1 == irq_ready) {
		irq_ready = 0;
		return POLLIN;
	}
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.poll = gpiodev_poll
};

static struct miscdevice gpio_dev = {
	.name = POLL_DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);

	init_waitqueue_head(&waitqueue);
	if (gpio_request(GPIO_NUM, "input-gpio")) {
		pr_err("%s(): failed to allocate gpio\n", __func__);
		return -EFAULT;
	}

	if (gpio_direction_input(GPIO_NUM)) {
		pr_err("%s: failed to set gpio direction\n", __func__);
		gpio_free(GPIO_NUM);
		return -EFAULT;
	}

	gpio_set_debounce(GPIO_NUM, 300);

	irq_number = gpio_to_irq(GPIO_NUM);
	ret = request_irq(irq_number,
			  gpiodev_irq_handler,
			  IRQF_TRIGGER_RISING,
			  "gpio_irq_poll",
			  NULL);
	if (ret != 0){
		pr_err("%s: failed to request interrupt\n", __func__);
		gpio_free(GPIO_NUM);
		return -EFAULT;
	}

	ret = misc_register(&gpio_dev);
	if (0 != ret) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		return -EFAULT;
	}

	pr_info("%s(): gpio %d is mapped to irq %d\n",
		__func__, GPIO_NUM, irq_number);

	return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	free_irq(irq_number, NULL);
	gpio_free(GPIO_NUM);
	misc_deregister(&gpio_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Polling on gpio lines.");
