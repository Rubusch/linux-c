# PIC32MX470: usb hid demo using urb packages

The communication between the host (RPI) and the devie (PIC32) here is
done asynchronously by using USB request blocks (URBs).  

Kernel Config: make sure the following is set:  
- `CONFIG_HID_SUPPORT`
- `CONFIG_HID_GENERIC`
Make sure the following is _NOT_ set (or at least as module, then
unload the module). Alternatively a udev rule has to be in place
additionally to allow for USB matching:  
- `CONFIG_USB_HID`

## Hardware: Microchip Curiosity PIC32MX470 (PIC32MX470512H)

Connection:  
Connect the USB Micro-B port (J12) of the PIC32MX470 Curiosity
Development Board to the J19 USB-B type C connector. In case this will
need a USB type-C male to micro-B male cable.  

#### MPLAB X IDE Setup

Figure out the installation of the IDE [here](../pic32mx470__00-basics/README.md).  

#### Setup the HID Application in the MPLAB IDE

Use the [USB HID demo](../pic32mx470__01-hid-demo).  

# Build

## Module

Having crossbuild-essentials-arm64 installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module__ltc2607-dual-dac
$ make
```
Copy the module over to the target  

## Usage

```
# insmod ./pic32mx470-led.ko
#
# echo 1 > /sys/bus/usb/devices/1-1.4\:1.0/led
#
# echo 1 > /sys/bus/usb/devices/1-1.4\:1.0/led
# echo 2 > /sys/bus/usb/devices/1-1.4\:1.0/led
# echo 3 > /sys/bus/usb/devices/1-1.4\:1.0/led
#
# echo 0 > /sys/bus/usb/devices/1-1.4\:1.0/led
# echo 0 > /sys/bus/usb/devices/1-1.4\:1.0/led
# echo 0 > /sys/bus/usb/devices/1-1.4\:1.0/led
#
# rmmod pic32mx470-led.ko
```

Logs   
```
Jan 28 08:19:55 ctrl001 kernel: [27635.731001] usbcore: registered new interface driver lothars_usbled

Jan 28 08:19:59 ctrl001 kernel: [27639.415942] usb 1-1.4: new full-speed USB device number 24 using dwc_otg
Jan 28 08:19:59 ctrl001 kernel: [27639.518941] usb 1-1.4: New USB device found, idVendor=04d8, idProduct=003f, bcdDevice= 1.00
Jan 28 08:19:59 ctrl001 kernel: [27639.518969] usb 1-1.4: New USB device strings: Mfr=1, Product=2, SerialNumber=0
Jan 28 08:19:59 ctrl001 kernel: [27639.518983] usb 1-1.4: Product: Simple HID Device Demo
Jan 28 08:19:59 ctrl001 kernel: [27639.518996] usb 1-1.4: Manufacturer: Lothars USB HID Demo
Jan 28 08:19:59 ctrl001 kernel: [27639.520165] lothars_usbled 1-1.4:1.0: usbled_probe() - called
Jan 28 08:19:59 ctrl001 kernel: [27639.520193] lothars_usbled 1-1.4:1.0: usbled_probe() - usb_endpoint_num() = 1 [1] - endpoint number
Jan 28 08:19:59 ctrl001 kernel: [27639.520207] lothars_usbled 1-1.4:1.0: usbled_probe() - usb_endpoint_maxp() = 64 [?] - endpoint size
Jan 28 08:19:59 ctrl001 kernel: [27639.520222] lothars_usbled 1-1.4:1.0: usbled_probe() - endpoint IN address is 0x81
Jan 28 08:19:59 ctrl001 kernel: [27639.520235] lothars_usbled 1-1.4:1.0: usbled_probe() - endpoint OUT address is 0x01
Jan 28 08:19:59 ctrl001 kernel: [27639.520280] lothars_usbled 1-1.4:1.0: usbled_probe() - interrupt_in_urb submitted
Jan 28 08:19:59 ctrl001 mtp-probe: checking bus 1, device 24: "/sys/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.4"
Jan 28 08:19:59 ctrl001 mtp-probe: bus: 1, device: 24 was not an MTP device
Jan 28 08:20:00 ctrl001 mtp-probe: checking bus 1, device 24: "/sys/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.4"
Jan 28 08:20:00 ctrl001 mtp-probe: bus: 1, device: 24 was not an MTP device

