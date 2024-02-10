# completion

Shows the basic usage of a completion.

## Usage

```
root@ctrl001:/home/pi# insmod ./compl.ko
root@ctrl001:/home/pi# rmmod ./compl.ko
```
Logs  
```
[22:54:38.778] [10881.526047] mod_init(): called
[22:54:38.778] [10881.530827] mod_init(): time: 50 ms
[22:54:38.778] Feb 10 21:54:38 ctrl001 kernel: [10881.526047] mod_init(): called
[22:54:38.779] Feb 10 21:54:38 ctrl001 kernel: [10881.530827] mod_init(): time: 50 ms
[22:54:38.848] [10881.591678] timer_callback(): timer expired
[22:54:38.848] Feb 10 21:54:38 c[10881.597413] mod_init(): start the timer the second time: 500 ms
[22:54:38.849] trl001 kernel: [10881.591678] timer_callback(): timer expired
[22:54:38.850] Feb 10 21:54:38 ctrl001 kernel: [10881.597413] mod_init(): start the timer the second time: 500 ms
[22:54:38.927] [10881.707814] mod_init(): completion timed out!
[22:54:38.952] Feb 10 21:54:38 ctrl001 kernel: [10881.707814] mod_init(): completion timed out!
[22:54:39.335] [10882.111679] timer_callback(): timer expired
[22:54:39.355] Feb 10 21:54:39 ctrl001 kernel: [10882.111679] timer_callback(): timer expired
[22:54:47.923] [10890.682031] mod_exit(): called
[22:54:47.924] Feb 10 21:54:47 ctrl001 kernel: [10890.682031] mod_exit(): called
```

## References
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
