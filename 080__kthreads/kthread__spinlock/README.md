# Hello Kernelthread Module - Spinlocks

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod kthread.ko

$ sudo rmmod kthread

$ dmesg | tail
    Jan 31 16:14:30 debian kernel: init_hello_kernelthread() started
    Jan 31 16:14:30 debian kernel: thread1: spinlock is not locked
    Jan 31 16:14:30 debian kernel: thread1: spinlock is locked, now
    Jan 31 16:14:30 debian kernel: thread1 - counter = 1
    Jan 31 16:14:30 debian kernel: init_hello_kernelthread() kernelthread initialized
    Jan 31 16:14:30 debian kernel: thread2: spinlock is not locked
    Jan 31 16:14:30 debian kernel: thread2: spinlock is locked, now
    Jan 31 16:14:30 debian kernel: thread2 - counter = 2
    Jan 31 16:14:31 debian kernel: thread1: spinlock is not locked
    Jan 31 16:14:31 debian kernel: thread1: spinlock is locked, now
    Jan 31 16:14:31 debian kernel: thread1 - counter = 3
    Jan 31 16:14:31 debian kernel: thread2: spinlock is locked, now
    Jan 31 16:14:31 debian kernel: thread2 - counter = 4
    Jan 31 16:14:32 debian kernel: thread2: spinlock is not locked
    Jan 31 16:14:32 debian kernel: thread2: spinlock is locked, now
    Jan 31 16:14:32 debian kernel: thread2 - counter = 5
    Jan 31 16:14:32 debian kernel: thread1: spinlock is not locked
    Jan 31 16:14:32 debian kernel: thread1: spinlock is locked, now
    Jan 31 16:14:32 debian kernel: thread1 - counter = 6
    Jan 31 16:14:33 debian kernel: thread2: spinlock is not locked
    Jan 31 16:14:33 debian kernel: thread2: spinlock is locked, now
    Jan 31 16:14:33 debian kernel: thread2 - counter = 7
    Jan 31 16:14:34 debian kernel: cleanup_hello_kernelthread() READY.
```


## Notes

The demo shows elementary usage of spinlocks.  

If the kernel is running on a uniprocessor and CONFIG_SMP, CONFIG_PREEMPT aren’t enabled while compiling the kernel then spinlock will not be available. Because there is no reason to have a lock when no one else can run at the same time.  

#### Locking between User Contexts or between Bottom Halves

 * ``spin_lock(spinlock_t *lock)``
 * ``spin_trylock(spinlock_t *lock)``
 * ``spin_unlock(spinlock_t *lock)``
 * ``spin_is_locked(spinlock_t *lock)``

#### Locking between User Context and Bottom Halves

E.g. inside thread routine and tasklet function use:  

 * ``spin_lock_bh(spinlock_t *lock)``
 * ``spin_unlock_bh(spinlock_t *lock)``

#### Locking between Hard IRQ and Bottom Halves, Locking between Hard IRQs

E.g. inside irq handler and tasklet function use:  

 * ``spin_lock_irq(spinlock_t *lock)``
 * ``spin_unlock_irq(spinlock_t *lock)``
 * ``spin_lock_irqsave(spinlock_t *lock, unsigned long flags)``
 * ``spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)``


---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
