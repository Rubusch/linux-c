# Hello File Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod ./hellochardev.ko

$ sudo ./tester.exe
    READ: '' [0]
    WRITING '123' [123]
    READ: '123' [123]
    READY.

$ sudo rmmod hellochardev

$ dmesg | tail
    Jan 27 22:48:48 debian kernel: init_hello_chardev() initializing
    Jan 27 22:48:48 debian kernel: init_hello_chardev() major = 244, minor = 3
    Jan 27 22:48:48 debian kernel: init_hello_chardev() done.

    Jan 27 22:48:55 debian kernel: hello_open()
    Jan 27 22:48:55 debian kernel: hello_read()
    Jan 27 22:48:55 debian kernel: hello_write()
    Jan 27 22:48:55 debian kernel: hello_read()
    Jan 27 22:48:55 debian kernel: hello_release()

    Jan 27 22:49:03 debian kernel: cleanup_hello_chardev() READY.
```


## Notes

Demonstrates a character device and device file setup.  

---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
