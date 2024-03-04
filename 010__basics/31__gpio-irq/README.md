# GPIO: input converts to IRQ

The demo shows dealing with incomming asynchronous events on a gpio
line by IRQ generation.  

Note, depending on the framework (sysfs, actually deprecated) or the
currently used GPIO API, we're talking for the RPI3b/4/5 about
`PIN37`, i.e. `GPIO26`, or - e.g. on RPI5 - `gpiochip4 line
26`. Figure out which gpios are around, used, polarity, etc. by using
the package `gpiod`. Then simply execute `gpioinfo` in the shell.  

## Setup

Provoke an input signal by simply wiring the *GPIO26* to 3.3V.  

## Usage

```
# insmod ./gpioirq.ko
```
Now connect GPIO26 to 3.3V of the board, alternatively use a button
for this.  
```
# rmmod ./gpioirq.ko
```

Logs  
```
[22:35:00.036] Feb  9 21:34:59 ctrl001 kernel: [ 3291.863208] mod_exit(): called
[22:35:00.069] Feb  9 21:34:59 ctrl001 kernel: [ 3346.643063] mod_init(): called
[22:35:00.069] Feb  9 21:34:59 ctrl001 kernel: [ 3346.643158] mod_init(): gpio has irq number 185

<connect wire from GPIO26 on RPI3b to 3.3V>

[22:35:08.176] Feb  9 21:35:08 ctrl001 kernel: [ 3346.643232] mod_init(): gpio mapped to IRQ 185
[22:35:08.265] Feb  9 21:35:08 ctrl001 kernel: [ 3354.763233] gpio_irq_handler(): called, as triggered by IRQ
[22:35:12.973] Feb  9 21:35:12 ctrl001 kernel: [ 3354.851957] gpio_irq_handler(): called, as triggered by IRQ

```

## References
- https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
- The idea is mainly originated from: https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
