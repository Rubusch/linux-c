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
	int notify_fd;
	int trigger_fd;
	struct epoll_event ev;
	struct epoll_event evlist[MAX_EVENTS];
	char attrData[100];

	// prepare an fd_set
	epfd = epoll_create(NDESCRIPTORS);
        if (-1 == epfd) {
                perror("epoll_create() failed");
                exit(EXIT_FAILURE);
        }

	// obtain descriptor for notify
	if (0 > (notify_fd = open(TEST_SYSFS_NOTIFY, O_RDWR))) {
		perror("open() notify failed");
		exit(EXIT_FAILURE);
	}
	/*
	  Event types

	  EPOLLERR
	  An error condition occurred on the target file
	  descriptor. It shall not be necessary to set this event in
	  events; this interface shall always wait for it.

	  EPOLLET
	  This event shall set edge-triggered behavior for the target
	  file descriptor. The default epoll behavior shall be
	  level-triggered.

	  EPOLLHUP
	  A hang up occurred on the target file descriptor. It shall
	  not be necessary to set this event in events; this interface
	  shall always wait for it.

	  EPOLLIN
	  The file is accessible to read() operations.

	  EPOLLONESHOT
	  This event shall set one-shot behavior for the target file
	  descriptor. After epoll_wait() retrieves an event, the file
	  descriptor shall be disabled and epoll shall not report any
	  other events. To reenable the file descriptor with a new
	  event mask, the user should invoke epoll_ctl() with
	  EPOLL_CTL_MOD in the op parameter.

	  EPOLLOUT
	  The file is accessible to write() operations.

	  EPOLLPRI
	  Urgent data exists for read() operations.

	  EPOLLRDHUP
	  A stream socket peer closed the connection, or else the peer
	  shut down the writing half of the connection.
	 */
	ev.events = EPOLLHUP; // only interested in input events
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
	ev.events = EPOLLHUP; // only interested in input events
	ev.data.fd = trigger_fd;
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, trigger_fd, &ev)) {
		fprintf(stderr, "epoll_ctl() failed\n");
		exit(EXIT_FAILURE);
	}

	// we first need to read data until the end of the file
	cnt = read(notify_fd, attrData, 100);
	if (0 > cnt) {
		perror("initial notify read() failed");
		exit(EXIT_FAILURE);
	}

	cnt = read(trigger_fd, attrData, 100);
	if (0 > cnt) {
		perror("initial trigger read() failed");
		exit(EXIT_FAILURE);
	}

	/*
	  epoll_wait() does not really make sense here, since
	  there will always be data available (at least that
	  there are no data), so it always returns, but the
	  counter is incremented after a trigger.
	*/
	if (0 > (ev_count = epoll_wait(epfd, evlist, MAX_EVENTS, 3000000))) {
		perror("poll error");
		exit(EXIT_FAILURE);
	}
	printf("%s: epoll triggered %d times\n", __FILE__, ev_count);

	printf("%s: epoll_events[0]: %08X\n", __FILE__, evlist[0].data.fd);
	printf("%s: epoll_events[1]: %08X\n", __FILE__, evlist[1].data.fd);

	close(notify_fd);
	close(trigger_fd);

	exit(EXIT_SUCCESS);
}
