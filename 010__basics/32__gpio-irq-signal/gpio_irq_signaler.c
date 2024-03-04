// SPDX-License-Identifier: GPL-2.0+
/*
*/
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>

#include "gpio_irq_signaler.h"

unsigned int irq_number;

// manage userspace application via task handle
static struct task_struct *task = NULL;

static irqreturn_t
gpio_irq_signal_handler(int irq, void *dev_id)
{
	struct siginfo info;
	int ret;

	pr_info("%s(): called - irq triggered\n", __func__);
	if (task != NULL) {
		memset(&info, 0, sizeof(info));
		info.si_signo = SIG_NUM;
		info.si_code = SI_QUEUE;

		// send signal
		ret = send_sig_info(SIG_NUM, (struct kernel_siginfo *) &info, task);
		if (0 > ret) {
			pr_err("%s(): failed to send signal\n", __func__);
			return IRQ_NONE;
		}
	}
	return IRQ_HANDLED;
}

static int
gpio_close(struct inode *dev, struct file *inst)
{
	if (task != NULL)
		task = NULL;
	return 0;
}

static long int
gpio_ioctl(struct file *file, unsigned cmd, unsigned long arg)
{
	pr_info("%s(): called\n", __func__);
	if (cmd == REGISTER_UAPP) {
		task = get_current();
		pr_info("%s(): registered userspace application w/ pid %d\n",
			__func__, task->pid);
	}
	return 0;
}

static struct file_operations fops = {
	.release = gpio_close,
	.unlocked_ioctl = gpio_ioctl,
};

static struct miscdevice gpio_dev = {
	.name = GPIO_DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init mod_init(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);
	if (gpio_request(GPIO_NUM, "button-gpio")) {
		pr_err("%s(): failed to allocate GPIO\n", __func__);
		return -1;
	}

	if (gpio_direction_input(GPIO_NUM)) {
		pr_err("%s(): failed to set gpio direction\n", __func__);
		gpio_free(GPIO_NUM);
		return -1;
	}

	gpio_set_debounce(GPIO_NUM, 300);

	irq_number = gpio_to_irq(GPIO_NUM);
	ret = request_irq(irq_number,
			  gpio_irq_signal_handler,
			  IRQF_TRIGGER_RISING,
			  "gpio_irq_signaler",
			  NULL);
	if (0 != ret) {
		pr_err("%s(): failed to request interrupt %d\n",
		       __func__, irq_number);
		gpio_free(GPIO_NUM);
		return -1;
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
MODULE_DESCRIPTION("Sends a signal to a userspace application.");
