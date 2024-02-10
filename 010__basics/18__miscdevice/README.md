# Chardev Module Using miscdevice

The `miscdevice` framework is an interface exported by the Linux
kernel that allows modules to register their individual minor numbers.  

The miscdriver wraps over the common chardev driver tasks.

#### `misc_register()` performs:

- `alloc_chrdev_region();` – used for the major and minor number
- `cdev_init();` – used to initialize cdev
- `cdev_add();` – used to  add the cdev structure to the device
- `class_create();`  – used to create a class
- `device_create();`  – used to create a device

#### `misc_deregister()` performs:

- `cdev_del();` – used to delete cdev
- `unregister_chrdev_region();`  – used to remove the major and minor number
- `device_destroy();`  – used to delete the device
- `class_destroy();` – used to delete the class

The device driver implemented as a miscellaneous character uses the
major number allocated by the Linux kernel for **miscellaneous
devices**. This eliminates the need to define a unique major number
for the driver; this is important, as a conflict between major numbers
has become increasingly likely, and use of the misc device class is an
effective tactic. Each probed device is dynamically assigned a minor
number and is listed with a directory entry within the sysfs
pseudo-filesystem under `/sys/class/misc/`. Major number 10 is
officially assigned to the misc driver.  

## Module
Should compile cross - having `crossbuild-essentials-arm64` installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace
Compile cross as well, set ``$CROSS_COMPILE``.   
```
rpi$ cd ./userspace
rpi$ make
```
Copy the module over to the target  

## Usage
```
$ sudo insmod chardev.ko

$ sudo ./ioctl_test.elf

$ ls -l /dev/lothars_device
    crw------- 1 root root 10, 122 Oct  3 15:45 /dev/lothars_device

$ cat /sys/class/misc/lothars_device/dev
    10:122

$ ls -l /sys/class/misc/lothars_device
    lrwxrwxrwx 1 root root 0 Oct  3 15:46 /sys/class/misc/lothars_device -> ../../devices/virtual/misc/lothars_device

$ sudo rmmod chardev

$ dmesg | tail
    [  734.176532] chardev_open(): called
    [  734.176582] chardev_ioctl(): called, cmd = 100, arg = 110
    [  734.176617] chardev_close(): called
    [  801.709206] chardev_exit(): hello chardev exit
    [  994.089506] chardev_init(): hello chardev init
    [  994.089889] chardev_init(): got minor 122
    [ 1003.358642] chardev_open(): called
    [ 1003.358699] chardev_ioctl(): called, cmd = 100, arg = 110
    [ 1003.358736] chardev_close(): called
    [ 1068.685229] chardev_exit(): hello chardev exit
```

## Verified
* Verified against a RPI3 w/ aarch64

## References
* https://embetronicx.com/tutorials/linux/device-drivers/misc-device-driver/
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
