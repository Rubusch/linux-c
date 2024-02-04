# Kbuild with multiple files

Demonstrates a kbuild multiple files setup for a kernel module. Load
it as usual with:

```
$ sudo insmod ./hellomultiple.ko
```

`main.c` - The entry point, definition of `__init` / `__exit` functions,
and setup of `MODULE_*` macros. Additionally the driver header(s) can be
included.

`Makefile`: Set the 'all' target, and basic build environment
variables, such as KERNELDIR, define a target for building the
"module" and a target for cleaning up, "clean".

`Kbuild`: Define flags, e.g. `-O3`, defines, etc. in the Kbuild
file. Specify which `.o` files, i.e. based on .c files with the same
name, shall be build. Declare the includes by setting paths to
`Kbuild.include` files, starting the path from `$(src)`. Finally define
the main object as `obj-...` target.

`Kbuild.include`: Under the path where there are header files place
`Kbuild.include` files, specifying `-include` targets.

A `static` declaration for variables and functions necessary to avoid
namespace conflicts with other functions by the same name (in same
"common" namespace!).

## Usage

```
$ make
$ sudo insmod ./hellomultiple.ko
$ sudo rmmod hellomultiple
```
