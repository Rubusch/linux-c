// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>      /* For platform devices */
#include <linux/interrupt.h>            /* For IRQ */
#include <linux/of.h>                   /* For DT*/
#include <linux/err.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#define PROBED_MODULE_NAME "lothars-regulator-dummy"
#define DUMMY_VOLTAGE_MIN    850000
#define DUMMY_VOLTAGE_MAX    1600000
#define DUMMY_VOLTAGE_STEP   50000

struct my_private_data {
	int aaa;
	int bbb;
	struct mutex lock;
};

static const struct of_device_id reg_ids[] = {
	{ .compatible = "lothars,regulator-dummy", },
	{ },
};

static struct regulator_init_data reg_initdata[] = {
	[0] = {
		.constraints = {
			.always_on = 0,
			.min_uV = DUMMY_VOLTAGE_MIN,
			.max_uV = DUMMY_VOLTAGE_MAX,
		},
	},
	[1] = {
		.constraints = {
			.always_on = 1,
		},
	},
};

static int isl6271a_get_voltage_sel(struct regulator_dev *dev)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static int isl6271a_set_voltage_sel(struct regulator_dev *dev,
				    unsigned selector)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}


static struct regulator_ops reg_fixed_ops = {
	.list_voltage   = regulator_list_voltage_linear,
};


static struct regulator_ops reg_core_ops = {
	.get_voltage_sel = isl6271a_get_voltage_sel,
	.set_voltage_sel = isl6271a_set_voltage_sel,
	.list_voltage   = regulator_list_voltage_linear,
	.map_voltage    = regulator_map_voltage_linear,
};


static const struct regulator_desc regdrv_desc[] = {
	{
		.name       = "Regulator Core",
		.id     = 0,
		.n_voltages = 16,
		.ops        = &reg_core_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
		.min_uV     = DUMMY_VOLTAGE_MIN,
		.uV_step    = DUMMY_VOLTAGE_STEP,
	}, {
		.name       = "Regulator Fixed",
		.id     = 1,
		.n_voltages = 1,
		.ops        = &reg_fixed_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
		.min_uV     = 1300000,
	},
};

static int pdrv_probe (struct platform_device *pdev)
{
	struct regulator_config config = { };
	struct regulator_dev *dummy_regulator_rdev[2];
	int ret, i;

	pr_info("%s(): called\n", __func__);
	config.dev = &pdev->dev;

	for (i = 0; i < 2; i++){
		config.init_data = &reg_initdata[i];
//		dummy_regulator_rdev[i] = regulator_register(&regdrv_desc[i],
		dummy_regulator_rdev[i] = devm_regulator_register(&pdev->dev,
								  &regdrv_desc[i],
								  &config);
		if (IS_ERR(dummy_regulator_rdev)) {
			ret = PTR_ERR(dummy_regulator_rdev);
			pr_err("%s(): failed to register regulator: %d\n",
			       __func__, ret);
			return ret;
		}
	}

	platform_set_drvdata(pdev, dummy_regulator_rdev);
	return 0;
}

static int pdrv_remove(struct platform_device *pdev)
{
//	int i;
//	struct regulator_dev *dummy_regulator_rdev = platform_get_drvdata(pdev);

	pr_info("%s(): called\n", __func__);
//	for (i = 0; i < 2; i++)
//		regulator_unregister(&dummy_regulator_rdev[i]);
	return 0;
}

static struct platform_driver pdrv_driver = {
	.probe      = pdrv_probe,
	.remove     = pdrv_remove,
	.driver     = {
		.name     = PROBED_MODULE_NAME,
		.of_match_table = of_match_ptr(reg_ids),
	},
};
module_platform_driver(pdrv_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Setting up a dummy regulator implementation.");
