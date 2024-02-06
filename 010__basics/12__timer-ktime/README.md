# timer - high resolution timer using ktime

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
$ sudo insmod ./hello.ko num=3
$ sudo rmmod hello
$ dmesg | tail
   [ 6709.087341] [6/7] lothar's hello
   [ 6709.087350] [7/7] lothar's hello
   [ 6727.144308] lothar's init
   [ 6727.144335] [1/3] lothar's hello
   [ 6727.144353] [2/3] lothar's hello
   [ 6727.144362] [3/3] lothar's hello
   [ 6730.722522] lothar's exit - unloading module after 3 secs
   [ 6730.722547] [1/3] lothar's hello
   [ 6730.722557] [2/3] lothar's hello
   [ 6730.722566] [3/3] lothar's hello
```

## References

- Linux Driver Development for Embedded Processors, A. L. Rios, 2018

