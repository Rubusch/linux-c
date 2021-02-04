# USB Demo Module

The source was tested compiled and running on 5.4.75.  


## Usage

First, adjust the vendorid and deviceid in the source to the specific device.  

```
$ make
$ sudo insmod hello_usb.ko
```

Now plug the specified USB device and have a look into the ``/var/log/syslog``.  

```
$ sudo dmesg | tail
    Feb  3 16:00:50 debian kernel: usb 1-4.3.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
    Feb  3 16:00:50 debian kernel: usb 1-4.3.2: Product: J-Link
    Feb  3 16:00:50 debian kernel: usb 1-4.3.2: Manufacturer: SEGGER
    Feb  3 16:00:50 debian kernel: usb 1-4.3.2: SerialNumber: 000979015607
    Feb  3 16:00:50 debian kernel: cdc_acm 1-4.3.2:1.0: ttyACM0: USB ACM device
    Feb  3 16:00:50 debian kernel: cdc_acm 1-4.3.2:1.2: ttyACM1: USB ACM device
    Feb  3 16:00:50 debian kernel: pen_probe() - XXX probed usb device (1366:1051) XXX <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    Feb  3 16:00:50 debian kernel: usb-storage 1-4.3.2:1.5: USB Mass Storage device detected
    Feb  3 16:00:50 debian kernel: scsi host2: usb-storage 1-4.3.2:1.5
    Feb  3 16:00:50 debian mtp-probe: checking bus 1, device 43: "/sys/devices/pci0000:00/0000:00:14.0/usb1/1-4/1-4.3/1-4.3.2"
    Feb  3 16:00:50 debian mtp-probe: bus: 1, device: 43 was not an MTP device
    Feb  3 16:00:50 debian mtp-probe: checking bus 1, device 43: "/sys/devices/pci0000:00/0000:00:14.0/usb1/1-4/1-4.3/1-4.3.2"
    Feb  3 16:00:50 debian mtp-probe: bus: 1, device: 43 was not an MTP device
    Feb  3 16:00:51 debian kernel: scsi 2:0:0:0: Direct-Access     SEGGER   MSD Volume       1.00 PQ: 0 ANSI: 4
    Feb  3 16:00:51 debian kernel: sd 2:0:0:0: [sdc] 21829 512-byte logical blocks: (11.2 MB/10.7 MiB)
    Feb  3 16:00:51 debian kernel: sd 2:0:0:0: [sdc] Write Protect is off
    Feb  3 16:00:51 debian kernel: sd 2:0:0:0: [sdc] Mode Sense: 0b 00 00 08
    Feb  3 16:00:51 debian kernel: sd 2:0:0:0: [sdc] No Caching mode page found
    Feb  3 16:00:51 debian kernel: sd 2:0:0:0: [sdc] Assuming drive cache: write through
    Feb  3 16:00:51 debian kernel: sdc:
    Feb  3 16:00:51 debian kernel: sd 2:0:0:0: [sdc] Attached SCSI removable disk
    Feb  3 16:00:57 debian kernel: usb 1-4.3.2: USB disconnect, device number 43
    Feb  3 16:00:57 debian kernel: pen_disconnect() - XXX usb device disconnected XXX <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
```

Unload the driver as follows.  

```
$ sudo rmmod usb
```


## Notes

This is a basic instance of the usb driver skeleton. Find some more elaborated usb device driver example e.g. here:  
https://sysplay.github.io/books/LinuxDrivers/book/Content/Part11.html


---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * https://kernel.readthedocs.io/en/sphinx-samples/writing_usb_driver.html
 * https://sysplay.github.io/books/LinuxDrivers/book/Content/Part11.html
