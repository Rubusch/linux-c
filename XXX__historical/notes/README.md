# Personal Notes on the Linux Kernel

## References

Literature I studied and used over the time, I'd recommend (for the respective
kernel epoch).

* linux v4.19.x: Linux Driver Development for Embedded Processors, A. Rios, 2018  
  Good labs on an iMX7d board, an SAMA5D2 board and the RPI v3 (i.e. BCM2711, BCM2837 & CO and RPI v4) - updates can be found online on his repos. Modern APIs, GPIO/MMIO/UIO, regmap, IRQ, DMA/uDMA, IIO/sensors, USB, porting drivers)
* linux v4.19.x: Mastering Linux Device Driver Development, J. Madieu, 2020  
  Good explanation on modern APIs, MFD/Syscon, CCF/clocking/watchdogDD, ALSA, V4L2, PM/power, PCI, nvmem
* linux v2.6.34: Linux Kernel Development, R. Love, 2010 (3rd)  
  Fundamental concepts of states, interrupts, datastructures
* linux v2.6.10: Linux Device Drivers, J. Corbet, A. Rubini & GKH, 2005 (3rd)  
  A 4th edition was announced for so long.. in the meantime we read lwn.org
* linux v2.6.11: Understanding the Linux Kernel, D. Bovet & M. Cesati, 2005 (3rd)  
  Encyclopedic code analysis for a 2.6-er kernel, equally outdated for modern
* linux v2.4.x: Linux Netzwerkarchitektur (German), Wehrle, 2002  
  My first book on the networking stack at the time...
* elf format: Linkers and Loaders, J. Levine, 2000  
  A classic but still good and brief introduction into loading and linking, the elf format and related

