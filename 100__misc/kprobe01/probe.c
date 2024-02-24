/*
  kprobe: backtrace

  according to kernel sources demo

  # insmod ./probe.ko symbol=kernel_clone

  in /var/log/messages
  Feb 24 16:47:43 ctrl001 kernel: [ 1232.914237] probe: loading out-of-tree module taints kernel.
  Feb 24 16:48:24 ctrl001 kernel: [ 1274.213182] kprobe_init: XXX planted kprobe at 00000000353985b6
  Feb 24 16:48:24 ctrl001 kernel: [ 1274.214735] handler_pre: XXX <kernel_clone> p->addr = 0x00000000353985b6, pc = 0xffffffd9f988cb98, pstate = 0x80000005
  Feb 24 16:48:24 ctrl001 kernel: [ 1274.214769] handler_post: XXX <kernel_clone> p->addr = 0x00000000353985b6, pstate = 0x80000005

  REFERENCE:
  https://elixir.bootlin.com/linux/latest/source/samples/kprobes/kprobe_example.c
*/

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define MAX_SYMBOL_LEN 64
static char symbol[MAX_SYMBOL_LEN] = "_do_fork";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

/**
   alloc a probe structure for each probe
*/
static struct kprobe kp = {
	.symbol_name = symbol,
};

/**
   kprobe pre_handler

   called just before the probed instruction is executed
*/
static int __kprobes
handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	// arm64
	pr_info("XXX <%s> p->addr = 0x%p, pc = 0x%lx, pstate = 0x%lx\n",
		p->symbol_name, p->addr, (long)regs->pc, (long)regs->pstate);
	/*
	  a dump stack here will give a stack backtrace
	*/
	return 0;
}

/**
   kprobe post_handler

   called after the probed instruction is executed
*/
static void __kprobes
handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	pr_info("XXX <%s> p->addr = 0x%p, pstate = 0x%lx\n",
		p->symbol_name, p->addr, (long)regs->pstate);
}

static int __init
kprobe_init(void)
{
	int ret;
	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;

	ret = register_kprobe(&kp);
	if (0 > ret) {
		pr_err("XXX !!register_kprobe failed, returned %d!!\n", ret);
	} else {
		pr_info("XXX planted kprobe at %p\n", kp.addr);
		ret = 0;
	}
	return ret;
}

static void __exit
kprobe_exit(void)
{
	unregister_kprobe(&kp);
	pr_info("XXX kprobe at %p unregistered\n", kp.addr);
}

module_init(kprobe_init)
module_exit(kprobe_exit)

MODULE_AUTHOR("Lothar Rubusch");
MODULE_DESCRIPTION("kprobe: backtrace");
MODULE_LICENSE("GPL");
