# gpiod

```
$ make
aarch64-linux-gnu-gcc -g -Wall -lgpiod   -c -o gpio-demo.o gpio-demo.c
aarch64-linux-gnu-gcc -g -Wall -lgpiod -o gpio-demo.elf gpio-demo.o # -lgpiod
/usr/lib/gcc-cross/aarch64-linux-gnu/9/../../../../aarch64-linux-gnu/bin/ld: cannot find -lgpiod
collect2: error: ld returned 1 exit status
make: *** [Makefile:15: gpio-demo.elf] Error 1
```

NB: For compilation the environment needs `gpiod` and `libgpiod-dev` installed
for the toolchain. When cross-compiling this should be installed into the
cross-toolchain. Alternativly download gpiod from the github page and
compile it as library. Or, compile the entire source on the target (RPI)
directly.
