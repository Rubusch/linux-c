# Hello Kernelthread Module - RCU (read copy update)

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod kthread.ko

$ sudo rmmod kthread

$ dmesg | tail
    Jan 31 19:05:55 debian kernel: init_hello_kernelthread() started
    Jan 31 19:05:55 debian kernel: thread1: counter++
    Jan 31 19:05:55 debian kernel: init_hello_kernelthread() kernelthread initialized
    Jan 31 19:05:55 debian kernel: thread2: read counter = 1
    Jan 31 19:05:56 debian kernel: thread1: counter++
    Jan 31 19:05:56 debian kernel: thread2: read counter = 2
    Jan 31 19:05:57 debian kernel: thread1: counter++
    Jan 31 19:05:57 debian kernel: thread2: read counter = 3
    Jan 31 19:05:58 debian kernel: thread1: counter++
    Jan 31 19:05:58 debian kernel: thread2: read counter = 4
    Jan 31 19:05:59 debian kernel: cleanup_hello_kernelthread() READY.
```


## Notes

The demo shows elementary usage of RCU.  

RCUs implement lock-free synchronization mechanism where alternatively a rwlocks (read write locks) can be applied.  

theory: https://www.kernel.org/doc/Documentation/RCU/whatisRCU.txt  


---

## References:
 * Linux kernel source, comented API and documentation
 * https://www.kernel.org/doc/Documentation/RCU/whatisRCU.txt
 * https://en.wikipedia.org/wiki/Read-copy-update
