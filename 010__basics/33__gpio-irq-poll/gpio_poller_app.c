#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <poll.h>

#include "gpio_irq_poll.h"

int main()
{
	int fd;
	struct pollfd polldesc;

	fd = open(POLL_DEV_NODE, O_RDONLY);
	if (fd < 0) {
		perror("open() failed");
		exit(EXIT_FAILURE);
	}

	memset(&polldesc, 0, sizeof(polldesc));
	polldesc.fd = fd;
	polldesc.events = POLLIN;

	fprintf(stderr, "%s: wait for signal...\n", __FILE__);
	poll(&polldesc, 1, -1);
	fprintf(stderr, "%s: button pressed\n", __FILE__);

	exit(EXIT_SUCCESS);
}


