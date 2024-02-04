# procfs demo with `seq_operations`

Demonstrates writing to the procfs. The implementation does a proc
file entry, and implements a read handler with sequence operations.

## Usage

```
$ sudo insmod ./helloprocfs.ko
```

Open a terminal
```
$ cat /proc/lothars_procfs_entry
```

See on the first terminal the following log entry
```
$ dmesg | tail
    (...)
    Jan 21 23:01:52 debian kernel: read handler
```

Now check out the write handler
```
$ echo "foo" | sudo tee -a /proc/lothars_procfs_entry
    foo
    tee: /proc/lothars_procfs_entry: Operation not permitted

$ dmesg | tail
    (...)
    Jan 21 23:27:51 debian kernel: write handler
```
