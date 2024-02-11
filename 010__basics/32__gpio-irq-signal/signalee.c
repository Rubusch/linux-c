#define _XOPEN_SOURCE /* sigaction */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> /* O_RDONLY */
#include <sys/ioctl.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#include "gpio_irq_signaler.h"

void sighandler()
{
	fprintf(stderr, "%s: button pressed\n",
		__FILE__); // NB: never print in signalhandler!
}

int main() {
	int fd;

	// async save sighandler (...)
	struct sigaction sa;
	sa.sa_handler = sighandler;
	sa.sa_flags = 0;
	sigaction(SIG_NUM, &sa, NULL);

	fprintf(stderr, "%s: pid: %d\n", __FILE__, getpid());
	fd = open(GPIO_DEV_PATH, O_RDONLY);
	if (0 > fd) {
		perror("open() failed");
		exit(EXIT_FAILURE);
	}

	// register app
	if (ioctl(fd, REGISTER_UAPP, NULL)) {
		perror("ioctl() failed");
		close(fd);
		exit(EXIT_FAILURE);
	}

	// loop
	fprintf(stderr, "%s: wait for signal...\n", __FILE__);
	while(true)
		sleep(1);

	exit(EXIT_SUCCESS);
}


