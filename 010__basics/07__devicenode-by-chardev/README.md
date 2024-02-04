# Hello File Module

Demonstrates a character device and device file setup. Here the kernel
driver only allocates a major number, which will be printed to
log. The user must create the devicefile with mknod and the specific
major number.  

## Usage

```
$ sudo insmod ./hellofile.ko
$ dmesg | tail
    Jan 26 22:31:21 debian kernel: init_hello_devicefile() initializing
    Jan 26 22:31:21 debian kernel: init_hello_devicefile() major = 244, minor = 0
    Jan 26 22:31:21 debian kernel: init_hello_devicefile() done.

$ sudo mknod -m 666 /dev/lothars_char_dev c 244 0
$ ls -l /dev/ | grep lothar
    crw-rw-rw-   1 root root      244,   0 Jan 26 22:35 lothars_char_dev

$ sudo rmmod hellofile
$ dmesg | tail
    Jan 26 22:36:32 debian kernel: cleanup_hello_devicefile() READY.
```

## References

 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
