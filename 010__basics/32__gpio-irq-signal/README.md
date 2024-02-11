# gpio irq

The demo shows incomming asynchronous events on a gpio line. Then
reacts by sending a signal to a userspace application.  

## Setup

To _trigger the button_ simply wire the *gpio17* to 3.3V.  

## Usage

```
# insmod ./gpio_irq_signaler.ko
# ./signalee.elf
    signalee.c: pid: 1517
    signalee.c: wait for signal...
```
Now press the button, or connect gpio17 to 3.3V of the board.  
```
    signalee.c: button pressed
    signalee.c: button pressed
    signalee.c: button pressed
    signalee.c: button pressed
    ...
# rmmod gpio_irq_signaler.ko
```
Logs  
```
[12:02:26.138] Feb 11 00:06:18 ctrl001 kernel: [ 6565.505405] mod_init(): called
[12:02:26.138] Feb 11 00:06:18 ctrl001 kernel: [ 6565.506029] mod_init(): gpio 17 is mapped to irq 185

[12:02:32.145] Feb 11 00:06:24 ctrl001 kernel: [ 6571.512328] gpio_ioctl(): called
[12:02:32.145] Feb 11 00:06:24 ctrl001 kernel: [ 6571.512368] gpio_ioctl(): registered userspace application w/ pid 1517
[12:02:48.809] Feb 11 00:06:40 ctrl001 kernel: [ 6588.184449] gpio_irq_signal_handler(): called - irq triggered
[12:02:48.886] Feb 11 00:06:40 ctrl001 kernel: [ 6588.239525] gpio_irq_signal_handler(): called - irq triggered
[12:02:48.887] Feb 11 00:06:40 ctrl001 kernel: [ 6588.239582] gpio_irq_signal_handler(): called - irq triggered
[12:02:48.887] Feb 11 00:06:40 ctrl001 kernel: [ 6588.247376] gpio_irq_signal_handler(): called - irq triggered
...
[12:02:49.952] Feb 11 00:06:40 ctrl001 kernel: [ 6588.445907] gpio_irq_signal_handler(): called - irq triggered
[12:03:01.976] Feb 11 00:06:53 ctrl001 kernel: [ 6601.364925] mod_exit(): called
```

## References
- https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
- the idea is mainly taken from here: https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
