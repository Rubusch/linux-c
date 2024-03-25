# platform driver

The demo shows a dummy platform driver and a starter module. The
platform driver usually is loaded by DT binding, alternatively by
another module, as in this demo.  

# Build

## Module
For cross-compilation install `crossbuild-essentials-arm64`,
set at least `ARCH`, and `CROSS_COMPILE`. Build the rpi kernel
according to the rpi documentation.  
```
$ cat ~/workspace/source-me.sh
    export CROSS_COMPILE=aarch64-linux-gnu-
    export ARCH=arm64
    export KERNEL=kernel8
    export KDEFCONFIG_NAME=bcm2711_defconfig
    export KERNELDIR=/usr/src/linux
```

Build the module  
```
$ make
```
Copy the module to the target device  

The DT overlay fragment is built with the module. Copy the DT overlay
fragment to the target device, to `/boot/overlays`. Register the DT
overlay fragment in `/boot/configs.txt`.  

```
    ...
    [all]
    dtoverlay = <name of the .dtbo file>
    ...
```
Then reboot. Verify that phandles of the fragment are searcheable in the DT.  
```
# dtc -I fs -O dts /sys/firmware/devicetree/base | less
```

# Usage

```
# insmod platform.ko
# insmod start.ko PROBED_MODULE_NAME="lothars-platform-dummy"

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
```

Logs  
```
$ dmesg
[   29.307140] pdrv_probe(): called
[   29.473476] dummy_open(): called
[   29.473600] dummy_read(): called
[   29.473654] dummy_release(): called
[   29.642775] dummy_open(): called
[   29.642907] dummy_write(): called
[   29.643054] dummy_release(): called
[   29.996749] pdrv_remove(): called
```

## References
* Linux Device Driver Programming, J. Madieu, 2022
