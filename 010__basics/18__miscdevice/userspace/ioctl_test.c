// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVNAME "/dev/lothars_device"

int main()
{
	/*
	 * usage:
	 * $ mknod /dev/mydev c 202 0
	 * $ ./ioctl_test.elf
	 */
	int my_dev = open(DEVNAME, 0);
	if (0 > my_dev) {
		perror("failed to open device");
		exit(EXIT_FAILURE);
	}
	ioctl(my_dev, 100, 110); // cmd = 100, arg = 110
	close(my_dev);
	exit(EXIT_SUCCESS);
}
