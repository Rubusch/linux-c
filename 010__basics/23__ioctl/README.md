# Minimal ioctl and Chardev

The demo uses ioctl calls from userspace application. A chardev driver
will react on the ioctl calls.  

## kernel module

Having e.g. `crossbuild-essentials-arm64` installed, `ARCH`, and
`CROSS_COMPILE` set, execute the folloing. Copy the artifacts over to
the target, e.g. if pi is on 10.1.10.204  
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
pi@raspberrypi:~$ dmesg | tail
    [34246.160280] mod_init(): called
    [34249.495571] chardev_open(): called
    [34249.495634] chardev_ioctl(): called, cmd = 100, arg = 110
    [34249.495682] chardev_close(): called
    [34253.619808] mod_exit(): called
```

## References
- Linux Driver Development for Embedded Processors, A. L. Rios, 2018
