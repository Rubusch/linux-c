# DC934a - Subsystem DAC (LTC2607)

![DC934a Module](pics/dc934a.png)  


iio: The Industrial I/O core offers a unified framework for writing
drivers for many different types of embedded sensors.  

input: Some of the parts might be also used in input drivers, such as
for mouse, joystick, etc.  

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


#### Modification:
Solder out the device _U7_ from the DC934A board and also the LTC2607 I2C pull-up resistors: _R10_ and _R11_  

![DC934a Schema](pics/dc934a_schema.png)  
![DC934a Modification](pics/dc934a_modified.png)  

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

![DC934a Connection](pics/dc934a_connected.png)  
![DC934a RPI](pics/rpi_connected.png)  

# Build

## Module

Having crossbuild-essentials-arm64 installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module__ltc2607-dual-dac
$ make
```
Copy the module over to the target. Copy the `.dtbo` file over to `/boot/overlays`, and register it in `/boot/config.txt` as `dtoverlay`.  

## Userspace
Compile cross, then copy the .elf over to the target.   
```
rpi$ cd ./userspace__ltc2422-adc
rpi$ make
```

## Usage

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
    ltc2422_read() - value: 200064
    ltc2422_read() - adc_channel: 0
    read_adc() - adc_channel 0
    ltc2422_read() - value: 6ffad3
    ltc2422_read() - adc_channel: 1
    ltc2422_read() - value: 2ff916
    ltc2422_read() - adc_channel: 0
    	ADC A : 4.9937
    	ADC B : 4.9916
    READY.

# echo 0 > /sys/bus/iio/devices/iio\:device1/out_voltage0_raw
# ./iio_spidev.elf
    read_adc() - adc_channel 0
    ltc2422_read() - value: 6ffae7
    ltc2422_read() - adc_channel: 1
    read_adc() - adc_channel 1
    ltc2422_read() - value: 2ff901
    ltc2422_read() - adc_channel: 0
    ltc2422_read() - value: 6000d8
    ltc2422_read() - adc_channel: 1
    	ADC A : 0.0010
    	ADC B : 4.9915
    READY.

# echo 0 > /sys/bus/iio/devices/iio\:device1/out_voltage1_raw
# ./iio_spidev.elf
    read_adc() - adc_channel 0
    ltc2422_read() - value: 2ff914
    ltc2422_read() - adc_channel: 0
    read_adc() - adc_channel 0
    ltc2422_read() - value: 6000ce
    ltc2422_read() - adc_channel: 1
    ltc2422_read() - value: 20008d
    ltc2422_read() - adc_channel: 0
    	ADC A : 0.0010
    	ADC B : 0.0007
    READY.

# rmmod iio-ltc2607-dac
```

