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
    [22:57:11.409] Feb  5 23:43:41 ctrl001 kernel: [ 5207.852755] mod_init()
    [22:57:21.192] Feb  5 23:43:51 ctrl001 kernel: [ 5217.608495] open_procfs()
    [22:57:21.192] Feb  5 23:43:51 ctrl001 kernel: [ 5217.608699] read_procfs()
    [22:57:21.192] Feb  5 23:43:51 ctrl001 kernel: [ 5217.608725] READ
    [22:57:21.193] Feb  5 23:43:51 ctrl001 kernel: [ 5217.608858] read_procfs()
    [22:58:28.022] Feb  5 23:44:58 ctrl001 kernel: [ 5284.445341] open_procfs()
    [22:58:28.022] Feb  5 23:44:58 ctrl001 kernel: [ 5284.445537] write_procfs()
    [22:58:38.529] Feb  5 23:45:08 ctrl001 kernel: [ 5294.965630] mod_exit()
    ...
```
