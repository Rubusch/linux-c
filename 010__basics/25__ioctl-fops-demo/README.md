# More elaborate IOCTL demo

In this demo a loadable kernelmodule and a corresponding userspace
application show the usage of ioctl mechanisms.  

The ioctl implementation needs the following steps:  
 * Create IOCTL command in driver
 * Write IOCTL function in the driver
 * Create IOCTL command in a Userspace application
 * Use the IOCTL system call in a Userspace

## Usage

```
# insmod ./helloioctl.ko

# ./ioctl.elf
    device name: '/dev/lothars_chardev_device'
    enter a number to be sent to the ioctl device:
```
The userspace application is interactive, so enter, e.g. `123`  
```
123
    writing
    reading
    value = 123
    READY.

# rmmod helloioctl
```

Logs  
```
# dmesg | tail
    [83289.938018] init_hello_ioctl() - major = 244, minor = 100
    [83289.938049] init_hello_ioctl() device driver init - OK
    [83289.938049] If you want to talk to the device driver,
    [83289.938050] you'll have to create a device file, do a:
    [83289.938050] $ sudo mknod lothars_chardev c 100 0
    [83289.938050] the device file name is important, because
    [83289.938051] the ioctl program assumes that's the
    [83289.938051] file you'll use.
    [83296.017709] device_open(00000000ff9d7624)
    [83301.396645] device_ioctl(00000000ff9d7624, 1074291041, 140736227152884)
    [83301.396646] device_ioctl() value = 123
    [83301.396649] device_ioctl(00000000ff9d7624, 2148032866, 140736227152888)
    [83301.396660] device_release(00000000849b56cf, 00000000ff9d7624)
    [83359.091206] character device unregistered
    [83359.091208] READY.
```

## References

 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
