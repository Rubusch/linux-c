# PIC32MX470

## MPLAB X IDE

-> See my notes on basic PIC32 setup.  


## PIC32 Code

#### PIC32MX: Microchip's USB Demo Projects

There are USB HID demos, try to verify the setup by building them and playing them on the PIC32MX470, if e.g. `hid_basic_demo` works out, the setup is ok.  

For the Microchip demos, build the USB Device Library Application using the _M_PLAB _H_armony _C_onfigurator (MHC). Either implement yourself, or use the provided demos as a starting point, e.g.
`./harmony/harmony3/usb_apps_device/apps/hid_basic/firmware/pic32mx470_curiosity.X/nbproject/project.xml` (watch out for a "project.xml")  

## USB HID Demo

This is +/- how the setup of the USB HID Demo as a new project. Type `CTRL + SHIFT + n` (or `File -> New Project`) to start a new project.  

![Setup0010](./pics/0010.jpg)  
![Setup0020](./pics/0020.jpg)  
![Setup0030](./pics/0030.jpg)  
![Setup0040](./pics/0040.jpg)  
![Setup0050](./pics/0050.jpg)  
![Setup0060](./pics/0060.jpg)  
![Setup0070](./pics/0070.jpg)  
![Setup0080](./pics/0080.jpg)  
![Setup0090](./pics/0090.jpg)  
![Setup0100](./pics/0100.jpg)  
![Setup0110](./pics/0110.jpg)  
![Setup0120](./pics/0120.jpg)  
![Setup0130](./pics/0130.jpg)  
![Setup0140](./pics/0140.jpg)  
![Setup0150](./pics/0150.jpg)  
![Setup0160](./pics/0160.jpg)  
![Setup0170](./pics/0170.jpg)  
![Setup0180](./pics/0180.jpg)  
![Setup0190](./pics/0190.jpg)  
![Setup0200](./pics/0200.jpg)  
![Setup0210](./pics/0210.jpg)  
![Setup0220](./pics/0220.jpg)  
![Setup0230](./pics/0230.jpg)  
![Setup0240](./pics/0240.jpg)  


#### Source Organization

- Find the implementation in `app.c`, `app.h` and `main.c`
- The platform here is `default`, it is not supposed to be modified
- Find available LED instructions in `./firmware/src/config/default/bsp/bsp.h`
- USB device driver is prepared also in config/default

This Implementation registers for USB read requests directly, instead of setting up yet another state machine, then going into a USB-wait-for-data-state  


#### One word about MHC / Clocking plugin

After the MCC setup was setup, or anyhow adjusted, you need to `generate` the setup. Only this will generate the code. In case a meld-based diff will pop up to accept the design modifications to the project `src`. Example, the clock settings are by default turned off, no divider are seetup and USB won't clocked. This can be adjusted in the `plugin -> Clock Settings`. This is _not_ sufficient. Do the `generate` and accept the settings e.g. into the `./usb/initialization.c` file.  


## Flashing the Demo to the Target

TODO        

## References
* https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/UserGuides/MPLABHarmonyConfiguratorUsersGuide_v111.pdf  
