# Chardev Module Using devtmpfs

Device files are created by the kernel via the temporary devtmpfs filesystem. Any driver that wishes to register a device node will use the devtmpfs (via the core
driver) to do it. When a devtmpfs instance is mounted on /dev, the device node will initially be created with a fixed name, permissions and owner. These entries can
be both read from and written to. All device nodes are owned by root and have the default mode of 0600.  

Shortly afterward, the kernel will send an uevent to udevd. Based on the rules specified in the files within the `/etc/udev/rules.d`, `/lib/udev/rules.d` and
`/run/udev/rules.d` directories, udevd will create additional symlinks to the device node, or change its permissions, owner, or group, or modify the internal udevd
database entry (name) for that object. The rules in these three directories are numbered and all three directories are merged together. If udevd can't find a rule
for the device it is creating, it will leave the permissions and ownership at whatever devtmpfs used initially.

Kernel configuration: CONFIG_DEVTMPFS_MOUNT.  

## Module
Should compile cross - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace
Easiest is to copy the folder `userspace`  to the target  
```
rpi$ cd ./userspace
rpi$ make
```

## Usage
On the target perform the following to verify the functionality  
```
pi@raspberrypi:~$ cd ./userspace/

pi@raspberrypi:~/userspace$ make
    gcc -g -Wall   -c -o ioctl_test.o ioctl_test.c
    gcc -g -Wall -o ioctl_test.elf ioctl_test.o

pi@raspberrypi:~/userspace$ cd ..

pi@raspberrypi:~$ ll
    total 36616
    -rw-r--r-- 1 pi pi    11104 May  7 13:29 chardev.ko
    -rw-r--r-- 1 pi pi  8426768 May  5 16:19 linux-headers-6.3.1-lothar008+_6.3.1-g9ea5c8b206e2-3_arm64.deb
    -rw-r--r-- 1 pi pi 27782824 May  5 16:19 linux-image-6.3.1-lothar008+_6.3.1-g9ea5c8b206e2-3_arm64.deb
    -rw-r--r-- 1 pi pi  1262628 May  5 16:19 linux-libc-dev_6.3.1-g9ea5c8b206e2-3_arm64.deb
    drwxr-xr-x 2 pi pi     4096 May  7 13:30 userspace

pi@raspberrypi:~$ sudo insmod chardev.ko

pi@raspberrypi:~$ sudo ./userspace/ioctl_test.elf

pi@raspberrypi:~$ ls -l /sys/class/lothars_class/lothars_device
    lrwxrwxrwx 1 root root 0 May  7 13:31 /sys/class/lothars_class/lothars_device -> ../../devices/virtual/lothars_class/lothars_device

pi@raspberrypi:~$ sudo rmmod chardev

pi@raspberrypi:~$ dmesg
    [ 4566.355582] chardev: loading out-of-tree module taints kernel.
    [ 4566.356139] chardev_init(): hello chardev init
    [ 4566.356154] chardev_init(): allocated correctly with major number 236
    [ 4566.356242] chardev_init(): device class registered correctly
    [ 4566.356444] chardev_init(): the device is created correctly
    [ 4576.642901] chardev_open(): called
    [ 4576.642949] chardev_ioctl(): called, cmd = 100, arg = 110
    [ 4576.642984] chardev_close(): called
    [ 4616.264658] chardev_exit(): hello chardev exit
```

## Verified
* Verified against a RPI3 w/ aarch64

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
