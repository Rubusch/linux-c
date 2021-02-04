# Hello Module Init Arguments

A  linux kernel module, first tested: kernel 2.6.18.  


## Usage

```
$ make
$ sudo insmod hello.ko mystring="foobar" mybyte=255 myintArray=1

$ sudo rmmod hello
```

## Notes

Demonstrates passing of command line arguments. This sets only the first element of the array.  

A "static" declaration for variables and functions necessary to avoid namespace conflicts with other functions by the same name (in same "common" namespace!).  


---

## References:

* Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
