# Linked List usd with chardev driver

The chardev driver (miscdevice) establishes a devicenode. It allows reading
elements, which are stored in a linked list. The implementation uses
the kernel data structure for storing elements. Later the content of
the linked list can be printed. Note, the linked list will allow for
numbers only, characters won't be accepted.  

#### Notes on the kernel's linked list

A linked list is a data structure that consists of a sequence of
nodes. Each node is composed of two fields: the data field and the
reference field which is a pointer that points to the next node in the
sequence.  

Advantages of Linked Lists  

 * They are dynamic in nature which allocates the memory when
   required.
 * Insertion and deletion operations can be easily implemented.
 * Stacks and queues can be easily executed.
 * Linked List reduces the access time.

Disadvantages of Linked Lists  

 * The memory is wasted as pointers require extra memory for storage.
 * No element can be accessed randomly; it has to access each node sequentially.
 * Reverse Traversing is difficult in the linked list.

The linux kernel has built-in Linked List which are
**Double Linked List** defined in
``/lib/modules/$(uname -r)/build/include/linux/list.h``  


The source was tested compiled and running on 5.4.75.  


## Usage

```
$ sudo insmod ./list.ko

$ echo 7 | sudo tee -a /dev/lothars_device
    7

$ sudo cat /dev/lothars_device

$ echo 456 | sudo tee -a /dev/lothars_device
    456

$ sudo cat /dev/lothars_device

$ echo asdf | sudo tee -a /dev/lothars_device
    asdf
    tee: /dev/lothars_device: Invalid argument

$ sudo cat /dev/lothars_device
$ sudo rmmod list
```
Logs  
```
$ dmesg | tail
    Jan 30 19:21:01 debian kernel: init_hello_linkedlist() initializing...
    Jan 30 19:21:01 debian kernel: init_hello_linkedlist() major = 244, minor = 76

    Jan 30 19:21:04 debian kernel: hello_linkedlist_write()
    Jan 30 19:21:14 debian kernel: received 7

    Jan 30 19:21:14 debian kernel: hello_linkedlist_read()
    Jan 30 19:21:14 debian kernel: node 0 data = 7
    Jan 30 19:21:14 debian kernel: total nodes: 1

    Jan 30 19:21:30 debian kernel: hello_linkedlist_write()
    Jan 30 19:21:36 debian kernel: received 456

    Jan 30 19:21:36 debian kernel: hello_linkedlist_read()
    Jan 30 19:21:36 debian kernel: node 0 data = 7
    Jan 30 19:21:36 debian kernel: node 1 data = 456
    Jan 30 19:21:36 debian kernel: total nodes: 2

    Jan 30 19:21:52 debian kernel: hello_linkedlist_write()
    Jan 30 19:21:52 debian kernel: invalid value

    Jan 30 19:22:08 debian kernel: hello_linkedlist_read()
    Jan 30 19:22:08 debian kernel: node 0 data = 7
    Jan 30 19:22:08 debian kernel: node 1 data = 456
    Jan 30 19:22:08 debian kernel: total nodes: 2

    Jan 30 19:22:15 debian kernel: cleanup_hello_linkedlist() READY.
```

## References:

 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
