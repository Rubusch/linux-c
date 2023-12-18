# Consistent DMA Demo using dma_alloc_coherent()

The `dma_alloc_coherent()` and `dma_free_coherent()` are used to allocate huge amount of memories. Thus it is mostly used for consistent mappings. In this case and for DMA here in particular, the unbufferd I/O is more efficient. This is what the `dma_alloc_coherent()` functions are doing. Implicitely the `dma_addr_t` for source and destinations are set and mapped.  

# Build

## Devicetree

copy it to the specified location in the linux sources, then build it  
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
``` Dec 10 17:41:17 ctrl001 kernel: [ 3731.248618] sdma_m2m
soc:sdma_m2m: lothars_probe() - called Dec 10 17:41:17 ctrl001 kernel:
[ 3731.248648] sdma_m2m soc:sdma_m2m: lothars_probe() - 0. allocation
and 2. mapping: dma_alloc_coherent() Dec 10 17:41:17 ctrl001 kernel: [
3731.248695] sdma_m2m soc:sdma_m2m: lothars_probe() - 0. specify DMA
channel caps Dec 10 17:41:17 ctrl001 kernel: [ 3731.248708] sdma_m2m
soc:sdma_m2m: lothars_probe() - 1. request DMA channel

Dec 10 17:41:20 ctrl001 kernel: [ 3733.962931] sdma_write() - called
Dec 10 17:41:20 ctrl001 kernel: [ 3733.962962] sdma_m2m soc:sdma_m2m: sdma_write() - the wbuf string is '123
Dec 10 17:41:20 ctrl001 kernel: [ 3733.962962] ' (initially)
Dec 10 17:41:20 ctrl001 kernel: [ 3733.962980] sdma_m2m soc:sdma_m2m: sdma_write() - the rbuf string is '' (initially)
Dec 10 17:41:20 ctrl001 kernel: [ 3733.962993] sdma_m2m soc:sdma_m2m: sdma_write() - 3. DMA transaction memcpy()
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963050] sdma_m2m soc:sdma_m2m: sdma_write() - 4. setup a DMA callback
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963064] sdma_m2m soc:sdma_m2m: sdma_write() - 5. submit the DMA transaction
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963098] sdma_m2m soc:sdma_m2m: sdma_write() - 6. start DMA transaction
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963282] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - called
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963303] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - buffer copy passed!
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963320] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - wbuf = '123
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963320] '
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963324] sdma_m2m soc:sdma_m2m: sdma_write() - dma transaction has completed: DMA_COMPLETE
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963336] sdma_m2m soc:sdma_m2m: sdma_write() - wbuf = '123
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963336] '
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963339] sdma_m2m soc:sdma_m2m: dma_m2m_callback() - rbuf = '123
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963339] '
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963352] sdma_m2m soc:sdma_m2m: sdma_write() - rbuf = '123
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963352] '
Dec 10 17:41:20 ctrl001 kernel: [ 3733.963365] sdma_m2m soc:sdma_m2m: sdma_write() - 7. unmap DMA chunks

Dec 10 17:41:26 ctrl001 kernel: [ 3740.591889] sdma_m2m soc:sdma_m2m: lothars_remove() - called
```

## References
* https://www.kernel.org/doc/html/v6.3/core-api/dma-api-howto.html
* John Linn (Xilinx), Linux DMA in Device Drivers, v3.14, https://www.youtube.com/watch?v=yJg-DkyH5CM
