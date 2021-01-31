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


---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
