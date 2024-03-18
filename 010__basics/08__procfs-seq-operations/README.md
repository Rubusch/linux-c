# procfs demo with `seq_operations`

Demonstrates writing to the procfs. The implementation does a proc
file entry, and implements a read handler with sequence operations.

## Usage

```
$ sudo insmod ./hello.ko
```

Open a terminal
```
$ cat /proc/lothars_procfs_entry
    Hello ProcFS!
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

Logs  
```
[   24.638229] start_procfs()
[   24.638259] /proc/lothars_procfs_entry created
[   24.675862] open_procfs()
[   24.675997] read_procfs()
[   24.676013] READ
[   24.676072] read_procfs()
[   24.857405] stop_procfs()
[   24.857446] /proc/lothars_procfs_entry removed
```
