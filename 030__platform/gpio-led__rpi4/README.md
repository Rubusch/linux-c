# Hello Raspberry4 GPIO/LED

This is basically an implementation of the project to light a led as
described on embetronicx.com, using my own module and my own Raspi4
build.  

# Build

```
$ make
```

# Usage

```
pi@ctrl001:/tmp $ sudo insmod hello_gpio.ko

pi@ctrl001:/tmp $ sudo rmmod hello_gpio

pi@ctrl001:/tmp $ dmesg | tail
    [   26.473992] mindblowing_probe(): called
    [   26.474436] mindblowing_probe(): got minor 121
    [   26.474547] mindblowing_probe(): done
    [   26.657922] mindblowing_remove(): called
```

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * https://embetronicx.com/tutorials/linux/device-drivers/gpio-driver-basic-using-raspberry-pi/
