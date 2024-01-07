# DC934a - Subsystem DAC (LTC2607)

Provide a driver for the dual DAC LTC2607 (I2C). Then read the analog
outputs from the DAC device by the ADC LTC2422 (SPI). The LTC2607 DAC
outputs are connected to both LTC2422 ADC inputs.  

This setup will use the LTC2607 (I2C) driver, and add another for the
LTC2422 ADC which have two ports. This LTC2422 ADC then will toggle
between first ADC and second ADC.  

#### DAC LTC2607

The LTC2607 is a dual 12-bit, 2.7V to 5.5V rail-to-rail voltage output
DAC. It uses a 2-wire, I2C compatible serial interface. The LTC2607
operates in both the standard mode (clock rate of 100kHz) and the fast
mode (clock rate of 400kHz).  

The driver will control each LtC2607 internal DAC individually or both
DACa + DACb in a simultaneous mode. The IIO framework will generate
three separate sysfs files (attributes) used for sending data to the
dual DAC from the user space application.  


#### ADC LTC2422

The LTC2422 Analog device is a 2-channel 2.7V to 5.5V micropower
20-bit analogt-to-digital converter with an integrated oscillator,
8ppm INL and 1.2ppm RMS noise. The device uses delta-sigma technology
and a new digital filter architecture that settles in a single
cycle. This eliminates the latency found in conventional sigma delta
converters and simplifies multiplexed applications. The converter
accepts an external reference voltage from 0.1V to VCC.  

The LTC2422 serial output data stream is 24 bits long. The first 4
bits represent status information indicating the sign, selected
channel, input range and conversion state. the next 20 bits are the
conversion result, MSB first:  

- Bit 23 (first output bit): The end of conversion (EOC)
  indicator. This bit is available at the SDO pin during the
  conversion and sleep states whenever the CS pin is LOW. This bit
  is HIGH during the conversion and goes LOw when the conversion is
  complete.
- Bit 22 (second output bit): for the LtC2422, this bit is LOW if
  the last conversion was performed on CH0 and HIGH for CH1. This
  bit is always LOW for the LTC2422
- Bit 21 (third output bit): The conversion result sign indicator
  (SIG). If VIN is > 0, this bit is HIGH. If VIN is <0, this bit is
  LOW. The sign bit changes state during the zero code.
- Bit 20 (fourth output bit): The extended input range (EXR)
  indicator. If the input is within the normal input range 0 <= VIN
  <= VREF, this bit is LOW. If the input is outside the normal input
  range, VIN > VREF or VIN < 0, this bit is HIGH.
- Bit 19 (fifth output bit): The most significant bit (MSB).
- Bits 19-0: The 20 bit conversion result MSB first.
- Bit 0: The last significant bit (LSB).

## Hardware: Eval Board dc934a

DC934a Eval Board: https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/dc934a.html

![DC934a Board](../iio__dc934a__ltc2607-dac-ltc2422-adc-via-i2c/pics/dc934a.png)  

#### Modification:
Solder out the device _U7_ from the DC934A board and also the LTC2607 I2C pull-up resistors: _R10_ and _R11_  

![DC934a Board](../iio__dc934a__ltc2607-dac-ltc2422-adc-via-i2c/pics/dc934a_schema.png)  
![DC934a Board](../iio__dc934a__ltc2607-dac-ltc2422-adc-via-i2c/pics/dc934a_modified.png)  

#### Connection:
- 5V -> V+ (connector j1: 1)
- GND -> GND
- GPIO02 (i2c SDA) -> SDA [LTC2607/DAC on the DC934a]
- GPIO03 (i2c SCL) -> SCL
- GPIO08 (SPI_CE0_N) -> CS [LTC2422/ADC on the DC934a]
- GPIO11 (SPI_SCLK) -> SCK
- GPIO09 (SPI_MISO) -> MISO

#### Jumpers:
Use the device's LT1790ACS6-5 5V output (see _U3_ in schema) as the VREF, i.e.  
- jumper _JP1 VREFA_ to _5V_
- jumper _JP2_ to 5V REF
- jumper _JP5_ to _REFLO_
- jumper _JP4_, _JP6_, _JP7_ to VCC (connect 1 with middle), to configure the address i.e. CA0, CA1, CA2 are set to VCC, this matches with the next I2C slave address 0x72

NB: Using the 5V regulator (_5V REG_ in _JP2_) as the source for VCC
has the limitation that VCC may be slightly lower than VREF, which may
affect the full-scale error. Selecting the 5V REF as the source for
VCC overcomes this, however the total current that the LTC2607 can
source will be limited approximately 5mA.  

![DC934a Board](../iio__dc934a__ltc2607-dac-ltc2422-adc-via-i2c/pics/dc934a_connected.png)  
![DC934a Board](../iio__dc934a__ltc2607-dac-ltc2422-adc-via-i2c/pics/rpi_connected.png)  

# Build

## Devicetree

Copy it to the specified location in the linux sources, then build it  
```
$ cd linux
$ cp -arf <SOURCES>/devicetree/arch ./

$ make dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module

Having crossbuild-essentials-arm64 installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module__ltc2422-dual-adc
$ make

$ cd -

$ cd ./module__ltc2607-dual-dac
$ make
```
Copy the module over to the target  

