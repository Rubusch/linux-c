# Hello Module

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make
$ sudo insmod ./hello.ko

$ sudo rmmod hello

$ dmesg | tail
    Jan 26 14:03:16 debian kernel: Hello World!
    Jan 26 14:03:23 debian kernel: Goodbye World!
```


## Notes

 * Apply a "void" in the declaration for empty function parameters necessary to avoid warnings!  

 * A "static" declaration for variables and functions necessary to avoid namespace conflicts with other functions by the same name (in same "common" namespace!).  

 * The init_module(void) and cleanup_module(void) implementations must not have "static" return types!  

 * C90 conformity: declarations of variables have to be made at begin of each block (a function body is a block!)  

---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
