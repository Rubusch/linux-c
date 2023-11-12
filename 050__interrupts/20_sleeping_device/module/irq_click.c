/*
  Sleeping Device

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018

  VERIFIED:
  linux v6.3/aarch64
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/wait.h>  /* include wait queue */
//#include <linux/gpio/consumer.h> // TODO rm  

#define MAX_KEY_STATES 256

static char *LOTHARS_KEY_NAME = "PB_USER";
static char lothars_key_buf[MAX_KEY_STATES];
static int buf_rd, buf_wr;

/*
  struct for the "private" data, holding the device "dev"
*/
struct key_priv {
	struct device *dev;
	struct gpio_desc *gpio;
	struct miscdevice int_miscdevice;
	wait_queue_head_t wq_data_available;
	int irq;
};

/*
  the interrupt service routine (isr)

  obtain button state and update the lothars_key_buf states
 */
static irqreturn_t lothars_key_isr(int irq, void* data)
{
	int val;
	struct key_priv *priv = data;
	struct device *dev = priv->dev;

	dev_info(dev, "lothars_key_isr() - interrupt received, key: %s\n",
		 LOTHARS_KEY_NAME);

	val = gpiod_get_value(priv->gpio);
	dev_info(dev, "lothars_key_isr() - button state 0x%08x\n",
		 val);

	if (1 == val)
		lothars_key_buf[buf_wr++] = 'P';
	else
		lothars_key_buf[buf_rd++] = 'R';

	if (buf_wr >= MAX_KEY_STATES)
		buf_wr = 0;

	/* wake up the process */
	wake_up_interruptible(&priv->wq_data_available);

	return IRQ_HANDLED;
}

static ssize_t
lothars_dev_read(struct file* file, char __user *buff,
		 size_t count, loff_t *off)
{
	int ret;
	char ch[2];
	struct key_priv *priv;
	struct device* dev;

	// obtain the private data by container_of() macro from file member
	priv = container_of(file->private_data, struct key_priv, int_miscdevice);

	dev = priv->dev;
	dev_info(dev, "lothars_dev_read() - called\n");

	/*
	  put process to sleep - the condition is checked each time
	  the waitqueue is woken up
	 */
	ret = wait_event_interruptible(priv->wq_data_available, buf_wr != buf_rd);
	if (ret)
		return ret;

	/*
	  send values to user space (application)
	*/
	ch[0] = lothars_key_buf[buf_rd];
	ch[1] = '\n';
	if (copy_to_user(buff, &ch, 2))
		return -EFAULT;

	buf_rd++;
	if (MAX_KEY_STATES <= buf_rd)
		buf_rd = 0;

	*off += 1;
	return 2;
}

static const struct file_operations lothars_fops = {
	.owner = THIS_MODULE, // TODO really needed?   
	.read = lothars_dev_read,
};

//static int __init lothars_probe(struct platform_device* pdev)
static int
lothars_probe(struct platform_device* pdev)  // TODO try w/o __init   
{
	int ret;
	struct key_priv *priv;
	struct device *dev = &pdev->dev;

	dev_info(dev, "lothars_probe() - called\n");

	/* allocate device structure */
	dev_info(dev, "lothars_probe() - allocate device structure\n");
//	priv = devm_kzalloc(dev, sizeof(struct key_priv), GFP_KERNEL); // TODO rm   
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	priv->dev = dev;
//	platform_set_drvdata(pdev, priv);  
	dev_set_drvdata(dev, priv);  

	/* init the wait queue head */
	dev_info(dev, "lothars_probe() - init the wait queue head\n");
	init_waitqueue_head(&priv->wq_data_available);

	/* get virtual int number from device tree using two different methods */
	dev_info(dev, "lothars_probe() - get virtual int number from device\n");

/*	// first method
	priv->gpio = devm_gpiod_get(dev, NULL, GPIOD_IN);
	if (IS_ERR(priv->gpio)) {
		dev_err(dev, "lothars_probe() - gpio get failed\n");
		return PTR_ERR(priv->gpio);
	}

	priv->irq = gpiod_to_irq(priv->gpio);
	if (0 > priv->irq)
		return priv->irq;
	dev_info(dev, "lothars_probe() - irq number is '%d' by gpiod_to_irq\n",
		 priv->irq);

/*/	// second method
	priv->irq = platform_get_irq(pdev, 0);
	if (0 > priv->irq) {
		dev_err(dev, "lothars_probe() - irq is not available\n");
		return priv->irq;
	}
	dev_info(dev, "lothars_probe() - irq number '%d' by platform_get_irq()\n",
		 priv->irq);
// */

	ret = devm_request_irq(dev, priv->irq, lothars_key_isr,
			       IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			       LOTHARS_KEY_NAME, priv);
	if (ret) {
		dev_err(dev, "lothars_probe() - failed to request interrupt %d, error %d\n",
			priv->irq, ret);
		return ret;
	}

	priv->int_miscdevice.name = "lothars_device";
	priv->int_miscdevice.minor = MISC_DYNAMIC_MINOR;
	priv->int_miscdevice.fops = &lothars_fops;

	ret = misc_register(&priv->int_miscdevice);
	if (0 != ret) {
		dev_err(dev, "lothars_probe() - could not register the misc device lothars_device\n");
		return ret;
	}

	dev_info(dev, "lothars_probe() - done\n");

	return 0;
}

//static int __exit lothars_remove(struct platform_device* pdev)
static int
lothars_remove(struct platform_device* pdev)
//static void lothars_remove(struct platform_device* pdev) // TODO try w/o __exit and probably return void
{
	struct key_priv *priv = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "lothars_remove() - called\n");
	misc_deregister(&priv->int_miscdevice);

	return 0; // TODO rm?
}

static const struct of_device_id lothars_of_ids[] = {
	{ .compatible = "lothars,intkeywait", },
	{ },
};
MODULE_DEVICE_TABLE(of, lothars_of_ids);

static struct platform_driver lothars_platform_driver = {
	.probe = lothars_probe,
	.remove = lothars_remove,
	.driver = {
		.name = "intkeywait",
		.of_match_table = lothars_of_ids,
//		.owner = THIS_MODULE, // TODO really still needed?    
	},
};
module_platform_driver(lothars_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("sleeping and awakening in the kernel");
