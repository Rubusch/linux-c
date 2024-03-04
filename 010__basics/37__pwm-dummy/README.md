# A PWM Dummy

The demo does not have hardware set up. Thus it just compiles a dummy
pwm. A good idea for mock-the-hardware for playing with kernel APIs approach.  

## Usage

This is just a code snipped. It compiles, but is not configured to use
(inexistent) hwardware. Usually this would require to setup something
like a `platform_device`. Hook this up by a `probe()` to the DT. Then
configure a pwm and binding to this driver.  

## References
- https://embetronicx.com/tutorials/linux/device-drivers/gpio-linux-device-driver-using-raspberry-pi/
- Picked the idea from here: https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