NB: In case the module won't compile (or `make clean` fails) due to the symlink, enter the directory on the absolute path, not the linked path.  

## Userspace
Compile cross, then copy the .elf over to the target.   
```
rpi$ cd ./userspace__ltc2422-adc
rpi$ make
```

## Usage

See the connected Accel Click appears on `1d` (i2c).  
```
$ sudo su
# modprobe i2c-dev
# modprobe industrialio

# insmod ./iio-ltc2607-dac.ko
# insmod ./iio-ltc2422-adc.ko

# ll /sys/bus/iio/devices/
    total 0
    lrwxrwxrwx 1 root root 0 Jan  3 22:38 iio:device0 -> ../../../devices/platform/soc/3f804000.i2c/i2c-1/1-0072/iio:device0
    lrwxrwxrwx 1 root root 0 Jan  3 22:38 iio:device1 -> ../../../devices/platform/soc/3f804000.i2c/i2c-1/1-0073/iio:device1
    lrwxrwxrwx 1 root root 0 Jan  3 22:38 iio:device2 -> ../../../devices/platform/soc/3f204000.spi/spi_master/spi0/spi0.0/iio:device2

# ll /sys/bus/iio/devices/iio\:device2/
    total 0
    -r--r--r-- 1 root root 4096 Jan  3 22:38 name
    lrwxrwxrwx 1 root root    0 Jan  3 22:38 of_node -> ../../../../../../../../firmware/devicetree/base/soc/spi@7e204000/ltc2422@0
    -rw-r--r-- 1 root root 4096 Jan  3 22:38 out_voltage0_raw
    drwxr-xr-x 2 root root    0 Jan  3 22:38 power
    lrwxrwxrwx 1 root root    0 Jan  3 22:38 subsystem -> ../../../../../../../../bus/iio
    -rw-r--r-- 1 root root 4096 Jan  3 22:38 uevent
    -r--r--r-- 1 root root 4096 Jan  3 22:38 waiting_for_supplier

# echo 65535 > /sys/bus/iio/devices/iio\:device1/out_voltage0_raw
```
Note: reading the channel will toggle, read first ADC channel, then read second, then again read first, and so on  
```
# cat /sys/bus/iio/devices/iio\:device2/out_voltage0_raw
    2850328

# cat /sys/bus/iio/devices/iio\:device2/out_voltage0_raw
    7073555

# cat /sys/bus/iio/devices/iio\:device2/out_voltage0_raw
    2875122

# cat /sys/bus/iio/devices/iio\:device2/out_voltage0_raw
    7073567

# ./iio_app.elf
    the value of the ADC channel 0
    	is : 4.6619
    fopen(): No such file or directory
    the value of the ADC channel 0
    	is : 4.6619
    READY.

# rmmod iio-ltc2607-dac.ko
# rmmod iio-ltc2422-adc.ko
```

Follow the logs   
```
Jan  3 22:37:40 ctrl001 kernel: [   62.719644] i2c_dev: i2c /dev entries driver
Jan  3 22:37:59 ctrl001 kernel: [   81.686557] iio_ltc2607_dac: loading out-of-tree module taints kernel.
Jan  3 22:37:59 ctrl001 kernel: [   81.687397] ltc2607 1-0072: ltc2607_probe() - called
Jan  3 22:37:59 ctrl001 kernel: [   81.687432] ltc2607 1-0072: ltc2607_probe() - was called from DAC00
Jan  3 22:37:59 ctrl001 kernel: [   81.687871] ltc2607 1-0072: ltc2607_probe() - the DAC answer is '3'
Jan  3 22:37:59 ctrl001 kernel: [   81.688145] ltc2607 1-0072: ltc2607_probe() - ltc2607 DAC registered
Jan  3 22:37:59 ctrl001 kernel: [   81.688423] ltc2607 1-0073: ltc2607_probe() - called
Jan  3 22:37:59 ctrl001 kernel: [   81.688451] ltc2607 1-0073: ltc2607_probe() - was called from DAC01
Jan  3 22:37:59 ctrl001 kernel: [   81.691176] ltc2607 1-0073: ltc2607_probe() - the DAC answer is '3'
Jan  3 22:37:59 ctrl001 kernel: [   81.691579] ltc2607 1-0073: ltc2607_probe() - ltc2607 DAC registered

Jan  3 22:38:10 ctrl001 kernel: [   92.795908] ltc2422 spi0.0: ltc2422_probe() - called

Jan  3 22:40:24 ctrl001 kernel: [  226.470424] ltc2422 spi0.0: the value is 2b7e18
Jan  3 22:40:38 ctrl001 kernel: [  240.553463] ltc2422 spi0.0: the value is 6bef13
Jan  3 22:40:47 ctrl001 kernel: [  249.264338] ltc2422 spi0.0: the value is 2bdef2
Jan  3 22:40:49 ctrl001 kernel: [  251.172193] ltc2422 spi0.0: the value is 6bef1f

Jan  3 22:42:55 ctrl001 kernel: [  377.474030] ltc2607 1-0073: ltc2607_remove() - called
Jan  3 22:42:55 ctrl001 kernel: [  377.474592] ltc2607 1-0072: ltc2607_remove() - called
```

## TODO

- voltage does not change, figure out why, in case fix

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 494ff, 508ff and 529  
