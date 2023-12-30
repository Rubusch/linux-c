# Chardev Module with Devicetree Binding (open firmware)

For the RPI3b: add the keys with the following DT binding. NB: There is an arm
and an arm64 version, anyway the 64 bit version  diverts to the 32 version
(v6.3.1).  
```
$ vi ./linux/arch/arm/boot/dts/bcm2710-rpi-3-b.dts
    ...
	&soc {
		virtgpio: virtgpio {
			compatible = "brcm,bcm2835-virtgpio";
			gpio-controller;
			#gpio-cells = <2>;
			firmware = <&firmware>;
			status = "okay";
		};

	    expgpio: expgpio {
			comatible = "brcm,bcm2835-expgpio";
			gpio-controller;
			#gpio-cells = <2>;
			firmware = <&firmware>;
			status = "okay";
		};
		chardev_keys {
			comatible = "lothars,chardev_keys";
		};
		...
```
Note the naming conventions when working with the devicetree:  
* `pin`: board / header, a pin represents a physical input or output carrying an electrical signal. Every input or output signal goes throuth a physical pin from or into a component.
* `pad`: the contact; Each pad has a logical/canonical name; this name is shown on the schematic symbol inside the part, enxt to the pin and pin number. This pad name typically corresponds to the first pad functionality.
* `pin muxing`: refers to the devicetree
* `net name`: The schematics assign a net name to the functional wire connected to the pad. This attempts to provide a description of what the wire is actually used for. the net name is usually the same as or similar to the pad multiplexing name.


IOMUX / The LInux user space naming convention:  
* Almost every pad has a GPIO function as one of its up to eight potential iomux modes.
* Linux uses a single integer to enumerate all pads, therefore e.g. NXP's bank/bit notation for GPIOs must be mapped.
* The bank/bit to Linux user space formula is: `linux gpio number = (gpio_bank - 1) * 32 + gpio_bit`, e.g. GPIO4_IO19 maps to `(4-1) * 32 + 19 = 115`

[further details on IOMUX on NPX's iMX7D p125ff -> references]

## Linux

```
$ cd linux
$ make dtbs
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module
Should compile cross - having crossbuild-essentials-arm64 installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace
Compile cross, then copy the .elf over to the target.   
```
rpi$ cd ./userspace
rpi$ make
```

## Usage
On the target perform the following to verify the functionality  
```
pi@raspberrypi:~$ sudo insmod chardev.ko

pi@raspberrypi:~$ sudo find /sys -name "*lothar*"
    /sys/bus/platform/drivers/lotharskeys
    /sys/module/chardev/drivers/platform:lotharskeys

pi@raspberrypi:~$ sudo rmmod chardev
```

## Verified
* Verified against a RPI3 w/ aarch64

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
