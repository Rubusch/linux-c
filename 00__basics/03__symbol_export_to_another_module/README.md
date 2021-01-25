
# Symbol Export between Kernel Modules

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod module_a.ko
$ sudo insmod module_b.ko

$ sudo rmmod module_b
$ sudo rmmod module_a

$ dmesg
    Jan 25 21:21:34 debian kernel: init_module_a() initializing..
    Jan 25 21:21:56 debian kernel: init_module_b() initializing..
    Jan 25 21:21:56 debian kernel: shared_func() has been called! <-----------------
    Jan 25 21:22:23 debian kernel: cleanup_module_b() READY.
    Jan 25 21:22:30 debian kernel: cleanup_module_a() READY.
    ^Z
```


## Notes

``EXPORT_SYMBOL`` exports the symbol to any loadable module.  

``EXPORT_SYMBOL_GPL`` exports the symbol only to GPL-licensed modules.  

**Limitations:**  
 * That symbol should not be static or inline.  
 * Order of loading the driver is matter. ie. We should load the module which has the definition of the symbol, then only we can load the module who is using that symbol.  
 * Variables, definitions of structs, etc. still will need the classic ``#include "foobar.h"`` i.e. in such cases to include some shared headers of the other module.  

---

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
