# dummy pwm driver

The demo shows a general pwm initialization, a similar example and
descriptions are taken from the references.  


## Build

Crosscompile the modules and copy them to the target device.  

## Usage
```
# insmod ./pwm-dummy.ko
# insmod ./pwm-dummy-ins.ko

# ls /sys/class/pwm/
    pwmchip0

# cat /sys/class/pwm/pwmchip0/npwm
    3

# udevadm info /sys/class/pwm/pwmchip0/
    P: /devices/platform/lothars-pwm-dummy.1/pwm/pwmchip0
    L: 0
    E: DEVPATH=/devices/platform/lothars-pwm-dummy.1/pwm/pwmchip0
    E: SUBSYSTEM=pwm

# ls -l /sys/class/pwm/pwmchip0/
    total 0
    lrwxrwxrwx 1 root gpio    0 Feb 17 22:25 device -> ../../../lothars-pwm-dummy.1
    --w--w---- 1 root gpio 4096 Feb 17 22:25 export
    -r--r--r-- 1 root gpio 4096 Feb 17 22:25 npwm
    drwxrwxr-x 2 root gpio    0 Feb 17 22:25 power
    lrwxrwxrwx 1 root gpio    0 Feb 17 22:25 subsystem -> ../../../../../class/pwm
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:25 uevent
    --w--w---- 1 root gpio 4096 Feb 17 22:25 unexport

# echo 1 > /sys/class/pwm/pwmchip0/export

# ls -l /sys/class/pwm/pwmchip0/
    total 0
    lrwxrwxrwx 1 root gpio    0 Feb 17 22:25 device -> ../../../lothars-pwm-dummy.1
    --w--w---- 1 root gpio 4096 Feb 17 22:26 export
    -r--r--r-- 1 root gpio 4096 Feb 17 22:25 npwm
    drwxrwxr-x 2 root gpio    0 Feb 17 22:25 power
    drwxrwxr-x 3 root gpio    0 Feb 17 22:26 pwm1
    lrwxrwxrwx 1 root gpio    0 Feb 17 22:25 subsystem -> ../../../../../class/pwm
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:25 uevent
    --w--w---- 1 root gpio 4096 Feb 17 22:25 unexport

# ls -l /sys/class/pwm/pwmchip0/pwm1/
    total 0
    -r--r--r-- 1 root gpio 4096 Feb 17 22:26 capture
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:26 duty_cycle
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:26 enable
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:26 period
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:26 polarity
    drwxrwxr-x 2 root gpio    0 Feb 17 22:26 power
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:26 uevent
```
Index starts with '0' for pwm devices, so '3' will be wrong here.  
```
# echo 3 > /sys/class/pwm/pwmchip0/export
    bash: echo: write error: No such device

# echo 2 > /sys/class/pwm/pwmchip0/export

# ls -l /sys/class/pwm/pwmchip0/
    total 0
    lrwxrwxrwx 1 root gpio    0 Feb 17 22:25 device -> ../../../lothars-pwm-dummy.1
    --w--w---- 1 root gpio 4096 Feb 17 22:28 export
    -r--r--r-- 1 root gpio 4096 Feb 17 22:25 npwm
    drwxrwxr-x 2 root gpio    0 Feb 17 22:25 power
    drwxrwxr-x 3 root gpio    0 Feb 17 22:26 pwm1
    drwxrwxr-x 3 root gpio    0 Feb 17 22:28 pwm2
    lrwxrwxrwx 1 root gpio    0 Feb 17 22:25 subsystem -> ../../../../../class/pwm
    -rw-rw-r-- 1 root gpio 4096 Feb 17 22:25 uevent
    --w--w---- 1 root gpio 4096 Feb 17 22:25 unexport

# echo 1 > /sys/class/pwm/pwmchip0/unexport
# echo 2 > /sys/class/pwm/pwmchip0/unexport

# rmmod pwm-dummy-ins
# rmmod pwm-dummy
```
Logs  
```
Feb 17 22:25:19 ctrl001 kernel: [ 2644.992519] pwmdrv_probe(): called
Feb 17 22:26:53 ctrl001 kernel: [ 2738.619756] pwmdrv_request(): called
Feb 17 22:28:00 ctrl001 kernel: [ 2805.555643] pwmdrv_request(): called
Feb 17 22:31:50 ctrl001 kernel: [ 3035.678478] pwmdrv_remove(): called
```

## References
* Linux Device Driver Programming, J. Madieu, 2022
