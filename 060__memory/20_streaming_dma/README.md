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

The device can be written but not read out  
```
root@ctrl001:/home/pi# insmod dma_demo.ko

root@ctrl001:/home/pi# echo abc > /dev/sdma_test
root@ctrl001:/home/pi# echo def > /dev/sdma_test

root@ctrl001:/home/pi# cat /dev/sdma_test
    cat: /dev/sdma_test: Invalid argument

root@ctrl001:/home/pi# rmmod dma_demo
```

In parallel watch the log `/var/log/messages`   
```
Nov 21 21:27:59 ctrl001 kernel: [  178.669384] dma_demo: loading out-of-tree module taints kernel.
Nov 21 21:27:59 ctrl001 kernel: [  178.671648] sdma_m2m soc:sdma_m2m: lothars_probe() - called
Nov 21 21:27:59 ctrl001 kernel: [  178.672707] sdma_m2m soc:sdma_m2m: lothars_probe() - done
...
Nov 21 21:28:39 ctrl001 kernel: [  218.612599] sdma_m2m soc:sdma_m2m: sdma_write() - the wbuf string is 'abc
Nov 21 21:28:39 ctrl001 kernel: [  218.612599] '
Nov 21 21:28:39 ctrl001 kernel: [  218.612652] sdma_m2m soc:sdma_m2m: sdma_write() - dma_src map optained
Nov 21 21:28:39 ctrl001 kernel: [  218.612678] sdma_m2m soc:sdma_m2m: sdma_write() - dma_dst map obtained
Nov 21 21:28:39 ctrl001 kernel: [  218.612791] sdma_m2m soc:sdma_m2m: sdma_write() - successful descriptor obtained
Nov 21 21:28:39 ctrl001 kernel: [  218.613013] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - called
Nov 21 21:28:39 ctrl001 kernel: [  218.613078] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - buffer copy passed!
Nov 21 21:28:39 ctrl001 kernel: [  218.613114] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - wbuf = 'abc
Nov 21 21:28:39 ctrl001 kernel: [  218.613114] '
Nov 21 21:28:39 ctrl001 kernel: [  218.613118] sdma_m2m soc:sdma_m2m: sdma_write() - the rbuf string is 'abc
Nov 21 21:28:39 ctrl001 kernel: [  218.613118] '
Nov 21 21:28:39 ctrl001 kernel: [  218.613148] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - rbuf = 'abc
Nov 21 21:28:39 ctrl001 kernel: [  218.613148] '
...
Nov 21 21:28:57 ctrl001 kernel: [  236.570829] sdma_m2m soc:sdma_m2m: sdma_write() - the wbuf string is 'def
Nov 21 21:28:57 ctrl001 kernel: [  236.570829] '
Nov 21 21:28:57 ctrl001 kernel: [  236.570882] sdma_m2m soc:sdma_m2m: sdma_write() - dma_src map optained
Nov 21 21:28:57 ctrl001 kernel: [  236.570907] sdma_m2m soc:sdma_m2m: sdma_write() - dma_dst map obtained
Nov 21 21:28:57 ctrl001 kernel: [  236.570946] sdma_m2m soc:sdma_m2m: sdma_write() - successful descriptor obtained
Nov 21 21:28:57 ctrl001 kernel: [  236.571167] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - called
Nov 21 21:28:57 ctrl001 kernel: [  236.571230] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - buffer copy passed!
Nov 21 21:28:57 ctrl001 kernel: [  236.571263] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - wbuf = 'def
Nov 21 21:28:57 ctrl001 kernel: [  236.571263] '
Nov 21 21:28:57 ctrl001 kernel: [  236.571295] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - rbuf = 'def
Nov 21 21:28:57 ctrl001 kernel: [  236.571295] '
Nov 21 21:28:57 ctrl001 kernel: [  236.571859] sdma_m2m soc:sdma_m2m: sdma_write() - the rbuf string is 'def
Nov 21 21:28:57 ctrl001 kernel: [  236.571859] '
...
Nov 21 21:29:21 ctrl001 kernel: [  260.470475] sdma_m2m soc:sdma_m2m: lothars_remove() - platform_remove_enter
Nov 21 21:29:21 ctrl001 kernel: [  260.471160] sdma_m2m soc:sdma_m2m: lothars_remove() - done
```

## References
* https://www.kernel.org/doc/html/v6.3/core-api/dma-api-howto.html
* Linux Driver Development for Embedded Procesesors, A. L. Rios, 2018, p. 382  
