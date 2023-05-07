# Hello File Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod ./hellofile.ko

$ sudo su
# echo 1 > /dev/lothars_hello_device
# cat /dev/lothars_hello_device
# exit

$ sudo rmmod hellofile

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

NB: make sure not to print function arguments of the device functions if they are not set, this can make the entire PC hang!  

## Notes

Demonstrates a character device and device file setup. The demo should show in the logs the startup of the chardev driver, when the `/dev` file is used, e.g. through `cat` or through `echo 1`, in the current situation the system might freeze (wrong memory region? binary write needed instead of characters?).  


---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
