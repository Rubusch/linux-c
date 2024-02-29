# Userspace communication to kernel device driver

Demonstrates a character device and device file setup. An additional
userspace application will use the device node to communicate to the
kernel driver.  

## Usage

```
# insmod ./hellochardev.ko

# ./tester.elf
    READ: '###...###' [0]
    WRITING '123' [123]
    READ: '123' [123]
    READY.

# rmmod hellochardev
```
Logs  
```
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

## References:
 * Linux kernel source, comented API and documentation
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
