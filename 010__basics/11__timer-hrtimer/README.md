# timer

Obtain timing information taken from, e.g.

```
linux/drivers/usb/mon/mon_text.c +188

   struct timespec64 now;
   unsigned int stamp;
   ktime_get_ts64(&now);
   stamp = now.tv_sec & 0xFFF;  / * 2^32 = 4294967296. Limit to 4096s. * /
   stamp = stamp * USEC_PER_SEC + now.tv_nsec / NSEC_PER_USEC;
```

## USAGE

```
$ sudo insmod ./hello.ko
$ sudo rmmod hello
$ dmesg | tail
    [  877.623791] hello_init(): called
    [  878.623845] say_time(): hello hrtimer started 1000 msec ago
    [  879.348392] hello_exit(): called
```

## References

- Linux Driver Development for Embedded Processors, A. L. Rios, 2018

