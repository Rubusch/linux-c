# chardev example
verified against a RPI3 w/ aarch64  

## kernel module
should compile cross - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
copy the module over to the target  

## userspace
easiest is to copy the folder `userspace`  to the target  
```
rpi$ cd ./userspace
rpi$ make
```

## usage
on the target perform the following to verify the functionality  
```
pi@raspberrypi:~$ sudo insmod ./chardev.ko

pi@raspberrypi:~$ sudo mknod /dev/mydev c 202 0

pi@raspberrypi:~$ sudo ./ioctl_test.elf

pi@raspberrypi:~$ sudo rmmod chardev

pi@raspberrypi:~$ dmesg | tail -n 10
    [ 3874.096369] IPv6: ADDRCONF(NETDEV_CHANGE): wlan0: link becomes ready
    [ 3975.510996] my_dev_open(): called
    [ 3975.511055] my_dev_ioctl(): called, cmd = 100, arg = 110
    [ 3975.511101] my_dev_close(): called
    [ 4006.419156] hello chardev exit
    [ 4018.302293] hello chardev init
    [ 4025.289036] my_dev_open(): called
    [ 4025.289085] my_dev_ioctl(): called, cmd = 100, arg = 110
    [ 4025.289120] my_dev_close(): called
    [ 4033.433427] hello chardev exit
```
