# Hello Atomic Module

Seqlock is a reader-writer consistent locking mechanism which is
giving importance to the writer. So this avoids the problem of writer
starvation.  


#### Working of seqlock

1. When no one is in a critical section then one writer can enter into
   a critical section by acquiring its lock. Once it took its lock
   then the writer will increment the sequence number by
   one. Currently, the sequence number is an odd value. Once done with
   the writing, again it will increment the sequence number by
   one. Now the number is an even value. So, when the sequence number
   is an odd value, writing is happening. When the sequence number is
   an even value, writing has done. Only one writer thread will be
   allowed in the critical section. So other writers will be waiting
   for the lock.  

2. When the reader wants to read the data, first it will read the
   sequence number. If it is an even value, then it will go to a
   critical section and reads the data. If it is an odd value (the
   writer is writing something), the reader will wait for the writer
   to finish (the sequence number becomes an even number). The value
   of the sequence number while entering into the critical section is
   called an old sequence number.  

3. After reading the data, again it will check the sequence number. If
   it is equal to the old sequence number, then everything is
   okay. Otherwise, it will repeat step 2 again. In this case, readers
   simply retry (using a loop) until they read the same even sequence
   number before and after. The reader never blocks, but it may have
   to retry if a write is in progress.  

4. When only the reader is reading the data and no writer is in the
   critical section, any time one writer can enter into a critical
   section by taking lock without blocking. This means the writer
   cannot be blocked for the reader and the reader has to re-read the
   data when the writer is writing. This means seqlock is giving
   importance to a writer, not the reader (the reader may have to wait
   but not the writer).  

[taken from www.embetronicx.com]  


#### When are Seqlocks needed?

We cannot use this seqlock in any situations like normal spinlock or
mutex. Because this will not be effective in such situations other
than the situations mentioned below.  
 * where read operations are more frequent than write.
 * where write access is rare but must be fast.
 * That data is simple (no pointers) that needs to be
   protected. Seqlocks generally cannot be used to protect data
   structures involving pointers, because the reader may be following
   a pointer that is invalid while the writer is changing the data
   structure.

[taken from www.embetronicx.com]  


## Usage

```
$ make

$ sudo insmod ./seqlock.ko

$ sudo rmmod seqlock

$ dmesg | tail
    Feb  2 22:50:28 debian kernel: initializing..
    Feb  2 22:50:28 debian kernel: thread1 created
    Feb  2 22:50:28 debian kernel: thread2 created
    Feb  2 22:50:29 debian kernel: kthread2() - read value 1
    Feb  2 22:50:30 debian kernel: kthread2() - read value 1
    Feb  2 22:50:31 debian kernel: kthread2() - read value 2
    Feb  2 22:50:32 debian kernel: kthread2() - read value 3
    Feb  2 22:50:33 debian kernel: kthread2() - read value 5
    Feb  2 22:50:34 debian kernel: kthread2() - read value 5
    Feb  2 22:50:35 debian kernel: kthread2() - read value 7
    Feb  2 22:50:36 debian kernel: kthread2() - read value 7
    Feb  2 22:50:37 debian kernel: kthread2() - read value 8
    Feb  2 22:50:38 debian kernel: kthread2() - read value 10
    Feb  2 22:50:39 debian kernel: kthread2() - read value 11
    Feb  2 22:50:40 debian kernel: kthread2() - read value 11
    Feb  2 22:50:41 debian kernel: kthread2() - read value 13
    Feb  2 22:50:42 debian kernel: kthread2() - read value 13
    Feb  2 22:50:43 debian kernel: kthread2() - read value 14
    Feb  2 22:50:44 debian kernel: kthread2() - read value 15
    Feb  2 22:50:44 debian kernel: READY.
```

## References:
 * Linux kernel source, comented API and documentation
 * Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
 * Highly inspired by / many thanks to www.embetronicx.com (2021)
