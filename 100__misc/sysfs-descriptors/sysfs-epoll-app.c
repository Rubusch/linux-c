/*
  Level triggered and edge triggered event notifications

  Level-triggered and edge-triggered are terms borrowed from
  electrical engineering. When we’re using epoll the difference is
  important. In edge triggered mode we will only receive events when
  the state of the watched file descriptors change; whereas in level
  triggered mode we will continue to receive events until the
  underlying file descriptor is no longer in a ready state. Generally
  speaking level triggered is the default and is easier to use and is
  what I’ll use for this tutorial, though it’s good to know edge
  triggered mode is available.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "sysfs_common.h"

#define TEST_SYSFS_TRIGGER  "/sys/" SYSFS_NODE_NAME "/trigger"
#define TEST_SYSFS_NOTIFY   "/sys/" SYSFS_NODE_NAME "/notify"

#define NDESCRIPTORS 2
#define MAX_EVENTS 5

int main (int argc, char* argv[])
{
	int epfd;
	int cnt, ev_count;
	int notify_fd, trigger_fd;
	struct epoll_event ev;
	struct epoll_event evlist[MAX_EVENTS];
	char attrData[100];

	// prepare an fd_set
	epfd = epoll_create(NDESCRIPTORS-1);
        if (-1 == epfd) {
                perror("epoll_create() failed");
                exit(EXIT_FAILURE);
        }

	// obtain descriptor for notify
	if (0 > (notify_fd = open(TEST_SYSFS_NOTIFY, O_RDWR))) {
		perror("open() notify failed");
		exit(EXIT_FAILURE);
	}
	ev.events = EPOLLIN; // only interested in input events
	ev.data.fd = notify_fd;
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, notify_fd, &ev)) {
		fprintf(stderr, "epoll_ctl() failed\n");
		exit(EXIT_FAILURE);
	}

	// obtan descriptor for trigger
	if (0 > (trigger_fd = open(TEST_SYSFS_TRIGGER, O_RDWR))) {
		perror("open() trigger failed");
		exit(EXIT_FAILURE);
	}
	ev.events = EPOLLIN; // only interested in input events
	ev.data.fd = trigger_fd;
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, trigger_fd, &ev)) {
		fprintf(stderr, "epoll_ctl() failed\n");
		exit(EXIT_FAILURE);
	}

	// we first need to read data until the end of the file
	cnt = read(evlist[0].data.fd, attrData, 100);
	if (0 > cnt) {
		perror("first read() failed");
		exit(EXIT_FAILURE);
	}

	cnt = read(evlist[1].data.fd, attrData, 100);
	if (0 > cnt) {
		perror("second read() failed");
		exit(EXIT_FAILURE);
	}

	// waiting on events
	if (0 > (ev_count = epoll_wait(epfd, evlist, MAX_EVENTS, -1)))
		perror("poll error");
	else if (0 == ev_count)
		printf("%s: timeout occurred!\n", __FILE__);
	else
		printf("%s: triggered\n", __FILE__);

	printf( "%s: epoll_events[0]: %08X\n", __FILE__, evlist[0].data.fd);
	printf( "%s: epoll_events[1]: %08X\n", __FILE__, evlist[1].data.fd);

	// done
	close(trigger_fd);
	close(notify_fd);

	exit(EXIT_SUCCESS);
}
