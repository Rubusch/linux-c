# Hello IOCTL

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod ./helloioctl.ko

$ sudo ./ioctl.elf
    XXX device_name '/dev/lothars_chardev_device'
    enter a character to send to ioctl device:
123
    writing
    reading
    value = 123
    READY.

$ sudo rmmod helloioctl

$ dmesg | tail
    [ 3586.383813] the device file name is important, because
    [ 3586.383830] the ioctl program assumes that's the
    [ 3586.383846] file you'll use.
    [ 3591.998477] device_open(0000000092fcc5aa)
    [ 3599.678060] device_ioctl(0000000092fcc5aa, 1074291041, 548814834576)
    [ 3599.678189] device_ioctl() value = 123
    [ 3599.678263] device_ioctl(0000000092fcc5aa, 2148032866, 548814834572)
    [ 3599.678545] device_release(00000000536d670b, 0000000092fcc5aa)
    [ 3616.653381] character device unregistered
    [ 3616.653419] READY.
```

## Notes

This is based on one of the Salzman Demos. Being it outdated or not, the string
handling was commented out. The demo is so to speak in progress or still not
upgraded.  

---

## References:
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18  
