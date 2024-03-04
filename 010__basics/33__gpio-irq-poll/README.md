# GPIO: Convert Input to IRQ and let a Userspace App poll on a Device Node

The demo shows incomming asynchronous events on a gpio line. The events on the GPIO line will provoke the generation of an IRQ. The kernel module is a Chardev driver (miscdevice) thus provides a device node, sysfs class representation, and a `cdev` instance.  

## Setup

Provoke the input signal, simply wire the *GPIO26* to 3.3V.  

## Usage

```
# insmod gpio_irq_poll.ko
# ./gpio_poller_app.elf
    gpio_poller_app.c: wait for signal...
```
Connect `GPIO26` to 3.3V of the board.
```
    gpio_poller_app.c: button pressed
# rmmod gpio_irq_poll.ko
```

Logs  
```
[16:12:17.373] Feb 11 04:16:09 ctrl001 kernel: [21556.797189] mod_init(): called
[16:12:17.373] Feb 11 04:16:09 ctrl001 kernel: [21556.798286] mod_init(): gpio 26 is mapped to irq 185
[16:12:20.683] Feb 11 04:16:12 ctrl001 kernel: [21560.112828] gpiodev_poll(): called

<connect wire from GPIO25 on RPI3b to 3.3V>

[16:12:30.696] Feb 11 04:16:22 ctrl001 kernel: [21570.121423] gpiodev_irq_handler(): called by irq
[16:12:30.696] Feb 11 04:16:22 ctrl001 kernel: [21570.121555] gpiodev_poll(): called
[16:12:32.832] Feb 11 04:16:24 ctrl001 kernel: [21572.257419] gpiodev_irq_handler(): called by irq
[16:15:05.963] Feb 11 04:18:57 ctrl001 kernel: [21725.392110] mod_exit(): called
```

## References
- https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
- the idea is mainly taken from here: https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
