// SPDX-License-Identifier: GPL-2.0+
/*
  The Userspace implementation to the loadable kernel module.

  THIS CODE RUNS IN USERSPACE!!

  Until now we could have used cat for input and output, but now we
  need to do ioctl's which require writing our own process.

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
  Highly inspired by / many thanks to embetronicx.com (2021) - https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
*/

// device specifics such as ioctl numbers and major device file
#include "helloioctl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> /* int32_t, ... */
#include <fcntl.h> /* open() */
#include <unistd.h> /* close() */

#include "helloioctl.h"

/*
  main()
*/
int main(void)
{
	int fd = 0;
	int32_t value;
	int32_t number;
	char device_name[32];

	memset(device_name, '\0', sizeof(device_name));
	strcat(device_name, "/dev/");
	strcat(device_name, HELLO_DEVICE_NAME);

	fprintf(stdout, "device name: '%s'\n", device_name);
	if (0 > (fd = open(device_name, O_RDWR))) {
		perror("open failed");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "enter a number to be sent to the ioctl device:\n");
	scanf("%d", &number);
	fprintf(stdout, "writing\n");
	ioctl(fd, WR_VALUE, (int32_t *)&number);

	fprintf(stdout, "reading\n");
	ioctl(fd, RD_VALUE, (int32_t *)&value);
	fprintf(stdout, "value = %d\n", value);

/*
	fd = open(HELLO_DEVICE_FILENAME, 0);
	if (0 > fd) {
		printf("can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(EXIT_FAILURE);
	}

	ioctl_get_nth_byte(fd);
	ioctl_get_msg(fd);
	ioctl_set_msg(fd, msg);
// */

	close(fd);
	fprintf(stdout, "READY.\n");
	exit(EXIT_SUCCESS);
}
