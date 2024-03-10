# ADXL345 driver

ADXL345 Accel click mikroBUS: https://www.mikroe.com/accel-spi-board

This is an approach to extract an upstream kernel driver for the adxl345. As far as I understood, this iio based driver was implemented in an outreachy internship program. It was presented AFAIR on the Embedded Linux Conference Europe (Lyon?). By chance I attended exactly this slot held be Julia Lawall and was listening to the author Eva Rachel Retuya. It was just by chance that I ended up with this hardware on my table and found in the datasheet and literature that not all features of the hardware were already implemented.

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

Check functionality of the accelerometer.  

Load the module and observe incoming data on i2c with the tool evtest.  
```
# evtest /dev/input/event0
    Input driver version is 1.0.1
    Input device ID: bus 0x18 vendor 0x0 product 0x0 version 0x0
    Input device name: "IOACCEL keyboard"
    Supported events:
      Event type 0 (EV_SYN)
      Event type 1 (EV_KEY)
        Event code 2 (KEY_1)
    Properties:
    Testing ... (interrupt to exit)

--- move the board, to generate input data with the accelerometer ---

    Event: time 1703185720.684805, type 1 (EV_KEY), code 2 (KEY_1), value 1
    Event: time 1703185720.684805, -------------- SYN_REPORT ------------
    Event: time 1703185720.740839, type 1 (EV_KEY), code 2 (KEY_1), value 0
    Event: time 1703185720.740839, -------------- SYN_REPORT ------------
    Event: time 1703185720.852791, type 1 (EV_KEY), code 2 (KEY_1), value 1
    Event: time 1703185720.852791, -------------- SYN_REPORT ------------
    Event: time 1703185720.908827, type 1 (EV_KEY), code 2 (KEY_1), value 0
    Event: time 1703185720.908827, -------------- SYN_REPORT ------------
    Event: time 1703185721.692780, type 1 (EV_KEY), code 2 (KEY_1), value 1
    Event: time 1703185721.692780, -------------- SYN_REPORT ------------
    Event: time 1703185721.748833, type 1 (EV_KEY), code 2 (KEY_1), value 0
    Event: time 1703185721.748833, -------------- SYN_REPORT ------------
    Event: time 1703185721.916789, type 1 (EV_KEY), code 2 (KEY_1), value 1
    Event: time 1703185721.916789, -------------- SYN_REPORT ------------
    Event: time 1703185721.972838, type 1 (EV_KEY), code 2 (KEY_1), value 0
    Event: time 1703185721.972838, -------------- SYN_REPORT ------------
    ^C

# rmmod input_demo
```

Follow the logs   
```
Dec 21 19:01:13 ctrl001 kernel: [  722.135841] i2c_dev: i2c /dev entries driver

Dec 21 19:08:06 ctrl001 kernel: [ 1135.192614] input_demo: loading out-of-tree module taints kernel.
Dec 21 19:08:06 ctrl001 kernel: [ 1135.194003] adxl345 1-001d: ioaccel_probe() - called
Dec 21 19:08:06 ctrl001 kernel: [ 1135.195141] input: IOACCEL keyboard as /devices/platform/soc/3f804000.i2c/i2c-1/1-001d/input/input0
```

## References
* Linux kernel 6.3.13, this is the mainline kernel driver with some upgrades
* ADXL345 datasheet
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018 for ways how to verify the driver
