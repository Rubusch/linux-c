// helloflashingkeyboard.c
/*
  flashes the LEDs of a keyboard!

  this needs the KDSETLED to be set - didn't work with a 
  2.6.18 kernel!!!!


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
//*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("does something usefull =)");

/*
  defines / macros
//*/

#define BLINK_DELAY HZ / 5
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF

/*
  prototypes
//*/

static void my_timer_func(unsigned long);
static int __init kbleds_init(void);
static int __init kbleds_init(void);

/*
  globals
//*/

struct timer_list My_timer;
struct tty_driver *My_driver;
char Kbledstatus = 0;

/*
  functions
//*/

/*
  function my_timer_func blinks the keyboard LEDs periodically 
  by invoking command KDSETLED of ioctl() on the keyboard driver. 
  To learn more on virtual terminal ioctl operations, please 
  see file:

  /usr/src/linux/drivers/char/vt_ioctl.c, function vt_ioctl()

  the argument to KDSETLED is alternatively set to 7 (thus causing 
  the led mode to be set to LED_SHOW_IOCTL, and all the leds are 
  lit) and to 0xFF (any value above 7 switches back the led mode 
  to LED_SHOW_FLAGS, thus the LEDs reflect the actual keyboard 
  status). To learn more on this, please see file:

  /usr/src/linux/drivers/char/keyboard.c, function setledstate()
//*/
static void my_timer_func(unsigned long ptr)
{
	int *pstatus = (int *)ptr;

	if (*pstatus == ALL_LEDS_ON) {
		*pstatus = RESTORE_LEDS;
	} else {
		*pstatus = ALL_LEDS_ON;
		(My_driver->ioctl)(vc_cons[fg_console].d->vc_tty, NULL,
				   KDSETLED, *pstatus);
		My_timer.expieres = jiffies + BLINK_DELAY;
		add_timer(&My_timer);
	}
}

/*
  Linux init & cleanup stuff
//*/

static int __init kbleds_init(void)
{
	int idx = 0;
	printk(KERN_INFO "kbleds: loading\n");
	printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
	for (idx = 0; idx < MAX_NR_CONSOLES; ++idx) {
		if (!vc_cons[idx].d) {
			break;
		}
		printk(KERN_INFO "poet_atkm: console[%i / %i] #%i, tty %lx\n",
		       idx, MAX_NR_CONSOLES, vc_cons[idx].d->vc_num,
		       (unsigned long)vc_cons[idx].d->vc_tty);
	}
	printk(KERN_INFO "kbleds: finished scanning consoles\n");
	My_driver = vc_cons[fg_console].d->vc_tty->driver;
	printk(KERN_INFO "kbleds: tty driver magic %x\n", My_driver->magic);

	// set up the LED blink timer the first time
	init_timer(&My_timer);
	My_timer.function = my_timer_func;
	My_timer.data = (unsigned long)&kbledstatus;
	My_timer.expires = jiffies + BLINK_DELAY;
	add_timer(&My_timer);

	return 0;
}

static void __exit kbleds_cleanup(void)
{
	printk(KERN_INFO "kbleds: unloading...\n");
	del_timer(&My_timer);
	(My_driver->ioctl)(vc_cons[fg_console].d->vc_tty, NULL, KDSETLED,
			   RESTORE_LEDS);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);
