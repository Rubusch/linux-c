# Device Node by Module

Demonstrates a character device and device file setup with a class instance. In this example the kernel module creates the file `/dev/lothars_hello_device`.  

## Usage

```
$ sudo insmod hello.ko

$ ll /dev/ | grep hello
    crw-------  1 root root    237, 123 Feb 28 07:32 lothars_hello_device

$ sudo rmmod hello

$ dmesg | tail
    [ 1461.825850] init_hello_chardev(): initializing
    [ 1461.825988] init_hello_chardev(): major = 237, minor = 123
    [ 1461.827544] init_hello_chardev() done.
    [ 1506.942989] cleanup_hello_chardev() READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
