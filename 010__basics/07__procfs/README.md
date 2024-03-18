# procfs with fileoperations (fops)

Demonstrates writing to the procfs. The implementation does a proc
file entry, and implements a read handler.

## Usage

```
$ sudo insmod ./hello.ko
$ cat /proc/lothars_dir/lothars_file
    Hello ProcFS!
$ rmmod hello
```

Logs  
```
$ tail -f /var/log/messages
    ...
	[   24.077552] mod_init()
	[   24.133629] open_procfs()
	[   24.133769] read_procfs()
	[   24.133785] READ
	[   24.133846] read_procfs()
	[   24.356034] mod_exit()
    ...
```
