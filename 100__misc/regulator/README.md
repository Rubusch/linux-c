# Regulator demo

Shows a dummy regulator implementation.  

## Usage

```
# insmod ./regulator-dummy.ko
# insmod ./start.ko PROBED_MODULE_NAME="lothars-regulator-dummy"

# ls /sys/class/regulator/
regulator.0  regulator.1  regulator.2  regulator.3  regulator.4  regulator.5  regulator.6

# ls -l /sys/class/regulator/
    total 0
    lrwxrwxrwx 1 root root 0 Feb 18 20:42 regulator.0 -> ../../devices/platform/reg-dummy/regulator/regulator.0
    lrwxrwxrwx 1 root root 0 Feb 18 20:42 regulator.1 -> ../../devices/platform/cam_dummy_reg/regulator/regulator.1
    lrwxrwxrwx 1 root root 0 Feb 18 20:42 regulator.2 -> ../../devices/platform/fixedregulator_3v3/regulator/regulator.2
    lrwxrwxrwx 1 root root 0 Feb 18 20:42 regulator.3 -> ../../devices/platform/fixedregulator_5v0/regulator/regulator.3
    lrwxrwxrwx 1 root root 0 Feb 18 20:42 regulator.4 -> ../../devices/platform/cam1_regulator/regulator/regulator.4
    lrwxrwxrwx 1 root root 0 Feb 18 21:55 regulator.5 -> ../../devices/platform/lothars-regulator-dummy.1/regulator/regulator.5
    lrwxrwxrwx 1 root root 0 Feb 18 21:55 regulator.6 -> ../../devices/platform/lothars-regulator-dummy.1/regulator/regulator.6

# cat /sys/class/regulator/regulator.5/name
Regulator Core

# cat /sys/class/regulator/regulator.6/name
Regulator Fixed

# cat /sys/class/regulator/regulator.5/type
voltage

# cat /sys/class/regulator/regulator.6/microvolts
1300000

# cat /sys/class/regulator/regulator.5/microvolts
850000

# rmmod start
# rmmod regulator-dummy.ko
```
Logs  
```
[22:57:15.239] Feb 18 21:57:15 ctrl001 kernel: [ 3313.183137] isl6271a_get_voltage_sel(): called
[22:57:33.923] Feb 18 21:57:33 ctrl001 kernel: [ 3331.865246] pdrv_remove(): called
```

## References
* Regulator API (linux)
* Linux Device Driver Programming, J. Madieu, 2022
