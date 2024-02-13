// SPDX-License-Identifier: GPL-2.0+
/*
  REFERENCES
  - Linux Device Driver Programming, J. Madieu, 2022
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h> /* for work queue */
#include <linux/kmod.h>

static struct delayed_work initiate_shutdown_work;

static void delayed_shutdown(struct work_struct *work)
{
    char *cmd = "/sbin/shutdown";
    char *argv[] = { cmd, "-h", "now", NULL, };
    char *envp[] = { "HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL, };

    // calls userspace application
    call_usermodehelper(cmd, argv, envp, 0);
}

static int __init mod_init(void)
{
    INIT_DELAYED_WORK(&initiate_shutdown_work, delayed_shutdown);
    schedule_delayed_work(&initiate_shutdown_work, msecs_to_jiffies(200));
    return 0;
}

static void __exit mod_exit(void)
{
    return;
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Execute userspace application.");
