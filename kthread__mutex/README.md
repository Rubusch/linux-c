# Hello Kernelthread Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod kthread.ko

$ sudo rmmod kthread

$ dmesg | tail
    Jan 31 12:27:11 debian kernel: init_hello_kernelthread() started
    Jan 31 12:27:11 debian kernel: init_hello_kernelthread() kernelthread initialized
    Jan 31 12:27:11 debian kernel: thread1 - counter = 1
    Jan 31 12:27:11 debian kernel: thread2 - counter = 2
    Jan 31 12:27:12 debian kernel: thread1 - counter = 3
    Jan 31 12:27:12 debian kernel: thread2 - counter = 4
    Jan 31 12:27:13 debian kernel: thread1 - counter = 5
    Jan 31 12:27:13 debian kernel: thread2 - counter = 6
    Jan 31 12:27:14 debian kernel: thread2 - counter = 7
    Jan 25 11:56:35 debian kernel: cleanup_hello_kernelthread() READY.
```


## Notes

The demo shows elementary usage of mutex.


---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
