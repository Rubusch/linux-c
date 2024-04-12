/*
  kprobe: backtrace

  # insmod probe

  $ tail -f /var/log/messages
  ...
  Feb 24 17:15:55 ctrl001 kernel: [ 2925.069932] kprobe_init(): planted kprobe at 00000000353985b6
  Feb 24 17:15:55 ctrl001 kernel: [ 2925.071728] handler_pre(): kernel_clone p->addr = 0x00000000353985b6, pc = 0xffffffd9f988cb98, pstate = 0x80000005
  Feb 24 17:15:55 ctrl001 kernel: [ 2925.071772] handler_post(): kernel_clone p->addr = 0x00000000353985b6, pstate = 0x80000005


  REFERENCE:
  https://developpaper.com/original-kprobe-on-arm64-of-kernel-debugging-and-tracking-technology/
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define MAX_SYMBOL_LEN  64
static char symbol[MAX_SYMBOL_LEN] = "kernel_clone";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

/* For each probe you need to allocate a kprobe structure */
//static struct kprobe kp = {
//        .symbol_name    = symbol,
//};

/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
        pr_info("%s(): %s p->addr = 0x%p, pc = 0x%llx, pstate = 0x%08llx\n",
                __func__, p->symbol_name, p->addr, regs->pc, regs->pstate);

        /* A dump_stack() here will give a stack backtrace */
        return 0;
}

/* kprobe post_handler: called after the probed instruction is executed */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
                                unsigned long flags)
{
        pr_info("%s(): %s p->addr = 0x%p, pstate = 0x%08llx\n",
                __func__, p->symbol_name, p->addr, regs->pstate);
}

static struct kprobe kp = {
        .symbol_name = symbol,
        .pre_handler = handler_pre,
        .post_handler = handler_post,
};

static int __init kprobe_init(void)
{
        int ret;

        ret = register_kprobe(&kp);
        if (ret < 0) {
                pr_err("%s(): register_kprobe failed, returned %d\n", __func__, ret);
                return ret;
        }
        pr_info("%s(): planted kprobe at %p\n", __func__, kp.addr);
        return 0;
}

static void __exit kprobe_exit(void)
{
        unregister_kprobe(&kp);
        pr_info("%s(): kprobe at %p unregistered\n", __func__, kp.addr);
}

module_init(kprobe_init);
module_exit(kprobe_exit);

MODULE_AUTHOR("Lothar Rubusch");
MODULE_DESCRIPTION("kprobe: backtrace");
MODULE_LICENSE("GPL");
