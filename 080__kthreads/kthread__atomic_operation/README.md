# Hello Atomic Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod helloatomic.ko

$ sudo rmmod helloatomic

$ dmesg | tail
    Jan 25 18:02:30 debian kernel: initializing..
    Jan 25 18:02:30 debian kernel: Thread1 created
    Jan 25 18:02:30 debian kernel: Thread1 [value: 1] [bit: 0]
    Jan 25 18:02:30 debian kernel: Thread2 created
    Jan 25 18:02:30 debian kernel: Thread2 [value: 2] [bit: 1]
    Jan 25 18:02:31 debian kernel: Thread1 [value: 3] [bit: 0]
    Jan 25 18:02:31 debian kernel: Thread2 [value: 4] [bit: 1]
    Jan 25 18:02:32 debian kernel: Thread1 [value: 5] [bit: 0]
    Jan 25 18:02:32 debian kernel: Thread2 [value: 6] [bit: 1]
    Jan 25 18:02:33 debian kernel: Thread2 [value: 7] [bit: 0]
    Jan 25 18:02:33 debian kernel: Thread1 [value: 8] [bit: 1]
    Jan 25 18:02:34 debian kernel: Thread2 [value: 9] [bit: 0]
    Jan 25 18:02:34 debian kernel: Thread1 [value: 10] [bit: 1]
    Jan 25 18:02:35 debian kernel: Thread1 [value: 11] [bit: 0]
    Jan 25 18:02:35 debian kernel: Thread2 [value: 12] [bit: 1]
    Jan 25 18:02:37 debian kernel: Thread1 [value: 13] [bit: 0]
    Jan 25 18:02:37 debian kernel: Thread2 [value: 14] [bit: 1]
    Jan 25 18:02:38 debian kernel: Thread2 [value: 15] [bit: 0]
    Jan 25 18:02:39 debian kernel: READY.
```


## Notes

Demonstrates to kthreads dealing with very basic atomic operations.  


---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
