# Hello Raspberry4 GPIO/LED

The source was tested compiled and running on 5.4.75.  

# Build

```
$ make
```

# Usage

```
pi@ctrl001:/tmp $ sudo insmod hello_gpio.ko 

pi@ctrl001:/tmp $ sudo rmmod hello_gpio 

pi@ctrl001:/tmp $ dmesg | tail
    [   33.586933] hello_gpio: loading out-of-tree module taints kernel.
    [   33.587543] mod_init() - major 239, minor 0
    [   33.592209] gpio_irq = 186
    [   33.592309] mod_init() done
    [  578.339581] mod_init() - major 238, minor 0
    [  578.340543] mod_init() done
```

# Notes

This is basically an implementation of the project to light a led as
described on embetronicx.com, using my own module and my own Raspi4
build.  

---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * https://embetronicx.com/tutorials/linux/device-drivers/gpio-driver-basic-using-raspberry-pi/
