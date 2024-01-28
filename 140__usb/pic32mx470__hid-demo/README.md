# PIC32MX470

## MPLAB X IDE

Select `PIC32MX470512H` of family `PIC32MX`.  

- MPLAB X IDE: https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs  
- X32 compiler: https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers/xc32  
- MPLAB X Harmony v3: clone from github  


#### Install MPLAB X IDE  
```
$ sudo apt install -y libusb-1.0-0

$ tar xf ./MPLABX-v6.15-linux-installer.tar
$ sudo ./MPLABX-v6.15-linux-installer.sh
-> only select MPLAB X IDE, and
-> PIC32 support
```

#### Install Harmony 3  
```
$ sudo chown <user>:<user> -R /opt/microchip
$ mkdir -p /opt/microchip/harmony && cd $_
$ git clone https://github.com/Microchip-MPLAB-Harmony/mplabx-plugin
```

#### Install the Plugins Setup   

- `Tools -> Plugins -> Downloaded -> Add plugins -> navigate to .nbm file above -> Open -> Install` (with 6.x, I previously found the plugin online under "Available Plugins ")
- `Tools -> Embedded -> MPLAB Harmony 3 Content Manager`
- Download all default packages into a path you choose
- Download `usb` and `usb-device`
- `File -> New Project -> 32 bit MCC Harmony Project` (specify the same path)
- And later, `Tools -> Embedded -> MPLAB Code Configurator v5 Open/Close` (or just click the blue shield icon with MCC)

#### Fix MPLAB X path

For _MPLAB X_ path, clone the MHC stuff into the folling path  
```
$ cd /opt/microchip/harmony/harmony3
$ git clone https://github.com/Microchip-MPLAB-Harmony/mhc.git ./mhc
```
...then select the path to `/opt/microchip/harmony/harmony3` as MPLAB X path, and it should accept it, since the folder `mhc` is around and populated accordingly as subfolder. The MPLAB Content Configuratior (MCC) now should work ...better.  

#### Fix XCLM setuid-root Permissions

Make sure the XCLM is owned by root, and has setuid-bit set. This will prevent a wall of warnings, and not functional toolchain.  
```
$ find ./ -type f -name xclm
    ...
	/opt/microchip/xc32/v4.35/bin/xclm

$ sudo chown root /opt/microchip/xc32/v4.35/bin/xclm
$ sudo chmod 4755 /opt/microchip/xc32/v4.35/bin/xclm
```

#### References: MPLAB X and PIC32MX470  

- Eval board: https://www.microchip.com/en-us/development-tool/dm320103
- Upgrade from "MHC-based MPLAB harmony v3" to "MCC-based project": https://microchipdeveloper.com/xwiki/bin/view/software-tools/harmony/update-and-configure-existing-mhc-proj-to-mcc-proj/
- Getting started with PIC32MX470: https://microchipdeveloper.com/xwiki/bin/view/software-tools/harmony/pic32mx470-getting-started-training-module/
- How to create a new project in MPLAB: https://microchipdeveloper.com/xwiki/bin/view/software-tools/harmony/getting-started-training-module-using-mcc/
- github repo of Microchip's USB code examples: https://github.com/Microchip-MPLAB-Harmony/usb_apps_device/
- Documentation on the USB code examples: https://mplab-discover.microchip.com/v2/category/com.microchip.code.examples/com.microchip.ide.project?dsl=PIC32MX470F512H


## PIC32 Code

#### PIC32MX: Microchip's USB Demo Projects

There are USB HID demos, try to verify the setup by building them and playing them on the PIC32MX470, if e.g. `hid_basic_demo` works out, the setup is ok.  

For the Microchip demos, build the USB Device Library Application using the _M_PLAB _H_armony _C_onfigurator (MHC). Either implement yourself, or use the provided demos as a starting point, e.g.
`./harmony/harmony3/usb_apps_device/apps/hid_basic/firmware/pic32mx470_curiosity.X/nbproject/project.xml` (watch out for a "project.xml")  

## USB HID Demo

Description of how to setup the USB HID Demo as a new project. Type `CTRL + SHIFT + n` (or `File -> New Project`) to start a new project.  

TODO          


#### Source Organization

- Find the implementation in `app.c`, `app.h` and `main.c`
- The platform here is `default`, it is not supposed to be modified
- Find available LED instructions in `./firmware/src/config/default/bsp/bsp.h`
- USB device driver is prepared also in config/default

This Implementation registers for USB read requests directly, instead of setting up yet another state machine, then going into a USB-wait-for-data-state  


#### One word about MHC / Clocking plugin

After the MCC setup was setup, or anyhow adjusted, you need to `generate` the setup. Only this will generate the code. In case a meld-based diff will pop up to accept the design modifications to the project `src`. Example, the clock settings are by default turned off, no divider are seetup and USB won't clocked. This can be adjusted in the `plugin -> Clock Settings`. This is _not_ sufficient. Do the `generate` and accept the settings e.g. into the `./usb/initialization.c` file.  

ref: https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/UserGuides/MPLABHarmonyConfiguratorUsersGuide_v111.pdf  


## Flashing the Demo to the Target

TODO        
