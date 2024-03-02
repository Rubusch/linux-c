# IOCTL Chardev Module Using devtmpfs

Device files are created by the kernel via the temporary devtmpfs
filesystem. Any driver that wishes to register a device node will use
the devtmpfs (via the core driver) to do it. When a devtmpfs instance
is mounted on /dev, the device node will initially be created with a
fixed name, permissions and owner. These entries can be both read from
and written to. All device nodes are owned by root and have the
default mode of 0600.  

Shortly afterward, the kernel will send an uevent to udevd
(daemon). Based on the rules specified in the files within the
`/etc/udev/rules.d`, `/lib/udev/rules.d` and `/run/udev/rules.d`
directories, udevd will create additional symlinks to the device node,
or change its permissions, owner, or group, or modify the internal
udevd database entry (name) for that object. The rules in these three
directories are numbered and all three directories are merged
together. If udevd can't find a rule for the device it is creating, it
will leave the permissions and ownership at whatever devtmpfs used
initially.  

Needs Kernel configuration: `CONFIG_DEVTMPFS_MOUNT`.  

## Module
Compile for a RPI, having `crossbuild-essentials-arm64` installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module
$ make
```
Copy the module and the userspace application over to the target  

## Usage
```
# insmod ./happy_ioctl.ko

# ./ioctl_test.elf

# ls -l /sys/class/lothars_class/lothars_device
    lrwxrwxrwx 1 root root 0 Oct  3 15:41 /sys/class/lothars_class/lothars_device -> ../../devices/virtual/lothars_class/lothars_device

# rmmod happy_ioctl
```
Logs  
```
$ dmesg | tail
    [ 3175.829061] chardev_init(): hello chardev init
    [ 3175.829109] chardev_init(): allocated correctly with major number 237
    [ 3175.829253] chardev_init(): device class registered correctly
    [ 3175.830412] chardev_init(): the device is created correctly
    [ 3178.260457] chardev_open(): called
    [ 3178.260523] chardev_ioctl(): called, cmd = 100, arg = 110
    [ 3178.260572] chardev_close(): called
    [ 3195.897009] chardev_exit(): hello chardev exit
```

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
