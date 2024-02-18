# RTC driver dummy

The snippet shows what is needed to setup an rtc driver in the linux kernel.

## Usage

Before  
```
# ls -l /dev/rtc*
    ls: cannot access '/dev/rtc*': No such file or directory
```
Loading the module  
```
$ sudo su

# insmod rtc-dummy.ko
# insmod start.ko PROBED_MODULE_NAME="lothars-rtc-dummy"

# ls -l /dev/rtc*
    lrwxrwxrwx 1 root root      4 Feb 17 22:25 /dev/rtc -> rtc0
    crw------- 1 root root 252, 0 Feb 17 22:25 /dev/rtc0

# udevadm info /dev/rtc0
    P: /devices/platform/lothars-rtc-dummy.1/rtc/rtc0
    N: rtc0
    L: -100
    S: rtc
    E: DEVPATH=/devices/platform/lothars-rtc-dummy.1/rtc/rtc0
    E: DEVNAME=/dev/rtc0
    E: MAJOR=252
    E: MINOR=0
    E: SUBSYSTEM=rtc
    E: USEC_INITIALIZED=539707937
    E: DEVLINKS=/dev/rtc

# cat /sys/class/rtc/rtc0/name
    lothars-rtc-dummy lothars-rtc-dummy.1

# cat /sys/class/rtc/rtc0/date
    1970-01-01
```
Logs  
```
Feb 17 22:25:41 ctrl001 kernel: [  529.304817] rtc_dummy: loading out-of-tree module taints kernel.

Feb 17 22:25:52 ctrl001 kernel: [  539.685807] pdrv_probe(): called
Feb 17 22:25:52 ctrl001 kernel: [  539.685881] lothars-rtc-dummy lothars-rtc-dummy.1: pdrv_probe(): begin_time is 540, rtc_time is 0
Feb 17 22:25:52 ctrl001 kernel: [  539.685914] rtcdrv_read_time(): called
Feb 17 22:25:52 ctrl001 kernel: [  539.686417] lothars-rtc-dummy lothars-rtc-dummy.1: registered as rtc0
Feb 17 22:25:52 ctrl001 kernel: [  539.686448] rtcdrv_read_time(): called
Feb 17 22:25:52 ctrl001 kernel: [  539.686516] lothars-rtc-dummy lothars-rtc-dummy.1: setting system clock to 1970-01-01T00:00:00 UTC (0)

Feb 17 22:27:38 ctrl001 kernel: [  645.749788] rtcdrv_read_time(): called
Feb 17 22:28:24 ctrl001 kernel: [  692.289698] pdrv_remove(): called
```

## References
* _short_ drivers in the kernel, e.g. `/drivers/rtc/rtc-aspeed.c`
* yet another _short_ driver in kernel `/drivers/rtc/rtc-efi.c`
* Linux Device Driver Programming, J. Madieu, 2022

