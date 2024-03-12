# iio driver dummy

iio: The Industrial I/O core offers a unified framework for writing
drivers for many different types of embedded sensors.  

input: Some of the parts might be also used in input drivers, such as
for mouse, joystick, etc.  

The demo shows a dummy for the industrial IO (iio) API. After
executing `make` there will be two kernel modules. The `-ins.ko`
module triggers the driver module to omit messing with the device-tree
entries (so, no reboot needed).  

## Usage

NB: w/o loading first the `industrialio` module, the symbols can never
be resolved ;)  

```
# modprobe industrialio

# insmod iio-dummy.ko
# insmod start.ko PROBED_MODULE_NAME="lothars-iio-dummy"
```

```
# ls -l /sys/bus/iio/devices/
    total 0
    lrwxrwxrwx 1 root root 0 Feb 18 20:35 iio:device0 -> ../../../devices/platform/lothars-iio-dummy.1/iio:device0

# ls -l /sys/bus/iio/devices/iio\:device0/
    total 0
    -rw-r--r-- 1 root root 4096 Feb 18 20:35 in_voltage0_raw
    -rw-r--r-- 1 root root 4096 Feb 18 20:35 in_voltage1_raw
    -rw-r--r-- 1 root root 4096 Feb 18 20:35 in_voltage2_raw
    -rw-r--r-- 1 root root 4096 Feb 18 20:35 in_voltage3_raw
    -rw-r--r-- 1 root root 4096 Feb 18 20:35 in_voltage_scale
    -r--r--r-- 1 root root 4096 Feb 18 20:35 name
    drwxr-xr-x 2 root root    0 Feb 18 20:35 power
    lrwxrwxrwx 1 root root    0 Feb 18 20:35 subsystem -> ../../../../bus/iio
    -rw-r--r-- 1 root root 4096 Feb 18 20:34 uevent

# cat /sys/bus/iio/devices/iio:device0/name
    iio_dummy

# udevadm info /sys/bus/iio/devices/iio\:device0
    P: /devices/platform/lothars-iio-dummy.1/iio:device0
    L: 0
    E: DEVPATH=/devices/platform/lothars-iio-dummy.1/iio:device0
    E: DEVTYPE=iio_device
    E: SUBSYSTEM=iio
```
Logs  
```
[22:05:53.668] Feb 18 21:05:53 ctrl001 kernel: [  231.612620] pdrv_probe(): called
[22:06:43.493] Feb 18 21:06:43 ctrl001 kernel: [  281.417333] pdrv_remove(): called
```

## References
* https://www.kernel.org/doc/html/latest/driver-api/iio/index.html
* https://wiki.analog.com/software/linux/docs/iio/iio
* Linux Device Driver Programming, J. Madieu, 2022