Jan 28 08:20:12 ctrl001 kernel: [27652.121522] lothars_usbled 1-1.4:1.0: led_store() - called
Jan 28 08:20:12 ctrl001 kernel: [27652.121573] lothars_usbled 1-1.4:1.0: led_store() - led = 1
Jan 28 08:20:12 ctrl001 kernel: [27652.186080] usb 1-1.4: led_urb_out_callback() - called

Jan 28 08:20:16 ctrl001 kernel: [27656.401945] lothars_usbled 1-1.4:1.0: led_store() - called
Jan 28 08:20:16 ctrl001 kernel: [27656.401996] lothars_usbled 1-1.4:1.0: led_store() - led = 1
Jan 28 08:20:16 ctrl001 kernel: [27656.467085] usb 1-1.4: led_urb_out_callback() - called

Jan 28 08:20:21 ctrl001 kernel: [27661.459139] lothars_usbled 1-1.4:1.0: led_store() - called
Jan 28 08:20:21 ctrl001 kernel: [27661.459192] lothars_usbled 1-1.4:1.0: led_store() - led = 2
Jan 28 08:20:21 ctrl001 kernel: [27661.524085] usb 1-1.4: led_urb_out_callback() - called

Jan 28 08:20:27 ctrl001 kernel: [27667.542249] lothars_usbled 1-1.4:1.0: led_store() - called
Jan 28 08:20:27 ctrl001 kernel: [27667.542305] lothars_usbled 1-1.4:1.0: led_store() - led = 3
Jan 28 08:20:27 ctrl001 kernel: [27667.607087] usb 1-1.4: led_urb_out_callback() - called
```
Press and release the switch S1, and check the logs   
```
Jan 28 08:20:36 ctrl001 kernel: [27676.295224] lothars_usbled 1-1.4:1.0: led_store() - called
Jan 28 08:20:36 ctrl001 kernel: [27676.295274] lothars_usbled 1-1.4:1.0: led_store() - read status
Jan 28 08:20:36 ctrl001 kernel: [27676.360084] usb 1-1.4: led_urb_out_callback() - called
Jan 28 08:20:36 ctrl001 kernel: [27676.385080] usb 1-1.4: led_urb_in_callback() - called
Jan 28 08:20:36 ctrl001 kernel: [27676.385143] usb 1-1.4: led_urb_in_callback() - switch is OFF

Jan 28 08:20:42 ctrl001 kernel: [27682.118680] lothars_usbled 1-1.4:1.0: led_store() - called
Jan 28 08:20:42 ctrl001 kernel: [27682.118730] lothars_usbled 1-1.4:1.0: led_store() - read status
Jan 28 08:20:42 ctrl001 kernel: [27682.183073] usb 1-1.4: led_urb_out_callback() - called
Jan 28 08:20:42 ctrl001 kernel: [27682.210080] usb 1-1.4: led_urb_in_callback() - called
Jan 28 08:20:42 ctrl001 kernel: [27682.210147] usb 1-1.4: led_urb_in_callback() - switch is ON

Jan 28 08:20:49 ctrl001 kernel: [27689.811471] lothars_usbled 1-1.4:1.0: led_store() - called
Jan 28 08:20:49 ctrl001 kernel: [27689.811523] lothars_usbled 1-1.4:1.0: led_store() - read status
Jan 28 08:20:49 ctrl001 kernel: [27689.876078] usb 1-1.4: led_urb_out_callback() - called
Jan 28 08:20:49 ctrl001 kernel: [27689.891090] usb 1-1.4: led_urb_in_callback() - called
Jan 28 08:20:49 ctrl001 kernel: [27689.891188] usb 1-1.4: led_urb_in_callback() - switch is OFF
Jan 28 08:21:07 ctrl001 kernel: [27707.484278] usbcore: deregistering interface driver lothars_usbled
Jan 28 08:21:07 ctrl001 kernel: [27707.484387] usb 1-1.4: led_urb_in_callback() - called
Jan 28 08:21:07 ctrl001 kernel: [27707.484399] usb 1-1.4: led_urb_in_callback() - switch is OFF
Jan 28 08:21:07 ctrl001 kernel: [27707.484440] lothars_usbled 1-1.4:1.0: usbled_disconnect() - called
Jan 28 08:21:07 ctrl001 kernel: [27707.484459] lothars_usbled 1-1.4:1.0: usbled_disconnect() - usb led is now disconnected
```

## References
* https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide
* https://www.microchip.com/DevelopmentTools/ProductDetails/dm320103
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 617ff, p. 628  
