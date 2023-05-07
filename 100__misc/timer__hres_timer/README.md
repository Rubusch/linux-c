# Timer Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod timer.ko

$ sudo rmmod timer


$ dmesg | tail
    Feb  2 00:27:32 debian kernel: init_hello() - initializing...
    Feb  2 00:27:37 debian kernel: timer_callback() - 0
    Feb  2 00:27:42 debian kernel: timer_callback() - 1
    Feb  2 00:27:47 debian kernel: timer_callback() - 2
    Feb  2 00:27:52 debian kernel: timer_callback() - 3
    Feb  2 00:27:53 debian kernel: cleanup_hello() - READY.
```


## Notes

Demonstrates the use of high resolution timer (nano secs).  

---

## References:

 * Linux Kernel API
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
