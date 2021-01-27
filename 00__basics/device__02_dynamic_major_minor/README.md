# Device Numbers

The loadable kernel module, tested on 5.4.75.  

## Usage

```
$ make
$ sudo insmod devnum.ko

$ sudo rmmod devnum

$ dmesg | tail
    Jan 27 18:32:04 debian kernel: init_devnum() initializing...!
    Jan 27 18:32:04 debian kernel: init_devnum() major = 244, minor = 123
    Jan 27 18:32:07 debian kernel: cleanup_devnum() READY.
```

NB: The minor number 123 was provided.  


## Notes

The demo shows the dynamic configuration of major and minor device numbers. There won't be a device file or proc entry, since the device ist just allocated in the kernel and not mapped to anywhere.   

Dynamical assignment of major numbers is preferable, a range of minor numbers can be provided.  

---

## References:
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
