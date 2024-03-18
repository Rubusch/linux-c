# ADXL345 driver

ADXL345 Accel click mikroBUS: https://www.mikroe.com/accel-spi-board

Currently (v6.8) the kernel has an input driver variant (2009?) for this
accelerometer, and an iio based driver approach. Probably, this particular
implementation should be moved to iio (e.g. ADC, DAC, accelerometers,...),
rather than to input devices (e.g. mice, trackballs, joysticks,...).  

First I extracted the upstream kernel iio driver for the
adxl345.

Note: The `adxl345_spi.ko` won't load easily, see insmod error:
```
# insmod adxl345_spi.ko
insmod: ERROR: could not insert module adxl345_spi.ko: Invalid module format
```
The fix is simply figure out the extra-version
```
# uname -r
6.3.13-lothar09+
```
So for this kernel, it will be `-lothar09+`. Edit the Makefile and set:
```
    EXTRAVERSION = "-lothar09+"
```

## Hardware

Connection (RPI3b to ADXL345), I used the same connection for the adxl345 as in the other demos, soldered to SPI.  
- SPI_CE0_N, GPIO08 / pin 24 -> CS
- SPI_SCLK, GPIO11 / pin 23 -> SCK
- SPI_MISO, GPIO09 / pin 21 -> SDO
- SPI_MOSI, GPIO10 / pin 19 -> SDI
- GPIO23 / pin 16 -> INT
- 3v3 -> 3v3
- GND -> GND

# Build

Having `crossbuild-essentials-arm64` installed, provide `KERNELDIR`, `ARCH`, and `CROSS_COMPILE`, then execute  
```
$ cd ./module
$ make
```
Copy the module over to the target. Copy the `.dtbo` file to `/boot/overlays` and register the DT overlay in `/boot/config.txt` as `dtoverlay` without file extension.  

## Usage
Load the kernel modules, as here done just for the spi version (i2c similar).  
```
$ sudo su

# modprobe industrialio
# modprobe regmap_spi

# insmod adxl345_core.ko
# insmod adxl345_spi.ko

# lsmod | grep 345
    adxl345_spi            16384  0
    adxl345_core           16384  1 adxl345_spi
    regmap_spi             16384  1 adxl345_spi
    industrialio          102400  1 adxl345_core
```

...or for the i2c variant do  
```
# modprobe industrialio
# modprobe regmap_i2c
# modprobe i2c-dev

# insmod adxl345_core.ko
# insmod adxl345_i2c.ko
```

Check functionality of the accelerometer.  

Load the module and observe incoming data on i2c with the tool evtest.  
```
# ll /sys/bus/iio/devices/iio\:device0/
total 0
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_sampling_frequency
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_scale
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_x_calibbias
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_x_raw
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_y_calibbias
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_y_raw
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_z_calibbias
-rw-r--r-- 1 root root 4096 Mar 18 18:29 in_accel_z_raw
-r--r--r-- 1 root root 4096 Mar 18 18:29 name
lrwxrwxrwx 1 root root    0 Mar 18 18:29 of_node -> ../../../../../../../../firmware/devicetree/base/soc/spi@7e204000/adxl345@0
drwxr-xr-x 2 root root    0 Mar 18 18:29 power
-r--r--r-- 1 root root 4096 Mar 18 18:29 sampling_frequency_available
lrwxrwxrwx 1 root root    0 Mar 18 18:29 subsystem -> ../../../../../../../../bus/iio
-rw-r--r-- 1 root root 4096 Mar 18 18:29 uevent
-r--r--r-- 1 root root 4096 Mar 18 18:29 waiting_for_supplier

# cat /sys/bus/iio/devices/iio\:device0/{in_accel_x_raw,in_accel_y_raw,in_accel_z_raw}
-18
5
252

```
move the board, to generate input data with the accelerometer    

# rmmod input_demo
```

Logs for spi (here `dmesg` when hardware is attached)  
```
[ 1539.449651] adxl345_core_probe(): called
[ 1539.449704] adxl345_setup(): called
[ 1539.449713] adxl345_setup(): calling setup()
[ 1539.449722] adxl345_spi_setup(): called
[ 1539.449730] adxl345_setup(): retrieving DEVID
[ 1539.449788] adxl345_setup(): setting full range  ## with hardware
[ 1539.449816] adxl345_setup(): enable measurement
```

Logs for i2c (here `/var/log/messages` when no hardware is attached)  
```
Mar 18 10:44:13 ctrl01 kernel: [  375.199296] adxl345_i2c_probe(): called
Mar 18 10:44:13 ctrl01 kernel: [  375.199420] adxl345_core_probe(): called
Mar 18 10:44:13 ctrl01 kernel: [  375.199452] adxl345_setup(): called
Mar 18 10:44:13 ctrl01 kernel: [  375.199461] adxl345_setup(): calling setup()
Mar 18 10:44:13 ctrl01 kernel: [  375.199470] adxl345_setup(): retrieving DEVID
Mar 18 10:44:13 ctrl01 kernel: [  375.200012] adxl345_i2c: probe of 1-001d failed with error -121
```


## References
* Linux kernel 6.3.13, this is the mainline kernel driver with some upgrades
* ADXL345 datasheet
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018 for ways how to verify the driver
