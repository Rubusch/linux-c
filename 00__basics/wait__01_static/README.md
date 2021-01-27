# Event triggered Wait - Static Implementation

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod ./waitqueue.ko

$ sudo cat /dev/lothars_hello_device

$ sudo rmmod waitqueue

$ dmesg | tail

    Jan 27 23:56:24 debian kernel: init_hello_chardev() initializing
    Jan 27 23:56:24 debian kernel: init_hello_chardev() major = 244, minor = 123
    Jan 27 23:56:24 debian kernel: thread created
    Jan 27 23:56:24 debian kernel: init_hello_chardev() done.
    Jan 27 23:56:24 debian kernel: waiting for event...

// trigger: 'cat /dev/lothars_hello_device' event..
    Jan 27 23:56:46 debian kernel: hello_open()
    Jan 27 23:56:46 debian kernel: hello_read()
    Jan 27 23:56:46 debian kernel: hello_release()
    Jan 27 23:56:46 debian kernel: event came from READ - read count: 1
    Jan 27 23:56:46 debian kernel: waiting for event...

// trigger: exit event through 'rmmod'
    Jan 27 23:57:00 debian kernel: event came from EXIT
    Jan 27 23:57:00 debian kernel: cleanup_hello_chardev() READY.
```

NB: make sure not to print function arguments of the device functions if they are not set, this can make the entire PC hang!  

## Notes

Demonstrates a character device and a wait on reading event. This implementation shows the static approach.  

---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
