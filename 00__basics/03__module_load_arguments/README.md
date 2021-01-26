# Hello Module Load Arguments

The source was tested compiled and running on 5.4.75.  


## Usage

```
$ make

$ sudo insmod ./helloarguments.ko hello_int_arg=76 hello_int_array=1,2,3 hello_string_arg="Hello"

$ sudo rmmod helloarguments

$ dmesg | tail
    Jan 26 17:52:32 debian kernel: init_hello_arguments() initializing...
    Jan 26 17:52:32 debian kernel: init_hello_arguments() hello_int_arg = 76
    Jan 26 17:52:32 debian kernel: init_hello_arguments() hello_int_arg_cb = 0
    Jan 26 17:52:32 debian kernel: init_hello_arguments() hello_int_array[0] = 1
    Jan 26 17:52:32 debian kernel: init_hello_arguments() hello_int_array[1] = 2
    Jan 26 17:52:32 debian kernel: init_hello_arguments() hello_int_array[2] = 3
    Jan 26 17:52:32 debian kernel: init_hello_arguments() hello_string_arg = 'Hello'
    Jan 26 17:52:37 debian kernel: cleanup_hello_arguments() READY.

```

NB: The string argument won't take spaces!  
NB: In case of a wrong argument, dmesg would show something like below. Just adjust the value fixes the issue.  

```
    helloarguments: unknown parameter 'xyz' ignored
```


Setting values via `sysfs` is possible, too. When triggering the "callback" value, the callback function will get triggered.    

```
$ echo '6' | sudo tee -a /sys/module/helloarguments/parameters/hello_int_arg_cb
    6

$ echo '4' | sudo tee -a /sys/module/helloarguments/parameters/hello_int_arg
    4


$ dmesg | tail
    Jan 26 18:13:53 debian kernel: new value: 6


$ sudo cat /sys/module/helloarguments/parameters/hello_int_arg
    4

$ sudo cat /sys/module/helloarguments/parameters/hello_int_arg_cb
    6
```


## Notes

There are several types of permissions:  

 * S_IWUSR
 * S_IRUSR
 * S_IXUSR
 * S_IRGRP
 * S_IWGRP
 * S_IXGRP

In this S_I is a common header.  
R = read ,W =write ,X= Execute.  
USR =user ,GRP =Group  
Using OR ‘|’ (or operation) we can set multiple permissions at a time.  


---

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
