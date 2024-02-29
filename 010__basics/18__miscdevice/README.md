# Miscdevice

The miscdevice wraps over the common chardev driver tasks, it sets up
cdev, adds it, creates a device class, provides a device node and a
class node. Thus it is the preferred way to setup a character device.  

The miscdevice API removes all the boilerplate code needed in a manual
initialization of the chardev devices. In case of customized driver
structs or the like the driver might be adjusted, or manual
instantiation of the device can be needed.  

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

The device driver implemented as a miscellaneous character device uses the
major number allocated by the Linux kernel for **miscellaneous
devices**. This eliminates the need to define a unique major number
for the driver; this is important, as a conflict between major numbers
has become increasingly likely, and use of the misc device class is an
effective tactic. Each probed device is dynamically assigned a minor
number and is listed with a directory entry within the sysfs
pseudo-filesystem under `/sys/class/misc/`. Major number 10 is
officially assigned to the misc driver.  

## Module

Should compile cross - having `crossbuild-essentials-arm64` installed,
`ARCH`, and `CROSS_COMPILE` set, execute  

```
$ make
$ scp ./*.elf pi@10.1.10.204:~/
$ scp ./*.ko pi@10.1.10.204:~/
```

## Usage
```
# insmod ./miscdevice.ko
# ./ioctl_app.elf

# ls -l /dev/lothars_device
    crw------- 1 root root 10, 122 Oct  3 15:45 /dev/lothars_device

# cat /sys/class/misc/lothars_device/dev
    10:122

# ls -l /sys/class/misc/lothars_device
    lrwxrwxrwx 1 root root 0 Oct  3 15:46 /sys/class/misc/lothars_device -> ../../devices/virtual/misc/lothars_device

# rmmod miscdevice

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

## References
* https://embetronicx.com/tutorials/linux/device-drivers/misc-device-driver/
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
