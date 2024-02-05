# Device Node by Module

Demonstrates a character device and device file setup with a class instance. In this example the kernel module creates the file `/dev/lothars_hello_device`.  

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ sudo insmod hellofile.ko

$ ll /dev | grep lothar
    crw-------   1 root root      244,   0 Jan 26 22:51 lothars_hello_device

$ sudo rmmod hellofile

$ dmesg | tail
    Jan 26 22:51:09 debian kernel: init_hello_devicefile() initializing
    Jan 26 22:51:09 debian kernel: init_hello_devicefile() major = 244, minor = 0
    Jan 26 22:51:09 debian kernel: init_hello_devicefile() done.
    Jan 26 22:51:26 debian kernel: cleanup_hello_devicefile() READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
