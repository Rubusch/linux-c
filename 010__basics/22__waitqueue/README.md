# Waitqueue

Demonstrates a threaded demo with waitqueue. The chardev only allows
for writing, not for reading from the device node.  

## Usage

```
# insmod ./waitqueue.ko
<wait>
# echo 1 > /dev/lothars_hello_device
<wait>
# echo 2 > /dev/lothars_hello_device
<wait>
# rmmod waitqueue
```
Logs  
```
[   25.686154] mod_init() called
[   25.687960] mod_init(): both threads up and running
[   28.858102] hello_write(): called
[   28.858139] hello_write(): waitqueue_flag is 1
[   30.718520] thread_routine(): waitqueue_flag is pending.. timeout
[   32.023160] hello_write(): called
[   32.023196] hello_write(): waitqueue_flag is 2
[   35.184843] mod_exit(): called
[   35.184932] thread_routine(): waitqueue_flag is 11
[   35.194924] thread_routine(): waitqueue_flag is pending.. timeout
```

## References:
 * Linux kernel source, comented API and documentation
 * https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
