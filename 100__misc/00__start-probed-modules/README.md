# Starter for `platform_driver`

Usually `platform_driver` should be loaded by DT binding and corresponding `probe()` according to a match in the compatibility list. For development the probe-started driver also may be started with this starter module. A reboot to reload/reinit by the DT is not needed.  

## Usage

First load the particular driver, started with a `probe()`  
```
# insmod ./rtc-dummy.ko
```

Then load the `start.ko`, provide the name of the `platform_driver` to load.  
```
# insmod ./start.ko PROBED_MODULE_NAME="lothars-rtc-dummy"
    ls: cannot access '/dev/rtc*': No such file or directory
```

NB: when unloading and reloading, the driver can be loaded, the `start.ko` is not needed to be loaded again.
