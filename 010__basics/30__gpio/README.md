# gpio led

The demo shows how to operate gpios from off the linux kernel.

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
