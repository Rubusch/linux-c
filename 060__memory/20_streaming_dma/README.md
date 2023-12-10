# Streaming DMA Demo

The streaming DMA mapping routines can be called from interrupt context. There are two versions of each map/unmap, one which will map/unmap a single memory region, and one which will map/unmap a scatterlist.  

To map a single region, you do:  
```
    struct device *dev = &my_dev->dev;
    dma_addr_t dma_handle;
    void *addr = buffer->ptr;
    size_t size = buffer->len;

    dma_handle = dma_map_single(dev, addr, size, direction);
    if (dma_mapping_error(dev, dma_handle)) {
            /*
             * reduce current DMA mapping usage,
             * delay and try again later or
             * reset driver.
             */
            goto map_error_handling;
    }
```

And unmap it:  
```
    dma_unmap_single(dev, dma_handle, size, direction);
```

# Build

## Devicetree

copy it to the specified location in the linux sources (6.3), then build it  
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
root@ctrl001:/home/pi# insmod dma_demo.ko
root@ctrl001:/home/pi# echo 123 > /dev/sdma_test
root@ctrl001:/home/pi# rmmod dma_demo
```

...watch the log `/var/log/messages`  
```
Dec 10 21:06:54 ctrl001 kernel: [ 1108.784508] sdma_m2m soc:sdma_m2m: lothars_probe() - called
Dec 10 21:06:54 ctrl001 kernel: [ 1108.784554] sdma_m2m soc:sdma_m2m: lothars_probe() - 0. specify DMA channel caps
Dec 10 21:06:54 ctrl001 kernel: [ 1108.784567] sdma_m2m soc:sdma_m2m: lothars_probe() - 1. request DMA channel
Dec 10 21:06:57 ctrl001 kernel: [ 1112.164883] sdma_write() - called
Dec 10 21:06:57 ctrl001 kernel: [ 1112.164930] sdma_m2m soc:sdma_m2m: sdma_write() - the wbuf string is '123
Dec 10 21:06:57 ctrl001 kernel: [ 1112.164930] ' (initially)
Dec 10 21:06:57 ctrl001 kernel: [ 1112.164963] sdma_m2m soc:sdma_m2m: sdma_write() - the rbuf string is '' (initially)
Dec 10 21:06:57 ctrl001 kernel: [ 1112.164988] sdma_m2m soc:sdma_m2m: sdma_write() - 2. DMA mapping
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165014] sdma_m2m soc:sdma_m2m: sdma_write() - dma_src map optained: 0xC671C080
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165043] sdma_m2m soc:sdma_m2m: sdma_write() - dma_dst map obtained: 0xC671E080
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165069] sdma_m2m soc:sdma_m2m: sdma_write() - 3. DMA transaction memcpy()
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165163] sdma_m2m soc:sdma_m2m: sdma_write() - 4. setup a DMA callback
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165189] sdma_m2m soc:sdma_m2m: sdma_write() - 5. submit the DMA transaction
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165252] sdma_m2m soc:sdma_m2m: sdma_write() - 6. start DMA transaction
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165497] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - called
Dec 10 21:06:57 ctrl001 kernel: [ 1112.165564] sdma_m2m soc:sdma_m2m: sdma_write() - dma transaction has completed: DMA_COMPLETE
Dec 10 21:06:58 ctrl001 kernel: [ 1113.165874] sdma_m2m soc:sdma_m2m: sdma_write() - wbuf = '123
Dec 10 21:06:58 ctrl001 kernel: [ 1113.165874] '
Dec 10 21:06:58 ctrl001 kernel: [ 1113.165905] sdma_m2m soc:sdma_m2m: sdma_write() - rbuf = '123
Dec 10 21:06:58 ctrl001 kernel: [ 1113.165905] '
Dec 10 21:06:58 ctrl001 kernel: [ 1113.165918] sdma_m2m soc:sdma_m2m: sdma_write() - 7. unmap DMA chunks
Dec 10 21:07:01 ctrl001 kernel: [ 1115.941237] sdma_m2m soc:sdma_m2m: lothars_remove() - called
```

## References
* https://www.kernel.org/doc/html/v6.3/core-api/dma-api-howto.html
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 382
