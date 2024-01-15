# PIC32MX470: led demos

Kernel Config: make sure the following is set:  
- CONFIG_HID_SUPPORT
- CONFIG_HID_GENERIC


[USB is a huge topic, here just some general/particular notes]  

#### USB Fundamentals

Information is reuquested in terms of **descriptors**. USB descriptors are data structures that are provided by devices to describe all of their attributes. This includes e.g. the product/vendor ID, any device class affiliation and strings describing the product and vendor.  

Endpoints can be categorized into **control** and **data** endpoints. Every USB device must provide at least one control endpoint at address 0 called the default endpoint. The endpoint number is a 4-bit integer associated with an endpoint (0-15); the same endpoint number is used to describe two endpoints, for instance EP1 IN and EP1 OUT. Examples: `EP1 IN = 0x81`, `EP1 OUT = 0x01`, `EP3 IN = 0x83` or `EP3 OUT = 0x03`.  

#### USB Data Transfers

- Control Transfer: Used to configure a device at attach and can be used for other device-specific purposes, such as specific register read/write access, or control of other pipes on the device
- Bulk Data Transfer: Transfer of relatively large quantities of data or data bursts; bulk transfers do not have guaranteed timing, but can provide the fastest data transfer rates
- Interrupt Data Transfer: Used for reliable delivery of data, e.g. characters or coordinates with human-perceptible echo or feedback response characteristics; USB mice and keyboards typically use interrupt data transfers
- Isochronous Data Transfer: Isochronous transfers have guaranteed timing, but do _not_ have error correction capability; isochronous data must be delivered at the rate received to maintain its timing and additionally may be sensitive to delivery delay; typically used for streaming audio or video

#### USB Request Block (URB)

Any communication between the host and device is done asynchronously
by using USB Request Blocks (URBs).   

- An URB consists of all relevant information to execute any USB transaction and deliver the data and status back.
- Execution of an URB is inherently an asynchronous operation, i.e. the `usb_submit_urb()` call returns immediately after it has successfully queued the requested action.
- Transfers for one URB can be canceled with `usb_unlink_urb()` at any time.
- Each URB has a completion handler, which is called after the action has been successfully completed or canceled. The URB also contains a context-pointer for passing information to the completion handler.
- Each endpoint for a device logically supports a queue of requests. You can fill that queue so that the USB hardware can still transfer data to an endpoint while your driver handles the completion of another. This maximizes use of USB bandwidth and supports seamless streaming of data to (or from) devices when using periodic transfer modes.


## Hardware: Microchip Curiosity PIC32MX470 (PIC32MX470512H)

PIC32 Board:  
https://www.microchip.com/DevelopmentTools/ProductDetails/dm320103

Connection:  
Connect the USB Micro-B port (J12) of the PIC32MX470 Curiosity
Development Board to the J19 USB-B type C connector. In case this will
need a USB type-C male to micro-B male cable.  

TODO    


## Software: MPLAB X IDE

MPLAB X IDE: https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs

```
$ sudo apt install -y libusb-1.0-0

$ tar xf ./MPLABX-v6.15-linux-installer.tar
$ sudo ./MPLABX-v6.15-linux-installer.sh
-> only select MPLAB X IDE, and
-> PIC32 support
```

## Setup the HID Application in the MPLAB IDE

TODO       

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
TODO       
```

Logs   
```
TODO      
```

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 594ff, p. 616  
