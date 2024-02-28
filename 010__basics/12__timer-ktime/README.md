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
    [ 1050.127610] hello_init(): called
    [ 1050.127654] say(): [1/3] hello again
    [ 1050.127682] say(): [2/3] hello again
    [ 1050.127702] say(): [3/3] hello again
    [ 1053.722465] hello_exit(): called
    [ 1053.722505] hello_exit(): unloading module after 3 secs
    [ 1053.722530] say(): [1/3] hello again
    [ 1053.722551] say(): [2/3] hello again
    [ 1053.722571] say(): [3/3] hello again
```

## References

- Linux Driver Development for Embedded Processors, A. L. Rios, 2018

