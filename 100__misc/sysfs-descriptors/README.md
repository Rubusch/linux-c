## sysfs demo with several multiplexing solutions

The sysfs demo shows several appraoches for multiplexing on the sysfs
descriptors.  

## kernel module

Having e.g. `crossbuild-essentials-arm64` installed, `ARCH`, and
`CROSS_COMPILE` set, execute the folloing. Copy the artifacts over to
the target, e.g. if pi is on 10.1.10.204  

```
$ make
$ scp ./*.elf pi@10.1.10.204:~/
$ scp ./*.ko pi@10.1.10.204:~/
```

## Usage
```
# insmod ./sysfs-demo.ko


# ls -l /sys/lothars-sysfs/
    total 0
    -rw-r--r-- 1 root root 4096 Feb 20 19:47 notify
    -rw-r--r-- 1 root root 4096 Feb 20 19:47 trigger

# ./sysfs-select.elf
	<waiting>
```

In another shell  
```
sh2# echo foo > /sys/lothars-sysfs/trigger
```

Now, the following will be received  
```
	<waiting>
    sysfs-select-app.c: change detected in /sys/.../trigger
```
Same for notify, here the demo with `poll()`...  
```
# ./sysfs-poll.elf
    <waiting>

sh2# echo foo > /sys/lothars-sysfs/notify

	...
	sysfs-poll-app.c: triggered
	sysfs-poll-app.c: revents[0]: 0000000A
	sysfs-poll-app.c: revents[1]: 00000000
```

...or with triggered function `trigger`  
```
# ./sysfs-poll.elf
	<waiting>

	...

	sysfs-poll-app.c: triggered
	sysfs-poll-app.c: revents[0]: 00000000
	sysfs-poll-app.c: revents[1]: 0000000A
```

Logs  
```
Feb 20 19:46:05 ctrl001 kernel: [  108.412540] mod_init(): called

Feb 20 19:48:13 ctrl001 kernel: [  235.956255] sysfs_show(): called - (notify)
Feb 20 19:48:13 ctrl001 kernel: [  235.956323] sysfs_show(): called - (trigger)

...or in dmesg
[  322.924523] sysfs_store(): called
[  322.924564] sysfs_store(): sysfs_notify store trigger = 0
    
```

## References

 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
 * https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
 * Linux Device Driver Programming, J. Madieu, 2022
