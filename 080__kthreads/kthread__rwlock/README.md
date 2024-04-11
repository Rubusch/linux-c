# Hello Kernelthread Module - Spinlocks

The demo shows elementary usage of spinlocks.  

If the kernel is running on a uniprocessor and CONFIG_SMP, CONFIG_PREEMPT aren’t enabled while compiling the kernel then spinlock will not be available. Because there is no reason to have a lock when no one else can run at the same time.  

#### Working of Read Write Spinlock

 * When there is no thread in the critical section, any reader or writer thread can enter into a critical section by taking respective read or write lock. But only one thread can enter into a critical section.  
 * If the reader thread is in the critical section, the new reader thread can enter arbitrarily, but the writer thread cannot enter. The writer thread has to wait until all the reader thread finishes their process.
 * If the writer thread is in the critical section, no reader thread or writer thread can enter.
 * If one or more reader threads are in the critical section by taking its lock, the writer thread can of course not enter the critical section, but the writer thread cannot prevent the entry of the subsequent read thread. He has to wait until the critical section has a reader thread. So this read-write spinlock is giving importance to the reader thread and not the writer thread. If you want to give importance to the writer thread than reader thread, then another lock is available in Linux which is seqlock.

#### Where to use Read Write Spinlock?

 * If you are only reading the data then you take read lock
 * If you are writing then go for a write lock

NB: Many people can hold a read lock, but a writer must be the sole holder.  

#### Locking between User Contexts, Locking between Bottom Halves

 * ``read_lock(rwlock_t *lock)``
 * ``read_unlock(rwlock_t *lock)``
 * ``write_lock(rwlock_t *lock)``
 * ``write_unlock(rwlock_t *lock)``

#### Locking between User Contexts and Bottom Halves

 * ``read_lock_bh(rwlock_t *lock)``
 * ``read_unlock_bh(rwlock_t *lock)``
 * ``write_lock_bh(rwlock_t *lock)``
 * ``write_unlock_bh(rwlock_t *lock)``

#### Locking between Hard IRQ and Bottom Halves, Locking between Hard IRQs

 * ``read_lock_irq(rwlock_t *lock)``
 * ``read_unlock_irq(rwlock_t *lock)``
 * ``write_lock_irq(rwlock_t *lock)``
 * ``write_unlock_irq(rwlock_t *lock)``
 * ``read_lock_irqsave(rwlock_t *lock, unsigned long flags)``
 * ``read_unlock_irqrestore(rwlock_t *lock, unsigned long flags)``
 * ``write_lock_irqsave(rwlock_t *lock, unsigned long flags)``
 * ``write_unlock_irqrestore(rwlock_t *lock, unsigned long flags)``

## Usage

```
$ make

$ sudo insmod kthread.ko

$ sudo rmmod kthread

$ dmesg | tail
    Jan 31 19:05:55 debian kernel: init_hello_kernelthread() started
    Jan 31 19:05:55 debian kernel: thread1: counter++
    Jan 31 19:05:55 debian kernel: init_hello_kernelthread() kernelthread initialized
    Jan 31 19:05:55 debian kernel: thread2: read counter = 1
    Jan 31 19:05:56 debian kernel: thread1: counter++
    Jan 31 19:05:56 debian kernel: thread2: read counter = 2
    Jan 31 19:05:57 debian kernel: thread1: counter++
    Jan 31 19:05:57 debian kernel: thread2: read counter = 3
    Jan 31 19:05:58 debian kernel: thread1: counter++
    Jan 31 19:05:58 debian kernel: thread2: read counter = 4
    Jan 31 19:05:59 debian kernel: cleanup_hello_kernelthread() READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
