# Hello Procfs


Demonstrates writing to the procfs. The implementation does a proc file entry, and implements a read handler.  

## Usage

Open a terminal:  

```
$ cat /proc/lothars_procfs_entry
    <waits...>
```

See on the first terminal the following log entry:  

```
$ dmesg
    (...)
    Jan 21 23:01:52 debian kernel: read handler
```

Now check out the write handler:
```
$ echo "foo" | sudo tee -a /proc/lothars_procfs_entry
    foo
    tee: /proc/lothars_procfs_entry: Operation not permitted

$ dmesg
    (...)
    Jan 21 23:27:51 debian kernel: write handler
```

## Notes

Several upgrades, since linux 3.10 the ``create_proc_entry()`` was changed to the new function, then the mmap from userspace became obsolete, nowadays a parameter ``char __user*`` performs the mapping through ``copy_from/to_user()``.  


---

## References:

* Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18

