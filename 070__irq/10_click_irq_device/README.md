# IRQ Click Device

  An intertupt will be generated and handled, a message will be
  printed out to the console, each a button is pressed. In the
  handler, the device structure will be recovered, which is used as a
  parameter in the `dev_info()` function then.

## Hardware: MikroE Button-R-Click

![button-r-click](pics/mikroe-button-r-click.png)

https://www.mikroe.com/button-r-click

Connection:  
- GPIO23 -> INT
- 3v3 -> 3v3
- GND -> GND

![setup](pics/setup-interrupt-button.png)


# Build

## Module

Should crosscompile - having crossbuild-essentials-arm64 installed,
ARCH, and `CROSS_COMPILE` set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Usage

Load the module, and press the button several times, watch as the
interrupts counter increases on several cores.  
```
# insmod ./irq_click.ko

# watch "cat /proc/interrupts"
    ...
    185:          1          2          0          2  pinctrl-bcm2835  23 Edge      LOTHARS_KEY
    IPI0:       288        314        263        282       Rescheduling interrupts
    IPI1:      3279       5267       4078       3627       Function call interrupts
    ...

# rmmod irq_click
```

In another shell watch the log  
```
$ tail -f /var/log/messages
    ...
## loading of the module
    Oct 26 22:25:49 ctrl001 kernel: [  132.224435] irq_click: loading out-of-tree module taints kernel.
    Oct 26 22:25:49 ctrl001 kernel: [  132.226234] intkey soc:int_key: lothars_probe() - called
    Oct 26 22:25:49 ctrl001 kernel: [  132.226531] intkey soc:int_key: lothars_probe() - irq number is '185' by gpiod_to_irq()
    Oct 26 22:25:49 ctrl001 kernel: [  132.226624] intkey soc:int_key: lothars_probe() - irq number is '185' by platform_get_irq()
    Oct 26 22:25:49 ctrl001 kernel: [  132.230099] intkey soc:int_key: lothars_probe() - got minor 121
    Oct 26 22:25:49 ctrl001 kernel: [  132.230143] intkey soc:int_key: lothars_probe() - done

## press the button
    Oct 26 22:26:26 ctrl001 kernel: [  169.591100] intkey soc:int_key: lothars_isr() - interrupt received, key: 'LOTHARS_KEY'
    Oct 26 22:26:30 ctrl001 kernel: [  172.994055] intkey soc:int_key: lothars_isr() - interrupt received, key: 'LOTHARS_KEY'
    Oct 26 22:26:31 ctrl001 kernel: [  174.756610] intkey soc:int_key: lothars_isr() - interrupt received, key: 'LOTHARS_KEY'
    Oct 26 22:26:34 ctrl001 kernel: [  177.906356] intkey soc:int_key: lothars_isr() - interrupt received, key: 'LOTHARS_KEY'
    Oct 26 22:26:36 ctrl001 kernel: [  179.294242] intkey soc:int_key: lothars_isr() - interrupt received, key: 'LOTHARS_KEY'

## unloading
    Oct 26 22:27:35 ctrl001 kernel: [  238.003566] intkey soc:int_key: lothars_remove() - called
```

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 274  
