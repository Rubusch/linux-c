/*
  The Userspace implementation to the loadable kernel module.

  THIS CODE RUNS IN USERSPACE!!

  Until now we could have used cat for input and output, but now we
  need to do ioctl's which require writing our own process.

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
  Highly inspired by / many thanks to www.embetronicx.com (2021)
  stackoverflow..
*/

#include "helloioctl.h"

static int done = 0;
int check = 0;

void sig_event_handler(int sig, siginfo_t *info, void *context)
{
	if (SIG_LOTHAR == sig) {
		check = info->si_int;
		fprintf(stdout,
			"APP: %s() - received signal from kernel: value = %u\n",
			__func__, check);
	}
	fprintf(stdout, "APP: %s() - READY.\n", __func__);
	exit(EXIT_SUCCESS);
}

/*
  main()
*/
int main(void)
{
	int fd = 0;
	int32_t number;
	char device_name[32];
	struct sigaction sa;

	memset(device_name, '\0', sizeof(device_name));
	strcat(device_name, "/dev/");
	strcat(device_name, HELLO_DEVICE_NAME);

	// signal handler - install customized signal handler
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sa.sa_sigaction = sig_event_handler;
	sigaction(SIG_LOTHAR, &sa, NULL);

	// ioctl - connect to the kernel (register application)
	fprintf(stdout, "APP: device name: '%s'\n", device_name);
	if (0 > (fd = open(device_name, O_RDWR))) {
		perror("open failed");
		exit(EXIT_FAILURE);
	}
	number = SIG_LOTHAR;
	fprintf(stdout, "APP: writing to ioctl (register application) %d\n",
		number);
	ioctl(fd, WR_VALUE, (int32_t *)&number);

	// wait for signals...
	fprintf(stdout, "APP: waiting for signal...\n");
	while (!done) {
		sleep(10);
	}

	close(fd);
	fprintf(stdout, "APP: READY.\n");
	exit(EXIT_SUCCESS);
}
