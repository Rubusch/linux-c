[![CircleCI](https://circleci.com/gh/Rubusch/c_linux.svg?style=shield)](https://circleci.com/gh/Rubusch/c_linux)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)


# Kernel Code Snippets

No guarantee, use the code snippets at your own risk. This is just a collection code snippets. The sources are typically build by a Makefile as external kernel module.  

## Usage

Open a terminal.  

```
$ sudo tail -f /var/log/syslog
```

Open another terminal build and load the module.  

```
$ make
$ sudo insmod *.ko
```

