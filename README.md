<!---
[![aarch64-ci](https://github.com/Rubusch/c_linux/actions/workflows/aarch64-ci.yml/badge.svg)](https://github.com/Rubusch/c_linux/actions/workflows/aarch64-ci.yml)
[![armhf-ci](https://github.com/Rubusch/c_linux/actions/workflows/armhf-ci.yml/badge.svg)](https://github.com/Rubusch/c_linux/actions/workflows/armhf-ci.yml)
[![CircleCI](https://dl.circleci.com/status-badge/img/gh/Rubusch/c_linux/tree/master.svg?style=shield)](https://dl.circleci.com/status-badge/redirect/gh/Rubusch/c_linux/tree/master)
-->
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)


# Lothar's Kernel Codes

![Hardware](./backstage/pics/screenshot01.jpg)  

No guarantee! No Warranty! No return! Use the codes at your own risk.  

Find here demos, code snippets, collected, implemented, assembled, fixed, modified, adjusted, extended examples from literature, reimplemented by the idea, or simply
kernel sources. I tried to provide documentation and references to the best of my knowledge, check out the provided READMEs. Some demos require additional hardware.  

All demos were verified on my **RPi 3b** 64-bit, with a linux 6.3.x. In some cases x86 (64-bit) with same kernel. For the usb demos I used a PIC32 board as
communication counterpart.  

The sources are built with `make`. In case provide `KERNELDIR`, `ARCH`, `CROSS_COMPILE` in the environment. `KERNELDIR` defaults to `/usr/src/linux`.  
