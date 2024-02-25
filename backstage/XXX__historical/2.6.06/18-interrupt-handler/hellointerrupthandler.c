// hellointerrupthandler.c
/*
  catches the keyboard input, it doesn't release the input any 
  more after, didn't compile problems with a 2.5.25 kernel

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  "static" declaration for variables and functions necessary 
  to avoid namespace conflicts with other functions by the same 
  name (in same "common" namespace!).

  init_module(void) and cleanup_module(void) implementations 
  must not have "static" return types!

  C90 conformity: declarations of variables have to be made at 
  begin of each block (a function body is a block!)

  declaration follows the C90 standard
  (C) Salzman 2001
//*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR(
	"Salzman 2001 - indentions: Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("does something usefull =)");

/*
  defines/macros
//*/

#define WORK_QUEUE_NAME "WQsched.c"

/*
  prototypes
//*/

int init_module(void);
void cleanup_module(void);

static void got_char(void *);
irqreturn_t irq_handler(int, void *, struct pt_regs *);

/*
  globals
//*/

static struct workqueue_struct *Work_queue;

/*
  functions
//*/

/*
  will get called by the kernel as soon as it's safe to do everything 
  normally allowed by kernel modules
//*/
static void got_char(void *scancode)
{
	printk(KERN_INFO "scan code %x %s.\n", (int)*((char *)scancode) & 0x7F,
	       *((char *)scancode) & 0x80 ? "Released" : "Pressed");
}

/*
  returns the IRQ_HANDLED macro
//*/
irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	// this variables are static because they need to be accessible
	// (through pointers) to the bottom half routine
	static int initialized = 0;
	static unsigned char scancode = 0;
	static struct work_struct task;
	unsigned char status = 0;

	// read keyboard status
	status = inb(0x64);
	scancode = inb(0x60);

	// won't compile in kernel > 2.6.18
	if (0 == initialized) {
		INIT_WORK(&task, got_char, &scancode);
		initialized = 1;
	} else {
		PREPARE_WORK(&task, got_char, &scancode);
	}

	queue_work(Work_queue, &task);

	return IRQ_HANDLED;
}

/*
  linux init & clean up
//*/

int init_module(void)
{
	Work_queue = create_workqueue(WORK_QUEUE_NAME);

	/*
    since the keyboard handler won't co-exist with another handler,
    such as us, we have to disable it (free its IRQ) before we do
    anything. 
    since we don't know where it is, there's no way to reinstate it 
    later - so the computer will have to be rebooted when we're done
  //*/
	free_irq(1, NULL);

	/*
    request IRQ 1, the keyboard IRQ, to go to our irq_handler
    SA_SHIRQ means we're willing to have other hanlders on this IRQ
    SA_INTERRUPT can be used to make the handler into a fast interrupt
  //*/

	return request_irq(1 // number of the keyboard IRQ on PCs
			   ,
			   irq_handler // our handler
			   ,
			   SA_SHIRQ, "test_keyboard_irq_hanlder",
			   (void *)(irq_handler));
}

void cleanup_module(void)
{
	/*
    this is only here for completeness. it's totally irrelevant, since
    we don't have a way to restore the normal keyboard interrupt so the 
    computer is completely useless and has to be rebooted
  //*/
	free_irq(1, NULL);
}
