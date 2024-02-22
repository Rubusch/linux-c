#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include "sysfs_common.h"

#define TEST_SYSFS_TRIGGER  "/sys/" SYSFS_NODE_NAME "/trigger"
#define TEST_SYSFS_NOTIFY   "/sys/" SYSFS_NODE_NAME "/notify"

int main (int argc, char* argv[])
{
	fd_set read_fds;
	int max_fd, ret, cnt;
	int notify_fd, trigger_fd;
	char attrData[100];

	// prepare an fd_set
	FD_ZERO (&read_fds);

	// obtain descriptor for notify, trigger, etc.
	if (0 > (notify_fd = open(TEST_SYSFS_NOTIFY, O_RDWR))) {
		perror("open() notify failed");
		exit(EXIT_FAILURE);
	}
	if (0 > (trigger_fd = open(TEST_SYSFS_TRIGGER, O_RDWR))) {
		perror("open() trigger failed");
		exit(EXIT_FAILURE);
	}

	// init fd_set with descriptors
	FD_SET (notify_fd, &read_fds);
	FD_SET (trigger_fd, &read_fds);

	// obtain highest descriptor, used for select()
	max_fd = notify_fd > trigger_fd ? notify_fd : trigger_fd;

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

	// select, wait on descriptors become ready
	ret = select(max_fd + 1, NULL, NULL, &read_fds, NULL);
	if (0 > ret)
		perror("select() events");
	if (FD_ISSET(notify_fd, &read_fds))
		printf("%s: change detected in /sys/.../notify\n",
			__FILE__);
	if (FD_ISSET(trigger_fd, &read_fds))
		printf("%s: change detected in /sys/.../trigger\n",
			__FILE__);

	// done
	close(trigger_fd);
	close(notify_fd);

	exit(EXIT_SUCCESS);
}
