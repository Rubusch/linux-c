// SPDX-License-Identifier: GPL-2.0+
/*
  Note: tasklets are actually deprecated nowadays.

  REFERENCES
  - Linux Device Driver Programming, J. Madieu, 2022
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>    /* tasklets api */

static char tasklet_data[] = "The red brown fox jumps over...";

static void
tasklet_function(struct tasklet_struct *unused)
{
    pr_info("%s(): called\n", __func__);
    pr_info("%s(): %s\n", __func__, tasklet_data);
    return;
}

DECLARE_TASKLET(lothars_tasklet, tasklet_function);

static int __init mod_init(void)
{
    pr_info("%s(): called\n", __func__);
    tasklet_schedule(&lothars_tasklet); // schedule the handler
    return 0;
}

void __exit mod_exit(void)
{
    pr_info("%s(): called\n", __func__);
    tasklet_kill(&lothars_tasklet);
    return;
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with tasklets.");
