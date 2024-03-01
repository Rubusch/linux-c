# Linked List usd with chardev driver

The chardev driver (miscdevice) establishes a devicenode. It allows reading
elements, which are stored in a linked list. The implementation uses
the kernel data structure for storing elements. Later the content of
the linked list can be printed. Note, the linked list will allow for
numbers only, characters won't be accepted.  

#### Notes on the kernel's linked list

A linked list is a data structure that consists of a sequence of
nodes. Each node is composed of two fields: The data field and the
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
 * Reverse traversing is difficult in the linked list.

The linux kernel has built-in Linked List which are
**Double Linked List** defined in
``/lib/modules/$(uname -r)/build/include/linux/list.h``  

## Usage

```
# insmod ./list.ko

# echo 7 > /dev/lothars_device

# cat /dev/lothars_device

# echo 456 > /dev/lothars_device

# cat /dev/lothars_device

# echo asdf > /dev/lothars_device
	bash: echo: write error: Invalid argument

# cat /dev/lothars_device
# rmmod list
```
Logs  
```
$ dmesg | tail
    [32694.039490] list: loading out-of-tree module taints kernel.
    [32694.040533] mod_init(): called
    [32707.675770] hello_linkedlist_write(): called
    [32707.675820] hello_linkedlist_write(): received 7
    [32749.879804] hello_linkedlist_read(): called
    [32749.879850] hello_linkedlist_read(): node 0 data = 7
    [32749.879875] hello_linkedlist_read(): total nodes: 1
    [32767.430232] hello_linkedlist_write(): called
    [32767.430272] hello_linkedlist_write(): received 456
    [32777.255551] hello_linkedlist_read(): called
    [32777.255597] hello_linkedlist_read(): node 0 data = 7
    [32777.255623] hello_linkedlist_read(): node 1 data = 456
    [32777.255645] hello_linkedlist_read(): total nodes: 2
    [32797.958093] hello_linkedlist_write(): called
    [32797.958135] hello_linkedlist_write(): invalid value
    [32825.747470] hello_linkedlist_read(): called
    [32825.747517] hello_linkedlist_read(): node 0 data = 7
    [32825.747543] hello_linkedlist_read(): node 1 data = 456
    [32825.747565] hello_linkedlist_read(): total nodes: 2
    [32832.490982] mod_exit(): called
```

## References:

 * https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
