// SPDX-License-Identifier: GPL-2.0+
/*
  The userspace application to test the chardev device file.
*/

//#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <fcntl.h> /* open() */
#include <unistd.h> /* close(), read(), write() */

#include "hellochardev.h"

u_int8_t read_buf[KERNEL_BUF_SIZE];
u_int8_t write_buf[KERNEL_BUF_SIZE];

int main()
{
	int fd;
	char filename[FILENAME_MAX];

	memset(filename, '\0', FILENAME_MAX);
	memset((char *)read_buf, '\0', sizeof(read_buf));
	memset((char *)write_buf, '\0', sizeof(write_buf));

	sprintf(filename, "/dev/%s", HELLO_DEVICE_NAME);

	fd = open(filename, O_RDWR);
	if (0 > fd) {
		fprintf(stderr, "cannot open device file '%s'.\n", filename);
		return -1;
	}

	// read
	memset(read_buf, '\0', sizeof(read_buf));
	read(fd, read_buf, KERNEL_BUF_SIZE);
	fprintf(stdout, "READ: '%s' [%s]\n", (char *)read_buf, "0");

	// write
	strcpy((char *)write_buf, "123");
	fprintf(stdout, "WRITING '%s' [123]\n", (char *)write_buf);
	write(fd, write_buf, 1 + strlen((char *)write_buf));
	memset(write_buf, '\0', sizeof(write_buf));

	// read
	memset(read_buf, '\0', sizeof(read_buf));
	read(fd, read_buf, KERNEL_BUF_SIZE);
	fprintf(stdout, "READ: '%s' [123]\n", (char *)read_buf);

	close(fd);
	fprintf(stdout, "READY.\n");
	exit(EXIT_SUCCESS);
}
