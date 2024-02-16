# Linked List Memory Allocation

Allocate a circular linked list in the kernel memory, composed of
several notes. Each node will be composed of two variables:  

1. A `buffer` pointer that points to a memory buffer allocated with `devm_kmalloc()` using a "for" loop
2. A `next` pointer that points to the next node of the linked list

NB: The demo uses a device tree declaration `linked_memory` which is
mainly needed to use the miscdevice struture, and device related API.  

# Build

## Module

Should crosscompile with `crossbuild-essentials-arm64` installed,
`ARCH`, and `CROSS_COMPILE` set, execute  
```
$ cd ./module
$ make
```
Copy the modules over to the target  

## Usage

```
# insmod ./list_alloc.ko
# insmod ./list_alloc_ins.ko

# echo abc > /dev/lothars_dev
# echo def > /dev/lothars_dev
# echo ghi > /dev/lothars_dev
# cat /dev/lothars_dev
    abc
    def
    ghi
# rmmod list_alloc
```

Observe the `/var/log/messages` in parallel  
```
Nov 21 10:57:00 ctrl001 kernel: [  304.742042] lothars_probe() - called
...
Nov 21 10:57:23 ctrl001 kernel: [  304.742747] lothars_probe() - got minor 121
Nov 21 10:57:23 ctrl001 kernel: [  328.167236] lothars_dev_open() - called
Nov 21 10:57:23 ctrl001 kernel: [  328.167597] lothars_dev_write() - called
Nov 21 10:57:23 ctrl001 kernel: [  328.167622] lothars_dev_write() - node number: 1
...
Nov 21 10:57:30 ctrl001 kernel: [  328.167713] lothars_dev_close() - called
Nov 21 10:57:30 ctrl001 kernel: [  335.227221] lothars_dev_open() - called
Nov 21 10:57:30 ctrl001 kernel: [  335.227381] lothars_dev_write() - called
Nov 21 10:57:30 ctrl001 kernel: [  335.227403] lothars_dev_write() - node number: 1
Nov 21 10:57:30 ctrl001 kernel: [  335.227436] lothars_dev_write() - called
Nov 21 10:57:30 ctrl001 kernel: [  335.227454] lothars_dev_write() - node number: 2
...
Nov 21 10:57:39 ctrl001 kernel: [  335.227541] lothars_dev_close() - called
Nov 21 10:57:39 ctrl001 kernel: [  344.002294] lothars_dev_open() - called
Nov 21 10:57:39 ctrl001 kernel: [  344.002446] lothars_dev_write() - called
Nov 21 10:57:39 ctrl001 kernel: [  344.002467] lothars_dev_write() - node number: 2
Nov 21 10:57:39 ctrl001 kernel: [  344.002501] lothars_dev_write() - called
Nov 21 10:57:39 ctrl001 kernel: [  344.002520] lothars_dev_write() - node number: 3
...
Nov 21 10:57:50 ctrl001 kernel: [  344.002617] lothars_dev_close() - called
Nov 21 10:57:50 ctrl001 kernel: [  354.824117] lothars_dev_open() - called
Nov 21 10:57:50 ctrl001 kernel: [  354.824320] lothars_dev_read() - called
Nov 21 10:57:50 ctrl001 kernel: [  354.824468] lothars_dev_read() - called
Nov 21 10:57:50 ctrl001 kernel: [  354.824556] lothars_dev_read() - called
Nov 21 10:57:50 ctrl001 kernel: [  354.824632] lothars_dev_read() - called
...
Nov 21 10:58:04 ctrl001 kernel: [  355.102919] lothars_dev_close() - called
```

## References
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 363  
