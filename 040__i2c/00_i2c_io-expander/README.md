# I2C Client Demo

If you write a driver for an I2C device, try to use the SMBus commands
if at all possible (if the device uses only that subset of the I2C
protocol). This makes it possible to use the device driver on both
SMBus adapters and I2C adapters (the SMBus command set is
automatically translated to I2C on I2C adapters, but plain I2C
commands cannot be handled at all on most pure SMBus adapters.  

## Hardware: PCF8574 I/O Expander Device

PCF8574 I2C Portexpander I/O Erweiterung 8 Pin  
https://www.bastelgarage.ch/pcf8574-i2c-portexpander-i-o-erweiterung-8-pin?search=PCF8574%20I2C%20Portexpander%20I%2FO%20Erweiterung%208%20Pin

connect:  

- GPIO2   -> SDA1
- GPIO3   -> SCL1
- VCC3.3V -> 3.3V
- GND     -> GND

## The Linux I2C Subsystem

1. The I2C bus core: The core of the I2C subsystem is located in the i2c-core.c file under the `drivers/i2c/` directory. It is a collection of code that provides interface support between an individual client driver and some I2C bus masters such as the iMX7D I2C controllers. It manages bus arbitration, retry handling and various otehr protocol details. It is registered with the `bus_register()` function and declares the I2C `bus_type` structure. The I2C core API is then a set of functions used for an I2C client device driver to send and receive.  

2. The I2C controller drivers: Find the I2C controller drivers under `drivers/i2c/busses/`. The I2C controller is a platform device that must be registered as a device to the platform bus. The I2C controller driver is a set of custom functions that issues read/writes to the I2C controller hardware I/O addresses of the SoC. There is a specific code for each I2C controller on the processor. These specific functions are called by the I2C core API when this invokes the `adap_algo_master_xfer()` after an I2C client driver has initiated an `i2c_transfer()`. In an I2C controller driver for example, you have to declare a private structure that includes an `i2c_adapter` structure.  

# Build

## Devicetree

The demo uses the GPIO expansion connector, GPIO2 and GPIO3 pins to the corresponding pads.  

NB: The GPIO2 and GPIO3 pins are set to the ALT0 function.  
```
    ...
    &i2c1 {
        pinctrl-names = "default";
        pinctrl-0 = <&i2c1_pins>;
        clock-frequency = <100000>;
        status = "okay";
        ...
        ioexp#38 {
            compatible = "lothars,ioexp";
            reg = <&0x38>;
        };

        ioexp@39 {
            compatible = "lothars,ioexp";
            reg = <0x39>;
        };
        ...
    };
    ...
```

A copy of the modified DTS is provided, copy it to the specified location in the linux sources (6.3), then build it.  

```
$ cd linux
$ cp -arf ~/workspace/lothars-modules/030__platform/devicetree_binding_uio_led/devicetree/arch ./

$ make dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module
Should crosscompile - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Usage

```
TODO      
```

## Verified
* Verified against a RPI3b w/ aarch64  

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 220  
