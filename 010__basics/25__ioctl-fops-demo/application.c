// SPDX-License-Identifier: GPL-2.0+
/*
  The Userspace implementation to the loadable kernel module.

  References:
  https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
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

	close(fd);
	fprintf(stdout, "READY.\n");
	exit(EXIT_SUCCESS);
}
