# GPIO LED Driver

The demo shows how to operate gpios from off the linux kernel. The
demo is useful to test/verify basic gpio setup.

NB: Since kernel 4.8 we have the "new" GPIO implementation, i.e.
GPIOs should not be assigned over (shared) sysfs but exclusively by
gpiobank and line_offset number. There is a userspace library by the
name of libgpiod and gpiod tools. Note gpio assignments here only hold
as long as the assigning PID (i.e. also the SHELL) persists, then is
undefined, and eventually falls back to default setting.  

## Setup

Wire from `gpio04` to a breadboard. Place some resistor. Then a
led. Wire then back to ground, e.g. next to `gpio04` on position 9
should be `gnd`.

## Usage

```
# insmod ./gpioled.ko
# echo 1 > /dev/lothars_gpio_driver
# echo 0 > /dev/lothars_gpio_driver
# echo 1 > /dev/lothars_gpio_driver
# echo 0 > /dev/lothars_gpio_driver
# rmmod gpioled.ko
```

## References
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
