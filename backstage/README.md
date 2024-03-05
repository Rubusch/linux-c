# Welcome to Builds-on-my-Machine CI

## Build

Source the cross-toolchain (where ever this is done, for me this is a
dockerized environment)  
- `ARCH`: the target architecture, e.g. `arm64`
- `CROSS_COMPILE`: prefix (if in PATH) or prefix-path to the toolchain binaries
- `KERNELDIR`: Linux kernel sources, either (linked) under `/usr/src/linux` or provided

Execute the build script, to build all possible build targets
```
$ cd ./backstage
$ ./builds-on-my-machine.sh
```

## Verification

TODO: labgrid setup

