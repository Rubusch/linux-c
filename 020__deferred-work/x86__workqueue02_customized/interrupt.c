// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Triggering Hardware Interrupt through Software
 *
 * NB: x86 only!
 * NB: tasklets are deprecated
 *
 * Intel processors handle interrupt using IDT (Interrupt Descriptor
 * Table).  The IDT consists of 256 entries with each entry
 * corresponding to a vector and of 8 bytes. All the entries are a
 * pointer to the interrupt handling function. The CPU uses IDTR to
 * point to IDT. The relation between those two can be depicted as
 * below,
 *
 * IDTR Register
 *
 *
 *  47                               16 15              0
 * +-----------------------------------+-----------------+
 * | IDT Base Addr                     |   IDT Limit     |
 * +-----------------------------------+-----------------+
 *                    |                       |
 *                    |      +----------------+
 *                    |      |
 *                    |      |   Interrupt Descriptor Table (IDT)
 *                    |      V         +-----------------+
 *                    +---->(+)------->| Gate for        |
 *                    |                |     Interrupt n |
 *                    |                +-----------------+
 *                    |                |                 |
 *                    |               ...               ...
 *                    |                |                 |
 *                    |                +-----------------+
 *                    |                +-----------------+
 *                    |                +-----------------+
 *                    |                | Gate Irpt. #2   |
 *                    |                +-----------------+
 *                    +--------------->| Gate for        |
 *                                     |    Interrupt #1 |
 *                                     +-----------------+
 *
 * An interrupt can be programmatically raised using int
 * instruction. For example, the Linux system call was using int $0x80.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <asm/hw_irq.h>

// interrupt handler
static irqreturn_t irq_handler(int, void *);

// chardev read()
static ssize_t hello_interrupt_read(struct file *, char __user *, size_t,
				    loff_t *);

// chardev device
#define HELLO_DEVICE_MINOR 123

#define HELLO_DEVICE_CHRDEV "lothars_chrdev"
#define HELLO_DEVICE_CLASS "lothars_class"
#define HELLO_DEVICE_NAME "lothars_device"

#define HELLO_WORKQUEUE_NAME "lothars_workqueue"

// chardev
dev_t dev;
static struct class *dev_class;
static struct cdev hello_interrupt_cdev;

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = hello_interrupt_read,
};

// interrupt and workqueue
#define IRQ_NO 11
static struct workqueue_struct *lothars_workqueue;

/*
 *  implementation
 */

/*
 * chardev read()
 *
 * Simulates something similar as an interrupt from a device.
 *
 * This used to work on older kernel versions, but fails on later
 * versions. The reason is that the generic IRQ handler do_IRQ() has
 * been changed for better IRQ handling performance. Instead of using
 * the irq_to_desc() function to get the IRQ descriptor, it reads it
 * from the per-CPU data. The descriptor is put there during the
 * physical device initialization. Since this pseudo device driver
 * don't have a physical device, do_IRQ() don't find it there and
 * returns with an error. If we want to simulate IRQ using software
 * interrupt, we must first write the IRQ descriptor to the per-CPU
 * data. Unfortunately, the symbol vector_irq, the array of the IRQ
 * descriptors in the per-CPU data, is not exported to kernel modules
 * during kernel compilation. The only way to change it, is to
 * recompile the whole kernel. If you think it worth the effort, you
 * can add the line:
 *
 *   EXPORT_SYMBOL (vector_irq);
 *
 * in the file: arch/x86/kernel/irq.c right after all the include lines.
 *
 * https://stackoverflow.com/questions/57391628/error-while-raising-interrupt-11-with-inline-asm-into-kernel-module
 */
static ssize_t hello_interrupt_read(struct file *filp, char __user *buf,
				    size_t len, loff_t *off)
{
	pr_info("%s()", __func__);
	asm("int $0x38"); // corresponding to IRQ 11
	return 0;
}

/*
 * interrupt handler
 */
static irqreturn_t irq_handler(int irq, void *dev_id)
{
	pr_info("%s() shared IRQ: interrupt occurred\n", __func__);

	// allocating workqueue
	queue_work(lothars_workqueue, &workqueue);

	return IRQ_HANDLED;
}

/*
 * workqueue
 */
void workqueue_fn(struct work_struct *work)
{
	pr_info("%s()\n", __func__);
}

