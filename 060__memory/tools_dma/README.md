# Instrumentation and Debug Code DMA Demo

TODO	

# Build

## Devicetree

Copy it to the specified location in the linux sources, then build it  
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
TODO          
```

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

debugging  
* https://wiki.st.com/stm32mpu/wiki/Dmaengine_overview
