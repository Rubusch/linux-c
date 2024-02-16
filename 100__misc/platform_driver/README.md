# platform driver

The demo shows a dummy platform driver and a starter module. The
platform driver usually is loaded by DT binding, alternatively by
another module, as in this demo.  

## Usage

```
$ sudo su
# insmod platform.c
# insmod platform-ins.c

# ls -l /sys/devices/platform/lothars-platform-dummy.1/
total 0
lrwxrwxrwx 1 root root    0 Feb 16 21:34 driver -> ../../../bus/platform/drivers/lothars-platform-dummy
-rw-r--r-- 1 root root 4096 Feb 16 21:41 driver_override
-r--r--r-- 1 root root 4096 Feb 16 21:41 modalias
drwxr-xr-x 2 root root    0 Feb 16 21:41 power
lrwxrwxrwx 1 root root    0 Feb 16 21:41 subsystem -> ../../../bus/platform
-rw-r--r-- 1 root root 4096 Feb 16 21:34 uevent

# udevadm info /dev/lothars_device
P: /devices/virtual/misc/lothars_device
N: lothars_device
L: 0
E: DEVPATH=/devices/virtual/misc/lothars_device
E: DEVNAME=/dev/lothars_device
E: MAJOR=10
E: MINOR=121
E: SUBSYSTEM=misc

# cat /dev/lothars_device
# echo "foo" > /dev/lothars_device

# rmmod platform-ins.ko
# rmmod platform


$ dmesg
Feb 16 21:31:03 ctrl001 kernel: [  343.403120] platform: loading out-of-tree module taints kernel.

Feb 16 21:34:50 ctrl001 kernel: [  571.005423] pf_probe(): called
Feb 16 21:47:03 ctrl001 kernel: [ 1303.682709] dummy_open(): called
Feb 16 21:47:03 ctrl001 kernel: [ 1303.682900] dummy_read(): called
Feb 16 21:47:03 ctrl001 kernel: [ 1303.683119] dummy_release(): called
Feb 16 21:47:11 ctrl001 kernel: [ 1311.423151] dummy_open(): called
Feb 16 21:47:11 ctrl001 kernel: [ 1311.423623] dummy_write(): called
Feb 16 21:47:11 ctrl001 kernel: [ 1311.423711] dummy_release(): called
```

## References:

* Linux Device Driver Programming, J. Madieu, 2022
