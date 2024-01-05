# Chardev Module Using devtmpfs

Device files are created by the kernel via the temporary devtmpfs
filesystem. Any driver that wishes to register a device node will use
the devtmpfs (via the core driver) to do it. When a devtmpfs instance
is mounted on /dev, the device node will initially be created with a
fixed name, permissions and owner. These entries can be both read from
and written to. All device nodes are owned by root and have the
default mode of 0600.  

Shortly afterward, the kernel will send an uevent to udevd. Based on
the rules specified in the files within the `/etc/udev/rules.d`,
`/lib/udev/rules.d` and `/run/udev/rules.d` directories, udevd will
create additional symlinks to the device node, or change its
permissions, owner, or group, or modify the internal udevd database
entry (name) for that object. The rules in these three directories are
numbered and all three directories are merged together. If udevd can't
find a rule for the device it is creating, it will leave the
permissions and ownership at whatever devtmpfs used initially.  

Kernel configuration: CONFIG_DEVTMPFS_MOUNT.  

## Module
Should compile cross - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace
Cross compile the source, setting CROSS_COMPILE as for the kernel.  
```
rpi$ cd ./userspace
rpi$ make
```
Then copy the .elf file over to the target.  

## Usage
```
$ sudo insmod ./chardev.ko

$ sudo ./ioctl_test.elf

$ ls -l /sys/class/lothars_class/lothars_device
    lrwxrwxrwx 1 root root 0 Oct  3 15:41 /sys/class/lothars_class/lothars_device -> ../../devices/virtual/lothars_class/lothars_device

$ sudo rmmod chardev

$ dmesg | tail
    [  424.096496] my_dev_close(): called
    [  448.599004] hello chardev exit
    [  708.336141] chardev_init(): hello chardev init
    [  708.336190] chardev_init(): allocated correctly with major number 239
    [  708.336313] chardev_init(): device class registered correctly
    [  708.336704] chardev_init(): the device is created correctly
    [  734.176532] chardev_open(): called
    [  734.176582] chardev_ioctl(): called, cmd = 100, arg = 110
    [  734.176617] chardev_close(): called
    [  801.709206] chardev_exit(): hello chardev exit
```

## Verified
* Verified against a RPI3 w/ aarch64

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
