# Hello Kernelthread Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod ./kthread.ko

$ sudo cat /dev/lothars_chardev
$ sudo cat /dev/lothars_chardev

$ sudo rmmod ./kthread.ko

$ dmesg | tail
    Feb  2 19:21:29 debian kernel: init_hello_completion() started
    Feb  2 19:21:29 debian kernel: init_hello_completion() - major = 244, minor = 0
    Feb  2 19:21:29 debian kernel: kthread_create() successful
    Feb  2 19:21:29 debian kernel: waiting for event...
## first read attempt
    Feb  2 19:21:40 debian kernel: chardev_read()
    Feb  2 19:21:40 debian kernel: event came from READ function - 0
    Feb  2 19:21:40 debian kernel: waiting for event...
## second read attempt
    Feb  2 19:21:48 debian kernel: chardev_read()
    Feb  2 19:21:48 debian kernel: event came from READ function - 1
    Feb  2 19:21:48 debian kernel: waiting for event...

    Feb  2 19:22:02 debian kernel: event came from exit function
    Feb  2 19:22:02 debian kernel: cleanup_hello_completion() READY.
```


## Notes

The demo shows elementary usage of kthread waitqueues and completion. When we want to notify or wakeup some thread or something when we finished some work, then we can use completion.  

There are 5 important steps in Completions.  

 * Initializing Completion
 * Re-Initializing Completion
 * Waiting for completion (The code is waiting and sleeping for something to finish)
 * Waking Up Task (Sending signal to sleeping part)
 * Check the status

NB: ``wait_for_completion()`` is calling spin_lock_irq()/spin_unlock_irq(), so it can only be called safely when you know that interrupts are enabled. **Calling it from IRQs-off atomic contexts will result in hard-to-detect spurious enabling of interrupts.**  

NB: ``try_wait_for_completion()`` is safe to be called in IRQ or atomic context.  

NB: There are better mechanisms for micro benchmarking than (mis)using completions for high precision micro benchmarking.  


#### Test if there are any threads awaiting

Use ``bool completion_done (struct completion * x);`` where, x holds the state of this particular completion. It returns 0 if there are waiters (wait_for_completion in progress) 1 if there are no waiters. This completion_done() is safe to be called in IRQ or atomic context.  


#### Demo

In the demo a thread is put to wait on a completion event, the completion event (async) comes from someone reading out the chardev, or unloading the module. The scenary is as fabricated as simple. Real world usage would be passing skbuff to other workers and waiting on those worker threads. When they are done pushing the resulting skbuf back to a queue, they signal via completion their status done.  

---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by and notes taken from - www.embetronicx.com (2021)
