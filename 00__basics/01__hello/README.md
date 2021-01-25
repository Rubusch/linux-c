# Hello Modul

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod hello.ko

$ sudo rmmod hello

$ dmesg | tail
    [128663.609279] hello: module license 'unspecified' taints kernel.
    [128663.610307] Hello World!

```


## Notes

When directly implementing the init_module() and cleanup_module() no "static" return values allowed!!!


---

## References:
Linux Kernel Module Programming Guide", Peter Jay Salzman, 2007-05-18
