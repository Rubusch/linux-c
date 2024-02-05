# Building

## Setup

- rpi: `10.1.10.204/24`
- pc:  `10.1.10.100/24`
- serial connection on rpi pins

## Kernel

Building 64-bit rpi3b kernel (in my setup). Please, refer to the official raspberry page how to build the kernel: https://www.raspberrypi.com/documentation/computers/linux_kernel.html

```
$ export CROSS_COMPILE=aarch64-linux-gnu-
$ export ARCH=arm64
$ export KERNEL=kernel8
$ export KDEFCONFIG_NAME=bcm2711_defconfig
$ export KERNELDIR=~/workspace

$ cd $KERNELDIR
```
(opt) configure the kernel
```
$ make menuconfig
```

build the kernel, dt and modules
```
$ make -j $(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs
$ mkdir -p ../ext4
$ env PATH=$PATH make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=../ext4 modules_install
```

copy kernel, artifacts, dt and modules over to the rpi  
NB: adjust to _your_ kernel version, e.g. here I used `6.3.13-lothar09+`...
```
$ ssh root@10.1.10.203 mv /boot/kernel8.img /boot/kernel8.img.orig
$ scp arch/arm64/boot/Image root@10.1.10.203:/boot/kernel8.img
$ scp arch/arm64/boot/dts/broadcom/*.dtb root@10.1.10.203:/boot/
$ scp arch/arm64/boot/dts/overlays/*.dtb* root@10.1.10.203:/boot/overlays/
$ scp arch/arm64/boot/dts/overlays/README root@10.1.10.203:/boot/overlays/
```
in case this is a re-compilation clear at least those files (and adjust version)
```
$ rm -f ../ext4/lib/modules/6.3.13-lothar09+/source
$ rm -f ../ext4/lib/modules/6.3.13-lothar09+/build
```
then, wrap all modules and copy them to the rpi
```
$ KOBJECTS=( $(find . -type f -name \*.ko) )
$ for item in ${KOBJECTS[*]}; do
	mkdir -p ../ext4/lib/modules/6.3.13-lothar09+/build/$(dirname $item);
	cp ${item} ../ext4/lib/modules/6.3.13-lothar09+/build/$(dirname $item)/;
	done

$ unset KOBJECTS
$ scp -r ../ext4/* root@10.1.10.203:/
```

## Modules

Environment, this can be equally go into a file, then just source the file
```
$ export CROSS_COMPILE=aarch64-linux-gnu-
$ export ARCH=arm64
$ export KERNEL=kernel8
$ export KDEFCONFIG_NAME=bcm2711_defconfig
$ export KERNELDIR=~/workspace
```
compile / clean
```
$ make clean && make
$ scp ./*.ko pi@10.1.10.204:~/
```
...and sometimes
```
$ scp ./*.elf pi@10.1.10.204:~/
