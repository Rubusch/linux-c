# DC934a - Subsystem DAC (LTC2607)

Provide a driver for the dual DAC LTC2607 (I2C). Then read the analog
outputs from the DAC device by the ADC LTC2422 (SPI). The LTC2607 DAC
outputs are connected to both LTC2422 ADC inputs.  

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

![DC934a Board](pics/dc934a.png)  

#### Modification:
Solder out the device _U7_ from the DC934A board and also the LTC2607 I2C pull-up resistors: _R10_ and _R11_  

![DC934a Board](pics/dc934a_schema.png)  
![DC934a Board](pics/dc934a_modified.png)  

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
- jumper _5V_ at _JP1 VREFA_
- jumper _5V REF_ in _JP2_
- jumper _JP4_, _JP6_, _JP7_ to VCC (connect 1 with middle), i.e. CA0, CA1, CA2 are set to VCC, this matches with the next I2C slave address 0x72

NB: Using the 5V regulator (_5V REG_ in _JP2_) as the source for VCC
has the limitation that VCC may be slightly lower than VREF, which may
affect the full-scale error. Selecting the 5V REF as the source for
VCC overcomes this, however the total current that the LTC2607 can
source will be limited approximately 5mA.  

![DC934a Board](pics/dc934a_connected.png)  
![DC934a Board](pics/rpi_connected.png)  

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
$ cd ./module__ltc2607-dual-dac
$ make
```
Copy the module over to the target  

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

# i2cdetect -l
    i2c-10	i2c       	i2c-11-mux (chan_id 1)          	I2C adapter
    i2c-1	i2c       	bcm2835 (i2c@7e804000)          	I2C adapter
    i2c-11	i2c       	bcm2835 (i2c@7e205000)          	I2C adapter
    i2c-0	i2c       	i2c-11-mux (chan_id 0)          	I2C adapter

# i2cdetect -y 1
         0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    00:                         -- -- -- -- -- -- -- --
    10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    70: -- -- 72 73 -- -- -- --

# insmod ./iio-ltc2607-dac.ko

# ll /sys/bus/iio/devices/
    total 0
    lrwxrwxrwx 1 root root 0 Jan  2 18:43 iio:device0 -> ../../../devices/platform/soc/3f804000.i2c/i2c-1/1-0072/iio:device0
    lrwxrwxrwx 1 root root 0 Jan  2 18:43 iio:device1 -> ../../../devices/platform/soc/3f804000.i2c/i2c-1/1-0073/iio:device1

# echo 65535 > /sys/bus/iio/devices/iio\:device1/out_voltage2_raw

# ./iio_spidev.elf
    read_adc() - adc_channel 0
    ltc2422_read() - value: 2b8034
    ltc2422_read() - adc_channel: 0
    read_adc() - adc_channel 0
    ltc2422_read() - value: 6bee2e
    ltc2422_read() - adc_channel: 1
    ltc2422_read() - value: 2be858
    ltc2422_read() - adc_channel: 0
    	ADC A : 3.7283
    	ADC B : 3.7211
    READY.

# echo 0 > /sys/bus/iio/devices/iio\:device1/out_voltage0_raw 
    root@ctrl001:/home/pi# ./iio_spidev.elf 
    read_adc() - adc_channel 0
    ltc2422_read() - value: 6bee28
    ltc2422_read() - adc_channel: 1
    read_adc() - adc_channel 1
    ltc2422_read() - value: 2be810
    ltc2422_read() - adc_channel: 0
    ltc2422_read() - value: 6becfb
    ltc2422_read() - adc_channel: 1
    	ADC A : 3.7268    <--- should be 0, now
    	ADC B : 3.7208
    READY.

# echo 0 > /sys/bus/iio/devices/iio\:device1/out_voltage1_raw

# ./iio_spidev.elf
    read_adc() - adc_channel 0
    ltc2422_read() - value: 2be88c
    ltc2422_read() - adc_channel: 0
    read_adc() - adc_channel 0
    ltc2422_read() - value: 6beef7
    ltc2422_read() - adc_channel: 1
    ltc2422_read() - value: 2be2b2
    ltc2422_read() - adc_channel: 0
    	ADC A : 3.7292   <--- should be 0
    	ADC B : 3.7142   <--- shoudl be 0 now, too
    READY.

# rmmod iio-ltc2607-dac
```

Follow the logs   
```
Jan  2 18:37:49 ctrl001 kernel: [  553.830688] i2c_dev: i2c /dev entries driver
Jan  2 18:38:10 ctrl001 kernel: [  575.448976] iio_ltc2607_dac: loading out-of-tree module taints kernel.
Jan  2 18:38:10 ctrl001 kernel: [  575.450417] ltc2607 1-0072: ltc2607_probe() - called
Jan  2 18:38:10 ctrl001 kernel: [  575.450475] ltc2607 1-0072: ltc2607_probe() - was called from DAC00
Jan  2 18:38:10 ctrl001 kernel: [  575.451183] ltc2607 1-0072: ltc2607_probe() - the DAC answer is '3'
Jan  2 18:38:10 ctrl001 kernel: [  575.451702] ltc2607 1-0072: ltc2607_probe() - ltc2607 DAC registered
Jan  2 18:38:10 ctrl001 kernel: [  575.452335] ltc2607 1-0073: ltc2607_probe() - called
Jan  2 18:38:10 ctrl001 kernel: [  575.452401] ltc2607 1-0073: ltc2607_probe() - was called from DAC01
Jan  2 18:38:10 ctrl001 kernel: [  575.453239] ltc2607 1-0073: ltc2607_probe() - the DAC answer is '3'
Jan  2 18:38:10 ctrl001 kernel: [  575.453754] ltc2607 1-0073: ltc2607_probe() - ltc2607 DAC registered

Jan  2 18:54:33 ctrl001 kernel: [ 1557.745634] ltc2607 1-0073: ltc2607_remove() - called
Jan  2 18:54:33 ctrl001 kernel: [ 1557.749287] ltc2607 1-0072: ltc2607_remove() - called
```

## Conclusion

The voltage will not change, neither when setting 655335 (0xffff), nor when setting all handles to 0 (0x0).  

The setup was carried out as described in the corresponding boot article. The IIO LTC2607 (dual DAC) via I2C driver can be initialized and communicates with the board as measured. The `spidev` based userspace program, communicates with the LTC2422 ADCs as seen in the output.  
![DC934a Board](pics/debug_ltc2607.png)  
![DC934a Board](pics/debug_ltc2607_signal_received.png)  

The purpose of my experiment was to figure out the Linux kernel driver for IIO API based drivers. This seems basically to work. It looks like either hardware needs still some modification, or might not work as expected.  

For a comparison, `drivers/iio/dac/ad5064.c` handles communication with the LTC2607 but over the Regmap API. Several examples dealing with I2C communication on the IIO API can be found in the kernel sources.  

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 494ff, 508ff and 516  
