/*
  kprobe: backtrace

  # insmod probe symbol="kernel_clone"

  $ tail -f /var/log/messages
  Feb 24 17:45:49 ctrl001 kernel: [ 4719.233296] kprobe_init: XXX planted kprobe at 00000000353985b6
  Feb 24 17:45:49 ctrl001 kernel: [ 4719.235142] handler_pre: XXX <kernel_clone> p->addr = 0x00000000353985b6, pc = 0xffffffd9f988cb98, pstate = 0x80000005
  Feb 24 17:45:49 ctrl001 kernel: [ 4719.235188] inst_generic_make_request: XXX regs[0]: ffffffc008743d68, regs[1]: 00000000, regs[2]: 00000000, regs[3]: 00000000... (dump further register values here)
  Feb 24 17:45:49 ctrl001 kernel: [ 4719.235212] handler_post: XXX <kernel_clone> p->addr = 0x00000000353985b6, pstate = 0x80000005
  Feb 24 17:45:51 ctrl001 kernel: [ 4720.865706] handler_pre: XXX <kernel_clone> p->addr = 0x00000000353985b6, pc = 0xffffffd9f988cb98, pstate = 0x40000005
  Feb 24 17:45:51 ctrl001 kernel: [ 4720.865746] inst_generic_make_request: XXX regs[0]: ffffffc00802bdc8, regs[1]: ffffff8007d61480, regs[2]: 00000011, regs[3]: 00800700... (dump further register values here)
  Feb 24 17:45:51 ctrl001 kernel: [ 4720.865770] handler_post: XXX <kernel_clone> p->addr = 0x00000000353985b6, pstate = 0x40000005
  ...


  REFERENCE:
  https://elixir.bootlin.com/linux/latest/source/samples/kprobes/kprobe_example.c
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define MAX_SYMBOL_LEN 64
static char symbol[MAX_SYMBOL_LEN] = "kernel_clone";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

/**
  alloc a probe structure for each probe
*/
static struct kprobe kp = {
	.symbol_name = symbol,
};

// TODO apply & use
static void
inst_generic_make_request(struct kprobe *p, struct pt_regs *regs)
{
// x86
//	pr_info("XXX eax: %08lx, ebx: %08lx, ecx: %08lx, edx: %08lx\n",
//			regs->ax, regs->bx, regs->cx, regs->dx);
//
//	pr_info("XXX esi: %08lx, edi: %08lx, ebp: %08lx, esp: %08lx\n",
//			regs->si, regs->di, regs->bp, regs->sp);

// arm64
	pr_info("XXX regs[0]: %08llx, regs[1]: %08llx, regs[2]: %08llx, regs[3]: %08llx... (dump further register values here)\n",
			regs->regs[0], regs->regs[1], regs->regs[2], regs->regs[3]);
}

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

	inst_generic_make_request(p, regs);

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
