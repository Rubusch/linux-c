# Minimal ioctl and Chardev

The demo uses ioctl calls from userspace application. A chardev driver will react on the ioctl calls.  

Verified against a RPI3 w/ aarch64  

## kernel module

Having e.g. `crossbuild-essentials-arm64` installed, `ARCH`, and `CROSS_COMPILE` set, execute the folloing. Copy the artifacts over to the target, e.g. if pi is on 10.1.10.204  
```
$ make
$ scp ./*.elf pi@10.1.10.204:~/
$ scp ./*.ko pi@10.1.10.204:~/
```

## Usage

```
# insmod ./ioctl.ko
# ./ioctl_app.elf
# rmmod ioctl.k
```
Logs  
```
pi@raspberrypi:~$ dmesg | tail -f
[12:42:32.935] Feb 11 00:46:24 ctrl001 kernel: [ 8253.329589] chrdev_init(): called
[12:42:37.055] Feb 11 00:46:28 ctrl001 kernel: [ 8972.319078] mod_init(): called
[12:42:37.093] Feb 11 00:46:28 ctrl001 kernel: [ 8976.459050] chardev_open(): called
[12:42:37.093] Feb 11 00:46:28 ctrl001 kernel: [ 8976.459117] chardev_ioctl(): called, cmd = 100, arg = 110
[12:42:37.094] Feb 11 00:46:28 ctrl001 kernel: [ 8976.459164] chardev_close(): called
```

## References
- Linux Driver Development for Embedded Processors, A. L. Rios, 2018
