# Hello IOCTL

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod helloioctl.ko

$ sudo ./ioctl.elf
    APP: device name: '/dev/lothars_chardev_device'
    APP: writing to ioctl (register application) 44
    APP: waiting for signal...
```

In a separate shell, perform a read on the device.  

```
$ sudo cat /dev/lothars_chardev_device
```

The first shell shows the following.  

```
    APP: sig_event_handler() - received signal from kernel: value = 1
    APP: sig_event_handler() - READY.

$ sudo rmmod helloioctl


$ dmesg | tail
    Feb  1 22:36:24 debian kernel: CHARDEV: init_hello_ioctl() - major = 244, minor = 123
    Feb  1 22:36:24 debian kernel: CHARDEV: init_hello_ioctl() device driver init - OK

    Feb  1 22:36:32 debian kernel: CHARDEV: device_ioctl(000000001495c6ed, 1074291041, 140728785201704)
    Feb  1 22:36:32 debian kernel: CHARDEV: device_ioctl() sig = 44

    Feb  1 22:36:37 debian kernel: CHARDEV: device_read()
    Feb  1 22:36:37 debian kernel: CHARDEV: device_read() sending SIGNAL to userspace application
    Feb  1 22:36:37 debian kernel: CHARDEV: processing signal to app...

    Feb  1 22:36:37 debian kernel: CHARDEV: device_release()
    Feb  1 22:36:37 debian kernel: CHARDEV: device_release()
    Feb  1 22:36:46 debian kernel: CHARDEV: character device unregistered
    Feb  1 22:36:46 debian kernel: CHARDEV: READY.
```


## Notes

A loadable kernelmodule and a corresponding userspace application to show the usage of ioctl mechanisms.  

The ioctl implementation needs the following steps:  
 * Create IOCTL command in driver
 * Write IOCTL function in the driver
 * Create IOCTL command in a Userspace application
 * Use the IOCTL system call in a Userspace


#### Signal sending to Userspace Application

A userspace application opens a file descriptor on the chardev IOCTL, and passes the signal number for a signal which itself could handle. On a READ event on the chardev /dev node the singal is sent from the kernel module (chardev driver) to the userspace application. The userspace application handles the signal via signal handler and sigaction. The action is ending the program. Unloading the kernelspace module unregisters the chardev.  

A better example would be as on www.embedtronicx.com the interrupt handler in kernelspace to send the signal (event) to the userspace application. Anyway simulating hardware interrupts seemed more tedious, than (mis-)using a read event on a chardev.  


---

## References:
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
