# timer - the plain old mod timer

Obtain timing information.

## USAGE

```
$ sudo insmod ./hello.ko
$ sudo rmmod hello
```

Logs  
```
...
[21:32:40.900] Feb  5 22:19:11 ctrl001 kernel: [  136.304069] mod_init(): called
<waits some seconds>
[21:32:40.900] Feb  5 22:19:11 ctrl001 kernel: [  137.311578] say_cb(): lothar's timer callback
...
```

## References
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
