# Hello File Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod ./hellofile.ko

$ sudo rmmod hellofile

$ dmesg | tail
    

```


## Notes

Demonstrates a character device and device file setup.  

---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
