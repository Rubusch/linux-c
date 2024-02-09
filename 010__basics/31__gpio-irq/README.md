# gpio irq

The demo shows incomming asynchronous events on a gpio line.  

## Usage

```
# insmod ./gpioirq.ko
```
Now press the button, or connect gpio25 to 3.3V of the board.
```
# rmmod ./gpioirq.ko
```

Logs  
```
[22:35:00.036] Feb  9 21:34:59 ctrl001 kernel: [ 3291.863208] mod_exit(): called
[22:35:00.069] Feb  9 21:34:59 ctrl001 kernel: [ 3346.643063] mod_init(): called
[22:35:00.069] Feb  9 21:34:59 ctrl001 kernel: [ 3346.643158] mod_init(): gpio has irq number 185

<connect wire from GPIO25 on RPI3b to 3.3V>

[22:35:08.176] Feb  9 21:35:08 ctrl001 kernel: [ 3346.643232] mod_init(): gpio mapped to IRQ 185
[22:35:08.265] Feb  9 21:35:08 ctrl001 kernel: [ 3354.763233] gpio_irq_handler(): called, as triggered by IRQ
[22:35:12.973] Feb  9 21:35:12 ctrl001 kernel: [ 3354.851957] gpio_irq_handler(): called, as triggered by IRQ

```

## References
- https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
