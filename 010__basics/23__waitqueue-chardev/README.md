# Chardev with Event Triggered Queue

Demonstrates a character device (miscdevice) and a wait on several
events using waitqueue. Here the read is triggered as an event to the
driver.  

## Usage

```
# insmod ./waitqueue.ko

# cat /dev/lothars_hello_class

# rmmod waitqueue

# dmesg | tail
    Jan 27 23:56:24 debian kernel: init_hello_chardev() initializing
    Jan 27 23:56:24 debian kernel: init_hello_chardev() major = 244, minor = 123
    Jan 27 23:56:24 debian kernel: thread created
    Jan 27 23:56:24 debian kernel: init_hello_chardev() done.
    Jan 27 23:56:24 debian kernel: waiting for event...

// trigger: 'cat /dev/lothars_hello_device' event..
    Jan 27 23:56:46 debian kernel: hello_open()
    Jan 27 23:56:46 debian kernel: hello_read()
    Jan 27 23:56:46 debian kernel: hello_release()
    Jan 27 23:56:46 debian kernel: event came from READ - read count: 1
    Jan 27 23:56:46 debian kernel: waiting for event...

// trigger: exit event through 'rmmod'
    Jan 27 23:57:00 debian kernel: event came from EXIT
    Jan 27 23:57:00 debian kernel: cleanup_hello_chardev() READY.
```

## References:
 * Linux kernel source, comented API and documentation
