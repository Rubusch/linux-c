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
$ dmesg | tail
    Jan 27 22:33:23 debian kernel: init_hello_devicefile() initializing
    Jan 27 22:33:23 debian kernel: init_hello_devicefile() major = 244, minor = 123
    Jan 27 22:33:23 debian kernel: init_hello_devicefile() done.

// write through 'echo'
    Jan 27 22:35:51 debian kernel: hello_open()
    Jan 27 22:35:51 debian kernel: hello_write()
    Jan 27 22:35:51 debian kernel: hello_release()

// read through 'cat'
    Jan 27 22:35:59 debian kernel: hello_open()
    Jan 27 22:35:59 debian kernel: hello_read()
    Jan 27 22:35:59 debian kernel: hello_release()

    Jan 27 22:36:06 debian kernel: cleanup_hello_devicefile() READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
