# file access

Shows how to open, read and write a file in the filesystem from off the kernel.  

## Usage

```
# insmod ./file.ko 
# rmmod file
# cat /tmp/foo 
   ....ooooOOOO0000*
```
Logs  
```
[23:21:30.750] [12493.528020] mod_init(): called
[23:21:30.804] [12493.532723] mod_init(): wrote 128 bytes to file
[23:21:30.805] [12493.538824] mod_init(): read 128 bytes: '....ooooOOOO0000*
[23:21:30.805] [12493.538824] '
[23:21:30.805] Feb 10 22:21:30 ctrl001 kernel: [12493.528020] mod_init(): called
[23:21:30.806] Feb 10 22:21:30 ctrl001 kernel: [12493.532723] mod_init(): wrote 128 bytes to file
[23:21:30.807] Feb 10 22:21:30 ctrl001 kernel: [12493.538824] mod_init(): read 128 bytes: '....ooooOOOO0000*
[23:21:30.808] Feb 10 22:21:30 ctrl001 kernel: [12493.538824] '
[23:21:37.798] [12500.574581] mod_exit(): called
[23:21:37.819] Feb 10 22:21:37 ctrl001 kernel: [12500.574581] mod_exit(): called
```

## References
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial
