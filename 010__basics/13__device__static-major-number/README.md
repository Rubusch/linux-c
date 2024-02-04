# Device Numbers

The demo shows the static configuration of major and minor device numbers. There won't be a device file or proc entry, since the device ist just allocated in the kernel and not mapped to anywhere.   

Static setup of major and minor numbers is not preferable.  

The loadable kernel module, tested on 5.4.75.  

## Usage

```
$ sudo insmod hello.ko

$ sudo rmmod hello

$ dmesg | tail
    Jan 27 18:20:09 debian kernel: init_devnum() initializing...!
    Jan 27 18:20:09 debian kernel: init_devnum() major = 123, minor = 123
    Jan 27 18:21:04 debian kernel: cleanup_devnum() READY.
```

## References

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
