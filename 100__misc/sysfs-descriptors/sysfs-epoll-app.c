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

  NB: epoll() is mainly used for sockets and not for files / file
  descriptors

  This demo does not actually work, since reading out on the sysfs
  descriptors ALWAYS will have a valid descriptor to read out. So
  epoll() does  not seem to block to wait. Anyway it shows that there
  were no new events.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdbool.h>

#include "sysfs_common.h"

#define TEST_SYSFS_TRIGGER  "/sys/" SYSFS_NODE_NAME "/trigger"
#define TEST_SYSFS_NOTIFY   "/sys/" SYSFS_NODE_NAME "/notify"

#define NDESCRIPTORS 2
#define MAX_EVENTS 5

int main (int argc, char* argv[])
{
	int epfd;
	int cnt;
	int ev_count;
	int notify_fd, trigger_fd;
	struct epoll_event ev;
	struct epoll_event evlist[MAX_EVENTS];
	char attrData[100];

	// prepare an fd_set
	epfd = epoll_create(NDESCRIPTORS - 1);
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

	/*
	  epoll_wait() does not really make sense here, since
	  there will always be data available (at least that
	  there are no data), so it always returns, but the
	  counter is incremented after a trigger.
	*/
	if (0 > (ev_count = epoll_wait(epfd, evlist, MAX_EVENTS, 100))) {
		perror("poll error");
		exit(EXIT_FAILURE);
	}

	printf("%s: triggered %d\n", __FILE__, ev_count);

	// we first need to read data until the end of the file
	cnt = read(evlist[0].data.fd, attrData, 100);
	if (0 > cnt) {
		perror("first read() failed");
		exit(EXIT_FAILURE);
	}
	printf("%s: '%s'\n", __FILE__, attrData);

	cnt = read(evlist[1].data.fd, attrData, 100);
	if (0 > cnt) {
		perror("second read() failed");
		exit(EXIT_FAILURE);
	}
	printf("%s: '%s'\n", __FILE__, attrData);

	printf("%s: epoll_events[0]: %08X\n", __FILE__, evlist[0].data.fd);
	printf("%s: epoll_events[1]: %08X\n", __FILE__, evlist[1].data.fd);

	close(trigger_fd);
	close(notify_fd);

	exit(EXIT_SUCCESS);
}
