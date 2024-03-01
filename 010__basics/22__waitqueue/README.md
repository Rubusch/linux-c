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
[14:26:06.430] Feb  8 13:26:06 ctrl001 kernel: [  137.651041] init_hello(): both threads up and running
[14:26:08.534] Feb  8 13:26:08 ctrl001 kernel: [  142.812511] thread_routine(): waitqueue_flag is pending.. timeout

[14:26:08.535] Feb  8 13:26:08 ctrl001 kernel: [  144.914633] hello_write(): called
[14:26:11.549] Feb  8 13:26:11 ctrl001 kernel: [  144.914680] hello_write(): waitqueue_flag is 1
[14:26:16.669] Feb  8 13:26:16 ctrl001 kernel: [  147.932563] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:21.817] Feb  8 13:26:21 ctrl001 kernel: [  153.052486] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:26.909] Feb  8 13:26:26 ctrl001 kernel: [  158.172490] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:32.046] Feb  8 13:26:32 ctrl001 kernel: [  163.292474] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:37.164] Feb  8 13:26:37 ctrl001 kernel: [  168.412500] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:39.607] Feb  8 13:26:39 ctrl001 kernel: [  173.532479] thread_routine(): waitqueue_flag is pending.. timeout

[14:26:39.633] Feb  8 13:26:39 ctrl001 kernel: [  176.009419] hello_write(): called
[14:26:42.252] Feb  8 13:26:42 ctrl001 kernel: [  176.009464] hello_write(): waitqueue_flag is 2
[14:26:47.393] Feb  8 13:26:47 ctrl001 kernel: [  178.652476] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:52.492] Feb  8 13:26:52 ctrl001 kernel: [  183.772468] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:53.477] Feb  8 13:26:53 ctrl001 kernel: [  188.892502] thread_routine(): waitqueue_flag is pending.. timeout

[14:26:53.478] Feb  8 13:26:53 ctrl001 kernel: [  189.829323] cleanup_hello(): called
[14:26:53.478] Feb  8 13:26:53 ctrl001 kernel: [  189.829435] thread_routine(): waitqueue_flag is 11
[14:26:53.479] Feb  8 13:26:53 ctrl001 kernel: [  189.839529] thread_routine(): waitqueue_flag is pending.. timeout
[14:26:53.480] Feb  8 13:26:53 ctrl001 kernel: [  189.849851] cleanup_hello(): READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
