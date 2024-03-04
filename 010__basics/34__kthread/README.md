# Kernel Thread Demo

Create a kernel thread. Put it to sleep, wake it up again, start /
stop operations..  

## Build

Build it for target device, e.g. RPI, then copy the .ko file over.  

## Usage

```
$ sudo insmod hello.ko

$ sudo rmmod hello

$ tail /var/log/kern.log
    Mar  4 20:13:41 ctrl001 kernel: [10301.891382] mod_init() started
    Mar  4 20:13:41 ctrl001 kernel: [10301.891854] mod_init() kernelthread initialized
    Mar  4 20:13:41 ctrl001 kernel: [10301.891883] kernelthread_routine() counting: 0
    Mar  4 20:13:42 ctrl001 kernel: [10302.918019] kernelthread_routine() counting: 1
    Mar  4 20:13:43 ctrl001 kernel: [10303.941935] kernelthread_routine() counting: 2
    Mar  4 20:13:44 ctrl001 kernel: [10304.965926] kernelthread_routine() counting: 3
    Mar  4 20:13:45 ctrl001 kernel: [10305.989940] kernelthread_routine() counting: 4
    Mar  4 20:13:46 ctrl001 kernel: [10307.013922] kernelthread_routine() counting: 5
    Mar  4 20:13:47 ctrl001 kernel: [10308.037994] mod_exit() READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, (2007)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
