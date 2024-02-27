# Character Device Driver

 1. Define the specific functions and define a file_operations structure and set the funcions to be used for which operation  

 2. Define and implement each operation for reading from user space to kernel space use put_user  

NB: `init_module(void)` and `exit_module(void)` implementations must not have `static` return types!  

C90 conformity: declarations of variables have to be made at begin of each block (a function body is a block!)  

The loadable kernel module, tested on 5.4.75.  

## Usage
```
$ make
$ sudo insmod hello.ko
$ dmesg
    (...)
    [198458.510364] I was assigned major number 244.
    [198458.510365] To talk to the device, create a dev file with
    [198458.510366] 'sudo mknod /dev/lothars_char_dev c 244 0'
    (...)

$ sudo mknod /dev/lothars_char_dev c 244 0

$ sudo cat /dev/lothars_char_dev
     already told you 1 times Hello World!
```

Additionally it creates an entry in `/proc/devices`  
