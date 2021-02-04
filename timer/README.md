# Timer Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod timer.ko

$ sudo rmmod timer


$ dmesg | tail
    Feb  1 23:54:32 debian kernel: init_hello() - initializing...
    Feb  1 23:54:33 debian kernel: timer_callback() - 0
    Feb  1 23:54:33 debian kernel: timer_callback() - 1
    Feb  1 23:54:34 debian kernel: timer_callback() - 2
    Feb  1 23:54:34 debian kernel: timer_callback() - 3
    Feb  1 23:54:35 debian kernel: timer_callback() - 4
    Feb  1 23:54:35 debian kernel: timer_callback() - 5
    Feb  1 23:54:36 debian kernel: timer_callback() - 6

    Feb  1 23:54:36 debian kernel: cleanup_hello() - READY.
```


## Notes

Demonstrates the use of a kernel timer.  

---

## References:

 * Linux Kernel API
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
