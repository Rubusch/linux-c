# Hello Raspberry4 GPIO/LED with Interrupt/Button

This is basically an implementation of the project to light a led as
described on embetronicx.com, using my own module and my own Raspi4
build.

# Build

```
$ make
```

# USAGE

```
pi@ctrl001:/tmp $ sudo insmod hello_gpio.ko

pi@ctrl001:/tmp $ sudo rmmod hello_gpio

pi@ctrl001:/tmp $ dmesg | tail
    [   26.846417] mindblowing_probe(): called
    [   26.847063] mindblowing_probe(): got minor 121
    [   26.847718] mindblowing_probe(): ok
    [   27.025593] mindblowing_remove(): called
```

## References:

 * https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
