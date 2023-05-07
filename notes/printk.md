# printk()

## References
- https://www.kernel.org/doc/html/latest/core-api/printk-basics.html

## Usage

Try to prefer the alias function to the plain `printk()` call.  
```
    printk(KERN_INFO "Message: %s\n", arg);
```

Check log level settings
```
pi@raspberrypi:~$ cat /proc/sys/kernel/printk
    3    4    1    3
```
The result shows the current, default, minimum and boot-time-default log levels.  
To change the current console_loglevel simply write the desired level to /proc/sys/kernel/printk. For example, to print all messages to the console:  
```
# echo 8 > /proc/sys/kernel/printk
```
Another way, using dmesg:  
```
# dmesg -n 5
```

name         | string | alias function
------------ | ------ | ----------------
KERN_EMERG   | 0      | pr_emerg()
KERN_ALERT   | 1      | pr_alert()
KERN_CRIT    | 2      | pr_crit()
KERN_ERR     | 3      | pr_err()
KERN_WARNING | 4      | pr_warn()
KERN_NOTICE  | 5      | pr_notice()
KERN_INFO    | 6      | pr_info()
KERN_DEBUG   | 7      | pr_debug()
KERN_DEFAULT | ""     | N/A
KERN_CONT    | C      | pr_cont()
