# LED Module with Devicetree Binding (LED class)

The LED class driver demo of A. Rios (see references).  

The LED class will simplify the development of drivers that control
LEDs. A "class" is both the implementation of a set of devices and the
set of devices themselves.  

The LED class supports the blinking, flashing and brightness control
features of physical LEDs. This class requires an underlying device to
be available (`/sys/class/leds/<device>/`). This underlying device mus
be able to turn the LED on or off, may be able to set the brightness
and might even provide timer functionality to autonomously blink the
LED with a given period and duty cycle.  

Note, the LED class introduces the optional concept of LED trigger. A
trigger is a kernel based source of LED events. The timer trigger is
an example that will periodically change the LED brightness between
LED_OFF and the current brightness setting. The "on" and "off" time
can be specified via `/sys/class/leds/<device>/delay_{on,off}` sysfs
entry in miliseconds.  

Note for the probe:  
* The `platform_get_resource()` function gets the I/O registers
  resource described by the DT reg property.
* The `dev_ioremap()` function maps the area of register addresses to
  kernel virtual addresses.
* The `for_each_child_of_node()` function walks for each sub-node of
  the main node, allocating a private structure for each one by using
  the `devm_kzalloc()` function, then initializes the `led_classdev`
  field of each private structure.
* The `devm_led_classdev_register()` function registers each LED class
  device to the LED subsystem.

Note, ledclass_dev is provided by the kernel.  

[further details on IOMUX on NPX's iMX7D p125ff -> references]

## Linux

A copy of the modified DTS is provided, copy it to the specified location in the linux sources (6.3), then build it.  

```
$ cd linux
$ make dtbs
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the /boot/bcm2710-rpi-3-b.dtb. In case make a safety backup first.  

## Module
Should compile cross - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace
Easiest is to copy the folder `userspace`  to the target  
```
rpi$ cd ./userspace
rpi$ make
```

## Usage
On the target perform the following to verify the functionality  
```
pi@raspberrypi:~$ sudo insmod chardev.ko

pi@raspberrypi:~$ sudo find /sys -name "*lothar*"
    /sys/bus/platform/drivers/lotharskeys
    /sys/module/chardev/drivers/platform:lotharskeys

pi@raspberrypi:~$ rmmod chardev
```
The module could be load, the devicetree binding would match.  

## Verified
* Verified against a RPI3 w/ aarch64

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 176
