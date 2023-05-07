# Chardev Module Using miscdevice

The `miscdevice` framework is an interface exported by the Linux kernel that allows modules to register their individual minor numbers.  

The device driver implemented as a miscellaneous character uses the major number allocated by the Linux kernel for **miscellaneous devices**. This eliminates the
need to define a unique major number for the driver; this is important, as a conflict between major numbers has become increasingly likely, and use of the misc
device class is an effective tactic. Each probed device is dynamically assigned a minor number and is listed with a directory entry within the sysfs
pseudo-filesystem under `/sys/class/misc/`. Major number 10 is officially assigned to the misc driver.  

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

pi@raspberrypi:~$ sudo insmod chardev.ko

pi@raspberrypi:~$ sudo ./userspace/ioctl_test.elf

pi@raspberrypi:~$ ls -l /dev/lothars_device
    crw------- 1 root root 10, 121 May  7 14:55 /dev/lothars_device

pi@raspberrypi:~$ cat /sys/class/misc/lothars_device/dev
    10:121

pi@raspberrypi:~$ ls -l /sys/class/misc/lothars_device
    lrwxrwxrwx 1 root root 0 May  7 14:56 /sys/class/misc/lothars_device -> ../../devices/virtual/misc/lothars_device

pi@raspberrypi:~$ sudo rmmod chardev

pi@raspberrypi:~$ dmesg | tail
    [ 9656.926328] chardev_init(): hello chardev init
    [ 9656.927167] chardev_init(): got minor 121
    [ 9669.830434] chardev_open(): called
    [ 9669.830489] chardev_ioctl(): called, cmd = 100, arg = 110
    [ 9669.830529] chardev_close(): called
    [ 9702.654548] chardev_exit(): hello chardev exit
```

## Verified
* Verified against a RPI3 w/ aarch64

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
