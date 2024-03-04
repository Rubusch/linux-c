# Kernel Thread Demo

Create a kernel thread. Put it to sleep, wake it up again, start /
stop operations..  

## Build

Build it for target device, e.g. RPI, then copy the .ko file over.  

## Usage

```
$ sudo insmod hello.ko

$ sudo rmmod hello

$ dmesg | tail
    Jan 25 11:56:20 debian kernel: init_hello_kernelthread() started
    Jan 25 11:56:20 debian kernel: init_hello_kernelthread() kernelthread initialized
    Jan 25 11:56:20 debian kernel: kernelthread_routine() counting: 0
    Jan 25 11:56:21 debian kernel: kernelthread_routine() counting: 1
    Jan 25 11:56:22 debian kernel: kernelthread_routine() counting: 2
    Jan 25 11:56:23 debian kernel: kernelthread_routine() counting: 3
    Jan 25 11:56:24 debian kernel: kernelthread_routine() counting: 4
    Jan 25 11:56:25 debian kernel: kernelthread_routine() counting: 5
    Jan 25 11:56:26 debian kernel: kernelthread_routine() counting: 6
    Jan 25 11:56:27 debian kernel: kernelthread_routine() counting: 7
    Jan 25 11:56:35 debian kernel: cleanup_hello_kernelthread() READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, (2007)
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
