# Event triggered Wait - Dynamic Implementation

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod ./waitqueue.ko

$ sudo cat /dev/lothars_hello_device

$ sudo rmmod waitqueue

$ dmesg | tail
    Jan 28 00:50:13 debian kernel: init_hello_chardev() initializing
    Jan 28 00:50:13 debian kernel: init_hello_chardev() major = 244, minor = 123
    Jan 28 00:50:13 debian kernel: thread created
    Jan 28 00:50:13 debian kernel: init_hello_chardev() done.
    Jan 28 00:50:13 debian kernel: waiting for event...

    Jan 28 00:50:19 debian kernel: hello_open()
    Jan 28 00:50:19 debian kernel: hello_read()
    Jan 28 00:50:19 debian kernel: event came from READ - read count: 1
    Jan 28 00:50:19 debian kernel: waiting for event...
    Jan 28 00:50:19 debian kernel: hello_release()

    Jan 28 00:50:25 debian kernel: event came from EXIT
    Jan 28 00:50:25 debian kernel: cleanup_hello_chardev() READY.

```


## Notes

Demonstrates a character device and a wait on reading event. This implementation shows the dynamic approach.  

---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
