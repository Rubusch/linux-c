# chardev driver: private data

The `miscdevice` framework is an interface exported by the Linux
kernel that allows modules to register their individual minor numbers.  

The `private_data` field usually is used to hand over underlying device drivers
or to extend a driver structure, such as here the `miscdriver` construct.  

## Module

Should compile cross - having `crossbuild-essentials-arm64` installed, `ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Userspace

Compile cross as well, set ``$CROSS_COMPILE``.   
```
rpi$ cd ./userspace
rpi$ make
```
Copy the module over to the target  

## Usage

```
# insmod ./priv_data.ko
# echo 123 > /dev/lothars_device
# cat /dev/lothars_device
# rmmod priv_data
```
Logs  
```
[20:08:13.805] Feb 10 19:08:13 ctrl001 kernel: [  896.551931] mod_init(): called
[20:08:18.482] Feb 10 19:08:18 ctrl001 kernel: [  901.227151] demo_write(): called
[20:08:18.483] Feb 10 19:08:18 ctrl001 kernel: [  901.227199] demo_write(): received from userspace '123'
[20:08:20.396] Feb 10 19:08:20 ctrl001 kernel: [  903.145314] demo_read(): called
[20:08:22.473] Feb 10 19:08:22 ctrl001 kernel: [  905.238212] mod_exit(): called
```

## References
* https://embetronicx.com/tutorials/linux/device-drivers/misc-device-driver/
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018
