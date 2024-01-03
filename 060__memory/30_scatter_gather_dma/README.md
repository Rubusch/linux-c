# Scatter/Gather DMA Demo

Scatterlists serve to map non-contiguous memory to contiguous slave
memory. Several `struct scatterlist` can be queued using a `struct
sg_table`.  

Scatterlists are very old kernel structures for dealing with
physically contiguous memory requirements. A `struct scatterlist` can
deal with up to page size amount of memory. For further chained
`struct scatterlist` objects, `struct sg_table` is used. This
fundamental connection to page size is a major limitation for
scatterlists' performance and dealing with contemporary
implementations. Furthermore, DMA addressing is mainly based on 32-bit
addresses due to historic capabilities of IOMMU, the used IOVAs (IO
virtual addresses). Thus 64-bit addresses might be masked out to
possible ranges. Two main reasons for this are described: originally
performance at dealing with 32-bit addresses was better, it is unsure
if all IOMMUs may deal easily with 64-bit addresses.  

- ref: https://lwn.net/Articles/234617/  
- ref: https://lwn.net/Articles/256368/  
- ref: https://lwn.net/Articles/904210/  

# Build

## Devicetree

Copy it to the specified location in the linux sources (6.3), then build it  
```
$ cd linux
$ cp -arf <SOURCES>/devicetree/arch ./

$ make dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb
```
Copy the file `bcm2710-rpi-3-b.dtb` to the target overwriting the `/boot/bcm2710-rpi-3-b.dtb`. In case make a safety backup first.  

## Module

Should crosscompile - having crossbuild-essentials-arm64 installed, ARCH, and CROSS_COMPILE set, execute  
```
$ cd ./module
$ make
```
Copy the module over to the target  

## Usage

```
root@ctrl001:/home/pi# insmod ./dma_demo.ko 
root@ctrl001:/home/pi# echo 123 > /dev/sdma_test 
root@ctrl001:/home/pi# rmmod dma_demo
```
watch the log  
```
Nov 29 22:59:30 ctrl001 kernel: [  733.838605] smda_m2m soc:sdma_m2m: lothars_probe() - called
Nov 29 22:59:30 ctrl001 kernel: [  733.839184] smda_m2m soc:sdma_m2m: lothars_probe() - done

Nov 29 22:59:33 ctrl001 kernel: [  736.631104] sdma_write() - called
Nov 29 22:59:33 ctrl001 kernel: [  736.631147] smda_m2m soc:sdma_m2m: sdma_write() - the dma_buf_src string is '123
Nov 29 22:59:33 ctrl001 kernel: [  736.631147] '
Nov 29 22:59:33 ctrl001 kernel: [  736.631177] smda_m2m soc:sdma_m2m: sdma_write() - scatterlist setup
Nov 29 22:59:33 ctrl001 kernel: [  736.631202] smda_m2m soc:sdma_m2m: sdma_write() - mapping
Nov 29 22:59:33 ctrl001 kernel: [  736.631307] smda_m2m soc:sdma_m2m: sdma_write() - init completion
Nov 29 22:59:33 ctrl001 kernel: [  736.631332] smda_m2m soc:sdma_m2m: sdma_write() - call dmaengine_submit()
Nov 29 22:59:33 ctrl001 kernel: [  736.631360] smda_m2m soc:sdma_m2m: sdma_write() - call dma_async_issue_pending()
Nov 29 22:59:33 ctrl001 kernel: [  736.631577] smda_m2m soc:sdma_m2m: dma_sg_callback() - finished SG DMA transaction
Nov 29 22:59:33 ctrl001 kernel: [  736.631694] smda_m2m soc:sdma_m2m: sdma_write() - the device can read '123
Nov 29 22:59:33 ctrl001 kernel: [  736.631694] '
Nov 29 22:59:33 ctrl001 kernel: [  736.631723] sdma_write() - buffer coherent sg copy successful

Nov 29 22:59:53 ctrl001 kernel: [  736.631741] sdma_write() - dma_buf_src is '123
Nov 29 22:59:53 ctrl001 kernel: [  736.631741] '
Nov 29 22:59:53 ctrl001 kernel: [  756.679095] smda_m2m soc:sdma_m2m: lothars_remove() - called
Nov 29 22:59:53 ctrl001 kernel: [  756.679768] smda_m2m soc:sdma_m2m: lothars_remove() - done
```

## Notes on Scatterlists

(written in times of Linux v6.3).  

Currently the `phyr API`, but also `dma_alloc_noncontiguous()` additions, are discussed as auxiliary for scatterlists when problems are encountered.  
ref: https://lwn.net/Articles/931943/  

## Assorted Notes on DMA "slave" - FAQ

The following post describes IMHO very well, one of the fundamental understanding issues of how the scatterlist concept works.  

Q: How to use locks on DMA channels?  

A: Don't! DMA is fast, blocking is slow. Any blocking approach slows DMA down here. Use the completion mechanism if needed to signal, if an operation finished.  

Q: When to call `dma_sync_{single,sg}()`?  

