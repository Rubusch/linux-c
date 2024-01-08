# ADXL345 driver

TODO

## Hardware

- ADXL345 Accel click mikroBUS: https://www.mikroe.com/accel-spi-board

Connection:  
- GPIO02 -> SDA
- GPIO03 -> SCL
- 3v3 -> 3v3
- GND -> GND

# Build

## Devicetree

Copy it to the specified location in the linux sources, then build it  
```
$ cd linux
$ cp -arf <SOURCES>/devicetree/arch ./
$ find . -name \*.dtb -delete
$ make dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module

Having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Usage

See the connected Accel Click appears on `1d` (i2c).  
```
$ sudo su
# modprobe regmap-spi
# modprobe regmap-i2c
```

Check functionality of the accelerometer.  
```
root@ctrl001:/home/pi# i2cset -y 1 0x1d 0x2d 0x08
root@ctrl001:/home/pi# while true; do i2cget -y 1 0x1d 0x33; sleep 1; done
    0xff
    0xff
    0xff
    0xff
    0xff
    0x00 <--- move the module around, to generate input data
    0x01
    0x01
    0xfe
    0xff
    0xff
    0xff
    0xff
    0xff
    ^C
```

Load the module and observe incoming data on i2c with the tool evtest.  
```
root@ctrl001:/home/pi# insmod input_demo.ko

root@ctrl001:/home/pi# evtest /dev/input/event0
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

root@ctrl001:/home/pi# rmmod input_demo
```

Follow the logs   
```
Dec 21 19:01:13 ctrl001 kernel: [  722.135841] i2c_dev: i2c /dev entries driver

Dec 21 19:08:06 ctrl001 kernel: [ 1135.192614] input_demo: loading out-of-tree module taints kernel.
Dec 21 19:08:06 ctrl001 kernel: [ 1135.194003] adxl345 1-001d: ioaccel_probe() - called
Dec 21 19:08:06 ctrl001 kernel: [ 1135.195141] input: IOACCEL keyboard as /devices/platform/soc/3f804000.i2c/i2c-1/1-001d/input/input0
```

## References
TODO: update - this is the mainline driver             
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 423  
