# Hello Sysfs Module

The heart of the sysfs model is the kobject. Kobject is the glue that
binds the sysfs and the kernel, which is represented by struct kobject
and defined in <linux/kobject.h>. A struct kobject represents a kernel
object, maybe a device or so, such as the things that show up as
directory in the sysfs filesystem.  

Some of the important fields are:  

`struct kobject`  
 * **name** (Name of the kobject. Current kobject is created with this
   name in sysfs.)
 * **parent** (This is kobject’s parent. When we create a directory in
   sysfs for current kobject, it will create under this parent
   directory)
 * **ktype** (the type associated with a kobject)
 * **kset** (a group of kobjects all of which are embedded in
   structures of the same type)
 * **sd** (points to a sysfs_dirent structure that represents this
   kobject in sysfs.)
 * **kref** (provides reference counting)

When creating a sysfs entry  
 * First create a sysfs directory
 * Second create a sysfs file

(Notes taken from https://embetronicx.com/tutorials/linux/device-drivers/sysfs-in-linux-kernel/)

## Usage

```
$ make

$ sudo insmod ./hello.ko

$ sudo cat /sys/kernel/lothars_sysfs/hello_sysfs_value
    0

$ echo 7 | sudo tee -a /sys/kernel/lothars_sysfs/hello_sysfs_value
    7

$ sudo cat /sys/kernel/lothars_sysfs/hello_sysfs_value
    7

$ sudo rmmod hello
```

Logs   
```
$ dmesg | tail
    Mar 19 19:17:17 ctrl01 kernel: [   23.405998] mod_init(): called
    Mar 19 19:17:17 ctrl01 kernel: [   23.504164] sysfs_show(00000000e64594d4, 00000000a129d74d, '0
    Mar 19 19:17:17 ctrl01 kernel: [   23.504164] ') - read!
    Mar 19 19:17:17 ctrl01 kernel: [   23.610155] sysfs_store(00000000e64594d4, 00000000a129d74d, '7
    Mar 19 19:17:17 ctrl01 kernel: [   23.610155] ', 2) - write!
    Mar 19 19:17:17 ctrl01 kernel: [   23.690580] sysfs_show(00000000e64594d4, 00000000a129d74d, '7
    Mar 19 19:17:17 ctrl01 kernel: [   23.690580] ') - read!
    Mar 19 19:17:17 ctrl01 kernel: [   23.903324] mod_exit(): called
    Mar 19 19:17:17 ctrl01 kernel: [   23.903369] mod_exit() READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
 * https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