A: So, firstly, just map it with dma_map_{single,sg}(), and after each DMA transfer call either:  
```
dma_sync_single_for_cpu(dev, dma_handle, size, direction);
```
or:  
```
dma_sync_sg_for_cpu(dev, sglist, nents, direction);
```
Then, if you wish to let the device get at the DMA area again, finish accessing the data with the CPU, and then before actually giving the buffer to the hardware call either:  
```
dma_sync_single_for_device(dev, dma_handle, size, direction);
```
or
```
dma_sync_sg_for_device(dev, sglist, nents, direction);
```
If you don’t touch the data from the first `dma_map_*()` call till `dma_unmap_*()`, then you don’t have to call the `dma_sync_*()` routines at all.  

Q: What is the difference between DMA-Engine and DMA-Controller?  
ref: https://stackoverflow.com/questions/44195754/what-is-the-difference-between-dma-engine-and-dma-controller  
author (answer): https://stackoverflow.com/users/5345652/sharon-katz  

A:   
DMA - Direct memory access. The operation of your driver reading or writing from/to your HW memory without the CPU being involved in it (freeing it to do other stuff).  

DMA Controller - reading and writing can't be done by magic. if the CPU doesn't do it, we need another HW to do it. Many years ago (at the time of ISA/EISA) it was common to use a shared HW on the motherboard that did this operation. In recent years , each HW has its own DMA HW mechanism. But in all cases this specific HW gets the source address and the destination address and passes the data. Usually triggering an interrupt when done.  

DMA Engine - [...] refer to the SW side that handles the DMA. DMA is a little more complicated than usual I\O since all memory SRC and DST has to be physically present at all times during the DMA operation. If the DST address is swapped to disk, the HW will write to a bad address and the system will crash. This and other aspects of DMA are handled by the driver with code sections you probably refer to as the "DMA Engine"   


Q: As we know scatter list gathers the memory scattered physically across the memory, but virtually contiguous though. When communicating with DMA it provides an abstracted view of memory to DMA as of this is memory contiguous physically.  
How does scatterlist handle this? Is it a kind of linked list maintained within the scatterlist?  
For example if have 4000 bytes of data to be transferred using DMA ,as data is physically contiguous how scatter list makes it contiguous ? will scatterlist implementation allocate 4000 bytes of data using Kmalloc to make sure that it gets a physically contiguous memory? or will it create a linked list?  
ref: https://stackoverflow.com/questions/29270447/how-does-scatterlist-works-in-linux  
author (answer): https://stackoverflow.com/users/2511795/0andriy  

A: It seems you are mixing two different scatter lists, i.e. struct scatterlist in Linux kernel and scatter list that might be supported by the specific DMA controller. In both cases they are not contiguous. This is like an array in the first case and linked list in the second. When you call dmaengine_prep_slave_sg() the actual DMA controller driver converts it to the internal structure which might be the copy of the SG list, a new SG list that fits in the maximum DMA block size boundaries, the DMA HW linked list and so on. Depending on the DMA controller driver (let's consider recently added drivers/dma/hsu/hsu.c) the initial SG list is copied and supplied to the hardware in chunks by at most 4 DMA HW descriptors, after finish of each block it gets an interrupt and continues if the initial amount of items is more than 4 until everything is sent / received.  

#### Example

Let's assume we have the following in the SG list that is supplied to DMA controller on the system where the maximum size of DMA block is 1024 (this is usually set by dma_set_max_seg_size() for callers to know about it) and memory page size is 4096 (to be sure that we have contiguous data when the size is less than a page):  

|	| Virtual address |	Length |
|---|-----------------|--------|
| 1 |          0xa000 |  0x555 |
| 2 |          0x7000 |  0x111 |

This, due to DMA controller constraints, become...  

|   | Virtual address | Length |
|---|-----------------|--------|
| 1 |          0xa000 |  0x400 |
| 2 |          0xa400 |  0x155 |
| 3 |          0x7000 |  0x111 |

...either on the caller (user of DMA controller) or callee (DMA controller itself). On top of that for the blocks 2 and 3 DMA controller driver may apply additional alignment rules, such as 4-byte boundary for each burst. With that it becomes...  

|   | Virtual address | Length |
|---|-----------------|--------|
| 1 |          0xa000 |  0x400 |
| 2 |          0xa400 |  0x154 |
| 3 |          0xa450 |  0x001 |
| 4 |          0x7000 |  0x110 |
| 5 |          0x7110 |  0x001 |

...and DMA controller will service the 5 separate data blocks.  


## References
* https://www.kernel.org/doc/html/v6.3/core-api/dma-api-howto.html
* https://docs.kernel.org/core-api/dma-api.html

dmaengine  
* https://docs.kernel.org/driver-api/dmaengine/provider.html
* https://docs.kernel.org/driver-api/dmaengine/client.html
* https://docs.kernel.org/driver-api/dmaengine/dmatest.html
* https://static.lwn.net/kerneldoc/driver-api/dmaengine/client.html

miscellaneous  
* https://stackoverflow.com
* https://github.com/fandahao17/Raspberry-Pi-DMA-Tutorial
* https://docs.kernel.org/driver-api/dmaengine/dmatest.html
* https://iosoft.blog/2020/05/25/raspberry-pi-dma-programming/
