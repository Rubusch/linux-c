# Hello Sysfs Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod ./hellosysfs.ko

$ sudo cat /sys/kernel/lothars_sysfs/hello_sysfs_value
    0

$ echo 7 | sudo tee -a /sys/kernel/lothars_sysfs/hello_sysfs_value
    7

$ sudo cat /sys/kernel/lothars_sysfs/hello_sysfs_value
    7

$ sudo rmmod hellosysfs

$ dmesg | tail
    Jan 26 11:02:56 debian kernel: init_hello_sysfs() initializing...
    Jan 26 11:03:00 debian kernel: sysfs_show(000000001c4ef222, 000000003ba4d8b1, '0
    Jan 26 11:03:00 debian kernel: ') - Read!
    Jan 26 11:03:04 debian kernel: sysfs_store(000000001c4ef222, 000000003ba4d8b1, '7
    Jan 26 11:03:04 debian kernel: ', 2) - Write!
    Jan 26 11:03:06 debian kernel: sysfs_show(000000001c4ef222, 000000003ba4d8b1, '7
    Jan 26 11:03:06 debian kernel: ') - Read!
    Jan 26 11:03:10 debian kernel: cleanup_hello_sysfs() READY.

```

## Notes

The heart of the sysfs model is the kobject. Kobject is the glue that binds the sysfs and the kernel, which is represented by struct kobject and defined in <linux/kobject.h>. A struct kobject represents a kernel object, maybe a device or so, such as the things that show up as directory in the sysfs filesystem.  

Some of the important fields are:  

struct kobject  
 * **name** (Name of the kobject. Current kobject is created with this name in sysfs.)
 * **parent** (This is kobject’s parent. When we create a directory in sysfs for current kobject, it will create under this parent directory)
 * **ktype** (the type associated with a kobject)
 * **kset** (a group of kobjects all of which are embedded in structures of the same type)
 * **sd** (points to a sysfs_dirent structure that represents this kobject in sysfs.)
 * **kref** (provides reference counting)

When creating a sysfs entry  
 * First create a sysfs directory
 * Second create a sysfs file

(Notes taken from https://embetronicx.com/tutorials/linux/device-drivers/sysfs-in-linux-kernel/)


---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
