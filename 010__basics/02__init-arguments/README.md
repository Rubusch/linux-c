# Hello Module Init Arguments

Demonstrates passing of command line arguments. This sets only the first element of the array.  

A "static" declaration for variables and functions necessary to avoid namespace conflicts with other functions by the same name (in same "common" namespace!).  

A  linux kernel module, first tested: kernel 2.6.18.  

## Usage

```
$ make
$ sudo insmod hello.ko mystring="foobar" myshort=255 myintArray=1
$ sudo rmmod hello
```

Logs  
```
...
[16:34:40.400] Feb  4 15:18:56 ctrl001 kernel: [  123.868094] Hello World
[16:34:40.400] Feb  4 15:18:56 ctrl001 kernel: [  123.868115] myshort is a short integer: 255
[16:34:40.401] Feb  4 15:18:56 ctrl001 kernel: [  123.868126] myint is a integer: 420
[16:34:40.402] Feb  4 15:18:56 ctrl001 kernel: [  123.868136] mylong is a long integer: 9999
[16:34:40.402] Feb  4 15:18:56 ctrl001 kernel: [  123.868145] mystring is a string: foobar
[16:34:40.403] Feb  4 15:18:56 ctrl001 kernel: [  123.868155] myintArray[0] = 1
[16:34:40.425] Feb  4 15:18:56 ctrl001 kernel: [  123.868164] myintArray[1] = -1
[16:34:40.426] Feb  4 15:18:56 ctrl001 kernel: [  123.868181] got 1 arguments for myintArray.
...
```

## References:

* Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
