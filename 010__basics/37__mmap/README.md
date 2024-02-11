# mmap demo

Playing around with page frame number and mapping function on a chardevice.

## Usage

NB: offset has to be provided in decimal, not in hex.  

```
# insmod ./mmap_dev.ko
# ./mmap_app.elf
    usage: ./mmap_app.elf [m,r,w,p] {data}

# ./mmap_app.elf m 123
    mmap_app.c: got '123'

# ./mmap_app.elf r
    mmap_app.c: READ: I got 4096 bytes with '123'

# ./mmap_app.elf w <------- missing an argument
    usage: ./mmap_app.elf w [data]

# ./mmap_app.elf w 456
    mmap_app.c: wrote 4096 bytes

# ./mmap_app.elf r
    mmap_app.c: READ: I got 4096 bytes with '456'

# ./mmap_app.elf p
    usage: ./mmap_app.elf p [offset]

# ./mmap_app.elf p 1
    mmap_app.c: offset '1'
    mmap_app.c: byte on offset 1 (dec): *(ptr + 0x01) = '0x32'

# ./mmap_app.elf p 2
    mmap_app.c: offset '2'
    mmap_app.c: byte on offset 2 (dec): *(ptr + 0x02) = '0x33'

# ./mmap_app.elf p 3
    mmap_app.c: offset '3'
    mmap_app.c: byte on offset 3 (dec): *(ptr + 0x03) = '0x00'

# rmmod mmap_dev.ko
```
Logs
```
[13:18:36.897] Feb 11 01:22:28 ctrl001 kernel: [11136.268931] mod_init(): called
[13:18:36.898] Feb 11 01:22:28 ctrl001 kernel: [11136.268997] mod_init(): allocated a page (4096 bytes)
[13:18:36.898] Feb 11 01:22:28 ctrl001 kernel: [11136.269023] mod_init(): PAGE_SHIFT: 12

[13:21:01.524] Feb 11 01:24:53 ctrl001 kernel: [11280.918473] chardev_mmap(): called
[13:21:10.912] Feb 11 01:25:02 ctrl001 kernel: [11290.306087] chardev_read(): called
[13:21:28.079] Feb 11 01:25:20 ctrl001 kernel: [11307.490506] chardev_write(): called
[13:21:34.816] Feb 11 01:25:26 ctrl001 kernel: [11314.211191] chardev_read(): called
[13:22:01.496] Feb 11 01:25:53 ctrl001 kernel: [11340.887729] chardev_mmap(): called
[13:22:14.568] Feb 11 01:26:06 ctrl001 kernel: [11353.960277] chardev_mmap(): called
[13:22:16.508] Feb 11 01:26:08 ctrl001 kernel: [11355.900961] chardev_mmap(): called
[13:22:24.487] Feb 11 01:26:16 ctrl001 kernel: [11363.880056] chardev_mmap(): called
[13:22:37.963] Feb 11 01:26:29 ctrl001 kernel: [11377.357512] mod_exit(): called

```

## References
- the idea is taking mainly from https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
