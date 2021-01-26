# Hello File Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod ./hellofile.ko

$ sudo rmmod hellofile

$ dmesg | tail
    Jan 26 21:10:44 debian kernel: init_hello_file() initializing
    Jan 26 21:10:44 debian kernel: init_hello_file() major = 244, minor = 100
    Jan 26 21:10:44 debian kernel: init_hello_file() done.
    Jan 26 21:10:57 debian kernel: cleanup_hello_file() READY.

```


## Notes

Demonstrates a character device and device file setup. The demo should show in the logs the startup of the chardev driver, when the `/dev` file is used, e.g. through `cat` or through `echo 1`, in the current situation the system might freeze (wrong memory region? binary write needed instead of characters?).

---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
