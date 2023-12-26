# LTC2607: 

TODO

## Hardware: Eval Board dc934a

The LTC2607 is a dual 12-bit, 2.7V to 5.5V rail-to-rail voltage output DAC. It uses a 2-wire, I2C compatible serial interface. The LTC2607 operates in both the standard mode (clock rate of 100kHz) and the fast mode (clock rate of 400kHz).  

The driver will control each LtC2607 internal DAC individually or both DACa + DACb in a simultaneous mode. The IIO framework will generate three separate sysfs files (attributes) used for sending data to the dual DAC from the user space application.  

LTC2607: https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/dc934a.html

Connection:  
- 5V -> V+ (connector j1: 1)
- GND -> GND
- GPIO02 (i2c SDA) -> SDA
- GPIO03 (i2c SCL) -> SCL

Use the device's LT1790ACS6-5 5V output (see _U3_ in schema) as the VREF, i.e.  
- jumper _5V_ at _JP1 VREFA_
- jumper _5V REF_ in _JP2_
- jumper _JP4_, _JP6_, _JP7_ to VCC (connect 1 with middle), i.e. CA0, CA1, CA2 are set to VCC, this matches with the next I2C slave address 0x72
- Solder out the device _U7_ from the DC934A board and also the LTC2607 I2C pull-up resistors: _R10_ and _R11_

NB: Using the 5V regulator (_5V REG_ in _JP2_) as the source for VCC has the limitation that VCC may be slightly lower than VREF, which may affect the full-scale error. Selecting the 5V REF as the source for VCC overcomes this, however the total current that the LTC2607 can source will be limited approximately 5mA.  

# Build

## Devicetree

Copy it to the specified location in the linux sources, then build it  
```
$ cd linux
$ cp -arf <SOURCES>/devicetree/arch ./

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
root@ctrl001:/home/pi# modprobe i2c-dev

root@ctrl001:/home/pi# i2cdetect -l
    i2c-10	i2c       	i2c-11-mux (chan_id 1)          	I2C adapter
    i2c-1	i2c       	bcm2835 (i2c@7e804000)          	I2C adapter
    i2c-11	i2c       	bcm2835 (i2c@7e205000)          	I2C adapter
    i2c-0	i2c       	i2c-11-mux (chan_id 0)          	I2C adapter

root@ctrl001:/home/pi# i2cdetect -y 1
         0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    00:                         -- -- -- -- -- -- -- --
    10: -- -- -- -- -- -- -- -- -- -- -- -- -- 1d -- --
    20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    70: -- -- -- -- -- -- -- --
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
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 423  