Follow the logs   
```
Jan  6 19:29:35 ctrl001 kernel: [25963.018180] ltc2607 1-0072: ltc2607_probe() - called
Jan  6 19:29:35 ctrl001 kernel: [25963.018237] ltc2607 1-0072: ltc2607_probe() - was called from DAC00
Jan  6 19:29:35 ctrl001 kernel: [25963.018945] ltc2607 1-0072: ltc2607_probe() - the DAC answer is '3'
Jan  6 19:29:35 ctrl001 kernel: [25963.019375] ltc2607 1-0072: ltc2607_probe() - ltc2607 DAC registered
Jan  6 19:29:35 ctrl001 kernel: [25963.019907] ltc2607 1-0073: ltc2607_probe() - called
Jan  6 19:29:35 ctrl001 kernel: [25963.019954] ltc2607 1-0073: ltc2607_probe() - was called from DAC01
Jan  6 19:29:35 ctrl001 kernel: [25963.022341] ltc2607 1-0073: ltc2607_probe() - the DAC answer is '3'
Jan  6 19:29:35 ctrl001 kernel: [25963.022933] ltc2607 1-0073: ltc2607_probe() - ltc2607 DAC registered

Jan  7 17:16:28 ctrl001 kernel: [  630.925472] ltc2607 1-0073: ltc2607_write_raw() - called
Jan  7 17:16:28 ctrl001 kernel: [  630.925518] ltc2607 1-0073: ltc2607_write_raw() - case IIO_CHAN_INFO_RAW: val '65535', chan->channel '2'
Jan  7 17:16:28 ctrl001 kernel: [  630.925551] ltc2607 1-0073: ltc2607_set_value() - called
Jan  7 17:16:28 ctrl001 kernel: [  630.925575] ltc2607 1-0073: ltc2607_set_value() - val '65535', channel '2'
Jan  7 17:16:28 ctrl001 kernel: [  630.925602] ltc2607 1-0073: ltc2607_set_value() - chan '0f' [0x00: DACa, 0x01: DACb, 0x0f: both DACs]
Jan  7 17:16:28 ctrl001 kernel: [  630.925629] ltc2607 1-0073: ltc2607_set_value() - outbuf[0] '3f'
Jan  7 17:16:28 ctrl001 kernel: [  630.925654] ltc2607 1-0073: ltc2607_set_value() - outbuf[1] 'ff'
Jan  7 17:16:28 ctrl001 kernel: [  630.925679] ltc2607 1-0073: ltc2607_set_value() - outbuf[2] 'ff'

Jan  7 17:17:24 ctrl001 kernel: [  686.833720] ltc2607 1-0073: ltc2607_write_raw() - called
Jan  7 17:17:24 ctrl001 kernel: [  686.833748] ltc2607 1-0073: ltc2607_write_raw() - case IIO_CHAN_INFO_RAW: val '0', chan->channel '0'
Jan  7 17:17:24 ctrl001 kernel: [  686.833771] ltc2607 1-0073: ltc2607_set_value() - called
Jan  7 17:17:24 ctrl001 kernel: [  686.833784] ltc2607 1-0073: ltc2607_set_value() - val '0', channel '0'
Jan  7 17:17:24 ctrl001 kernel: [  686.833797] ltc2607 1-0073: ltc2607_set_value() - chan '00' [0x00: DACa, 0x01: DACb, 0x0f: both DACs]
Jan  7 17:17:24 ctrl001 kernel: [  686.833810] ltc2607 1-0073: ltc2607_set_value() - outbuf[0] '30'
Jan  7 17:17:24 ctrl001 kernel: [  686.833823] ltc2607 1-0073: ltc2607_set_value() - outbuf[1] '00'
Jan  7 17:17:24 ctrl001 kernel: [  686.833835] ltc2607 1-0073: ltc2607_set_value() - outbuf[2] '00'

Jan  7 17:18:28 ctrl001 kernel: [  750.792575] ltc2607 1-0073: ltc2607_write_raw() - called
Jan  7 17:18:28 ctrl001 kernel: [  750.792622] ltc2607 1-0073: ltc2607_write_raw() - case IIO_CHAN_INFO_RAW: val '0', chan->channel '1'
Jan  7 17:18:28 ctrl001 kernel: [  750.792653] ltc2607 1-0073: ltc2607_set_value() - called
Jan  7 17:18:28 ctrl001 kernel: [  750.792678] ltc2607 1-0073: ltc2607_set_value() - val '0', channel '1'
Jan  7 17:18:28 ctrl001 kernel: [  750.792704] ltc2607 1-0073: ltc2607_set_value() - chan '01' [0x00: DACa, 0x01: DACb, 0x0f: both DACs]
Jan  7 17:18:28 ctrl001 kernel: [  750.792731] ltc2607 1-0073: ltc2607_set_value() - outbuf[0] '31'
Jan  7 17:18:28 ctrl001 kernel: [  750.792768] ltc2607 1-0073: ltc2607_set_value() - outbuf[1] '00'
Jan  7 17:18:28 ctrl001 kernel: [  750.792793] ltc2607 1-0073: ltc2607_set_value() - outbuf[2] '00'

Jan  7 17:19:18 ctrl001 kernel: [  800.697097] ltc2607 1-0073: ltc2607_remove() - called
Jan  7 17:19:18 ctrl001 kernel: [  800.700803] ltc2607 1-0072: ltc2607_remove() - called
```

## Debugging

Studying the datasheet and going through the settings, still a wrong
jumper setting was discovered.  

The setup was carried out as described in the corresponding boot
article. The IIO LTC2607 (dual DAC) via I2C driver can be initialized
and communicates with the board as measured. The `spidev` based
userspace program, communicates with the LTC2422 ADCs as seen in the
output.  

![DC934a Debugging](pics/debug_ltc2607.png)  
![DC934a Signal](pics/debug_ltc2607_signal_received.png)  

There were still some issues with the `spidev` device, sometimes it
appears on `/dev/spidev0.0`, sometimes on `/dev/spidev0.1`. Where the
latter was often not even useable.  

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 494ff, 508ff and 516  
