# Devicenode by Module for a Chardev

Demonstrates a character device and device file setup. A devicenode is
setup, and a chardev driver will read / write from / to it. The demo
shows in the logs the startup of the chardev driver, when the `/dev`
file is used, e.g. through `cat` or through `echo 1`  

## Usage

```
# insmod ./hello.ko

# echo 1 > /dev/lothars_hello_device
# cat /dev/lothars_hello_device

# rmmod hellofile
```

Logs  
```
[   23.918298] hello: loading out-of-tree module taints kernel.
[   23.919147] init_hello_chardev(): called
[   23.919178] init_hello_chardev(): major = 237, minor = 123
[   23.919739] init_hello_chardev(): done
[   24.100860] hello_open(): called
[   24.100985] hello_write(): called
[   24.101039] hello_release(): called
[   24.261369] hello_open(): called
[   24.261495] hello_read(): called
[   24.261549] hello_release(): called
[   24.453131] cleanup_hello_chardev(): READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
