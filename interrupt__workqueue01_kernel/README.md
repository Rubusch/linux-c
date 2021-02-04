# Hello Interrupt Module - Kernel Workqueue Demo

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod interrupt.ko

// see interrupt handler registered for interrupt 11
$ cat /proc/interrupts | grep lothar
      11:          0          0          0          0          0          0          0          0  IR-IO-APIC   11-edge      lothars_device

// start event
$ sudo cat /dev/lothars_device
    Message from syslogd@debian at Jan 28 22:53:07 ...
    kernel:do_IRQ: 2.56 No irq handler for vector

$ sudo rmmod interrupt

$ dmesg | tail
    Jan 28 22:52:55 debian kernel: init_hello_interrupt() initializing...
    Jan 28 22:52:55 debian kernel: init_hello_interrupt() major = 244, minor = 123
    Jan 28 22:53:07 debian kernel: hello_interrupt_read()
    Jan 28 22:53:07 debian kernel: do_IRQ: 2.56 No irq handler for vector
    Jan 28 22:53:37 debian kernel: cleanup_hello_interrupt() READY.

```

NB: the ``vector_irq[]`` is not available right away. Thus this part is commented out in the kernel due to compile it on the continuous integration (CI) system. In order to experiment with interrupts but without hardware, i.e. to misuse the IRQ 0x80, follow the comment from stack overflow:  

__This used to work on older kernel versions, but fails on later versions. The reason is that the generic IRQ handler ``do_IRQ()`` has been changed for better IRQ handling performance. Instead of using the irq_to_``desc()`` function to get the IRQ descriptor, it reads it from the per-CPU data. The descriptor is put there during the physical device initialization. Since this pseudo device driver don't have a physical device, ``do_IRQ()`` don't find it there and returns with an error. If we want to simulate IRQ using software interrupt, we must first write the IRQ descriptor to the per-CPU data. Unfortunately, the symbol vector_irq, the array of the IRQ descriptors in the per-CPU data, is not exported to kernel modules during kernel compilation. The only way to change it, is to recompile the whole kernel. If you think it worth the effort, you can add the line:__  

```
    EXPORT_SYMBOL (vector_irq);
```

__in the file: arch/x86/kernel/irq.c right after all the include lines.__  

https://stackoverflow.com/questions/57391628/error-while-raising-interrupt-11-with-inline-asm-into-kernel-module


## Notes

There are **4 bottom half mechanisms** are available in Linux:  

 * Workqueue
 * Threaded IRQs
 * Softirq
 * Tasklets


Work queues are added in the Linux kernel 2.6 version. Work queues are a different form of deferring work. Work queues defer work into a kernel thread; this bottom half always runs in process context. Because workqueue is allowing users to create a kernel thread and bind work to the kernel thread. So, this will run in process context and the work queue can sleep.  

Code deferred to a work queue has all the usual benefits of process context. Most importantly, work queues are schedulable and can therefore sleep.  

Normally, it is easy to **decide between using workqueue and softirq/taskl**:  

 * If the deferred work **needs to sleep, then workqueue** is used.
 * If the deferred work **need not sleep, then softirq or tasklet** are used.

There are two ways to implement Workqueue in Linux kernel.  

 * Using global workqueue (Static / Dynamic)  
 * Creating Customized Workqueues  

---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