// macro approach
DECLARE_WORK(workqueue, workqueue_fn);

/*
 * start / stop module
 */

int init_hello_interrupt(void)
{
	pr_info("%s() initializing...\n", __func__);
	if (alloc_chrdev_region(&dev, HELLO_DEVICE_MINOR, 1,
				HELLO_DEVICE_CHRDEV) < 0) {
		pr_err("alloc_chrdev_region() failed\n");
		return -ENOMEM;
	}
	pr_info("%s() major = %d, minor = %d\n", __func__,
		MAJOR(dev), MINOR(dev));

	cdev_init(&hello_interrupt_cdev, &fops);

	if (cdev_add(&hello_interrupt_cdev, dev, 1) < 0) {
		pr_err("cdev_add() failed\n");
		goto err_cdev;
	}

	dev_class = class_create(THIS_MODULE, HELLO_DEVICE_CLASS);
	if (!dev_class) {
		pr_err("class_create() failed\n");
		goto err_class;
	}

	if (!device_create(dev_class, NULL, dev, NULL, HELLO_DEVICE_NAME)) {
		pr_err("device_create() failed\n");
		goto err_device;
	}

	/**
	 * request_irq - Add a handler for an interrupt line
	 * @irq:The interrupt line to allocate
	 * @handler:Function to be called when the IRQ occurs.
	 *Primary handler for threaded interrupts
	 *If NULL, the default primary handler is installed
	 * @flags:Handling flags
	 * @name:Name of the device generating this interrupt
	 * @dev:A cookie passed to the handler function
	 *
	 * This call allocates an interrupt and establishes a handler; see
	 * the documentation for request_threaded_irq() for details.
	 */
	if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, HELLO_DEVICE_NAME,
			(void *)(irq_handler))) {
		pr_err("request_irq() failed!\n");
		goto err_irq;
	}

	// customized workqueue
	/**
	 * create_workqueue() is a macro:
	 *
	 * #define create_workqueue(name)
	 * alloc_workqueue("%s", WQ_MEM_RECLAIM, 1, (name))
	 * #define create_singlethread_workqueue(name)
	 * alloc_workqueue("%s", WQ_UNBOUND | WQ_MEM_RECLAIM, 1, (name))
	 *
	 * important flags for workqueues are the following:
	 * WQ_UNBOUND
	 * WQ_FREEZABLE
	 * WQ_MEM_RECLAIM
	 * WQ_HIGHPRI
	 * WQ_CPU_INTENSIVE
	 */
	lothars_workqueue = create_workqueue(HELLO_WORKQUEUE_NAME);

	return 0;

err_irq:
	free_irq(IRQ_NO, (void *)(irq_handler));

err_device:
	device_destroy(dev_class, dev);

err_class:
	class_destroy(dev_class);

err_cdev:
	cdev_del(&hello_interrupt_cdev);
	unregister_chrdev_region(dev, 1);

	return -ENOMEM;
}

void cleanup_hello_interrupt(void)
{
	/**
	 * destroy_workqueue - safely terminate a workqueue
	 * @wq: target workqueue
	 *
	 * Safely destroy a workqueue. All work currently pending will be done first.
	 */
	destroy_workqueue(lothars_workqueue);

	/**
	 *free_irq - free an interrupt allocated with request_irq
	 *@irq: Interrupt line to free
	 *@dev_id: Device identity to free
	 *
	 *Remove an interrupt handler. The handler is removed and if the
	 *interrupt line is no longer in use by any driver it is disabled.
	 *On a shared IRQ the caller must ensure the interrupt is disabled
	 *on the card it drives before calling this function. The function
	 *does not return until any executing interrupts for this IRQ
	 *have completed.
	 *
	 *This function must not be called from interrupt context.
	 *
	 *Returns the devname argument passed to request_irq.
	 */
	free_irq(IRQ_NO, (void *)(irq_handler));

	// chardev
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&hello_interrupt_cdev);
	unregister_chrdev_region(dev, 1);

	pr_info("%s() READY.\n", __func__);
}

/*
 * init / exit
 */

static int __init mod_init(void)
{
	return init_hello_interrupt();
}

static void __exit mod_exit(void)
{
	cleanup_hello_interrupt();
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates the Hello Interrupt World!");
