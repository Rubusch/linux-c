# Hello Module


## Usage

A basic linux kernel module, 2.6.18. It demonstrates modinfo support, author, tainting (GPL License), description showed by:  

```
# modinfo hello
```

## Notes

A "static" declaration for variables and functions is necessary to avoid namespace conflicts with other functions by the same name (in same "common" namespace!).  

---

## References:
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
