# gpio led

The demo shows how to operate gpios from off the linux kernel.

## Setup

Wire from `gpio04` to a breadboard. Place some resistor. Then a led. Wire then back to ground, e.g. next to `gpio04` on position 9 should be `gnd`.

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
