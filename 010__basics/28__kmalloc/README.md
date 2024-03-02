# kmalloc / kzalloc

The demo shows basic allocations with `kmalloc()` and
`kzalloc()`. This is memory basic usage, memory allocation API gets
more complicated where origin of the allocation comes into play. On
older 32-bit systems this means allocating linear memory or virtual
memory which then still needs to be mounted in, or e.g. turning on the
implicit enhancement of slabs and a slab allocator. For the basics
this demo (references) is a good starting point.  

## Usage

```
# insmod ./kmalloc.ko
# rmmod ./kmalloc.ko

# dmesg
    [22:39:16.983] Feb  8 21:39:17 ctrl001 kernel: [29588.409285] mod_exit(): ptr2->version 123, ptr2->data 'Humpty Dumpty sat on a wall.' (global data)
    [22:39:17.027] Feb  8 21:39:17 ctrl001 kernel: [29733.521303] mod_init(): called
    [22:39:17.027] Feb  8 21:39:17 ctrl001 kernel: [29733.521357] mod_init(): *ptr1: 0x00000000 - kmalloc()
    [22:39:17.027] Feb  8 21:39:17 ctrl001 kernel: [29733.521391] mod_init(): *ptr1: 0xc00abcde
    [22:39:17.027] Feb  8 21:39:17 ctrl001 kernel: [29733.521413] mod_init(): *ptr1: 0x00000000 - kzalloc()
    [22:39:17.027] Feb  8 21:39:17 ctrl001 kernel: [29733.521433] mod_init(): *ptr1: 0xc0012345
    [22:39:20.000] Feb  8 21:39:20 ctrl001 kernel: [29733.521454] mod_init(): ptr2->val 123, ptr2->data 'Humpty Dumpty sat on a wall.' (global data)
    [22:39:20.001] Feb  8 21:39:20 ctrl001 kernel: [29736.510556] mod_exit(): called
```

## References
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
