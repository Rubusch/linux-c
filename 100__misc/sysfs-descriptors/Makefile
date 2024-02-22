KERNELDIR ?= /usr/src/linux

## userspace
APP_SELECT ?= sysfs-select.elf
APP_POLL ?= sysfs-poll.elf
APP_EPOLL ?= sysfs-epoll.elf
HEADERS += sysfs_common.h

CC ?= gcc
HOSTCC ?= gcc

all: default

default: module $(APP_SELECT) $(APP_POLL) $(APP_EPOLL)

$(APP_SELECT): sysfs-select-app.c
	$(CROSS_COMPILE)$(HOSTCC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

$(APP_POLL): sysfs-poll-app.c
	$(CROSS_COMPILE)$(HOSTCC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

$(APP_EPOLL): sysfs-epoll-app.c
	$(CROSS_COMPILE)$(HOSTCC) -g -Wall -std=c11 -pedantic -Werror $^ -o $@

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete
	rm -f *.elf

.PHONY: all default module clean
