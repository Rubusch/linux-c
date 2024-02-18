// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>

#define PROBED_MODULE_NAME "lothars-pwm-dummy"

struct pwm_dummy_chip {
	struct pwm_chip chip;
	int foo;
	int bar;
	/* client structure goes here (SPI/I2C) */
};

/*
  Tool: a wrapper around container_of() to fetch member struct
*/
static inline struct pwm_dummy_chip *to_pwm_dummy_chip(struct pwm_chip *chip)
{
	return container_of(chip, struct pwm_dummy_chip, chip);
}


static int pwmdrv_request(struct pwm_chip *chip, struct pwm_device *pwm)
{
	pr_info("%s(): called\n", __func__);
	/*
	  one may need to do some initialization when a PWM channel
	  of the controller is requested. This should be done here.

	  one may do something like:
	      prepare_pwm_device(struct pwm_chip *chip, pwm->hwpwm);
	 */
	return 0;
}

/*
  From the pwm.h (kernel):

  If a pwm_state is set, the PWM driver is only required to maintain
  the power output but has more freedom regarding signal form. If
  supported, the signal can be optimized, for example to improve EMI
  by phase shifting individual channels.

  struct pwm_state {
	u64 period;
	u64 duty_cycle;
	enum pwm_polarity polarity;
	bool enabled;
	bool usage_power;
  };

  Also enable / disable via pwm_state.
 */
static int pwmdrv_apply_config(struct pwm_chip *chip, struct pwm_device *pwm,
			       const struct pwm_state *state)
{
	pr_info("%s(): called\n", __func__);
	/*
	 In this function, one ne can do something like:
	     struct pwm_dummy_chip *priv = to_pwm_dummy_chip(chip);
	     return send_command_to_set_config(priv, duty_ns, period_ns);
	 */
	return 0;
}

/*
  pwm controller ops
 */
static const struct pwm_ops pwmdrv_ops = {
	.request = pwmdrv_request,
	.apply = pwmdrv_apply_config,
};

static int pdrv_probe(struct platform_device *pdev)
{
	struct pwm_dummy_chip *priv;

	pr_info("%s(): called\n", __func__);
	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		pr_err("%s(): failed to alloc device memory\n", __func__);
		return -ENOMEM;
	}

	priv->chip.ops = &pwmdrv_ops;
	priv->chip.dev = &pdev->dev;
	priv->chip.base = -1; // dynamic base
	priv->chip.npwm = 3;  // 3 channel controller
	platform_set_drvdata(pdev, priv);

	return pwmchip_add(&priv->chip);
}

static int pdrv_remove(struct platform_device *pdev)
{
	struct pwm_dummy_chip *priv;

	pr_info("%s(): called\n", __func__);
	priv = platform_get_drvdata(pdev);
	pwmchip_remove(&priv->chip);
	return 0;
}

static const struct of_device_id pwmdrv_ids[] = {
	{ .compatible = "lothars,pwm-dummy", },
	{ }
};
MODULE_DEVICE_TABLE(of, pwmdrv_ids);

static struct platform_driver pdrv_driver = {
	.probe = pdrv_probe,
	.remove = pdrv_remove,
	.driver = {
		.name = PROBED_MODULE_NAME,
		.of_match_table = of_match_ptr(pwmdrv_ids),
	},
};
module_platform_driver(pdrv_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("A starter instead of a DT binding.");
