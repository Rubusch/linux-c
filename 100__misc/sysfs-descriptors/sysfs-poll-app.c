#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "sysfs_common.h"

#define TEST_SYSFS_TRIGGER  "/sys/" SYSFS_NODE_NAME "/trigger"
#define TEST_SYSFS_NOTIFY   "/sys/" SYSFS_NODE_NAME "/notify"

int main(int argc, char *argv[])
{
	int cnt, notify_fd, trigger_fd, rv;
	char attrData[100];

	// prepare pollfd
	struct pollfd ufds[2];

	// obtain descriptor for notify, trigger, etc.
	if (0 > (notify_fd = open(TEST_SYSFS_NOTIFY, O_RDWR))) {
		perror("open() notify failed");
		exit(EXIT_FAILURE);
	}
	if (0 > (trigger_fd = open(TEST_SYSFS_TRIGGER, O_RDWR))) {
		perror("open() trigger failed");
		exit(EXIT_FAILURE);
	}

	// init pollfd with descriptors
	ufds[0].fd = notify_fd;
	ufds[0].events = POLLPRI|POLLERR;
	ufds[1].fd = trigger_fd;
	ufds[1].events = POLLPRI|POLLERR;

	// we first need to read data until the end of the file
	cnt = read(notify_fd, attrData, 100);
	if (0 > cnt) {
		perror("first read() failed");
		exit(EXIT_FAILURE);
	}

	cnt = read(trigger_fd, attrData, 100);
	if (0 > cnt) {
		perror("second read() failed");
		exit(EXIT_FAILURE);
	}

	// start with empty revents counter
	ufds[0].revents = 0;
	ufds[1].revents = 0;

	// poll, wait actively for the descriptors become ready
	if (0 > (rv = poll( ufds, 2, 1000000)))
		perror("poll error");
	else if (0 == rv)
		printf("%s: timeout occurred!\n", __FILE__);
	else
		printf("%s: triggered\n", __FILE__);

	printf( "%s: revents[0]: %08X\n", __FILE__, ufds[0].revents);
	printf( "%s: revents[1]: %08X\n", __FILE__, ufds[1].revents);

	// done
	close( trigger_fd );
	close( notify_fd );

	exit(EXIT_SUCCESS);
}
