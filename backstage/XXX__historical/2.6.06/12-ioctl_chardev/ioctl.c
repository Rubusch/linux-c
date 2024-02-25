// ioctl.c
/*
  the process to use ioctl's to control the kernel module

  hence this code runs in USER SPACE!!!

  until now we could have used cat for input and output, but
  now we need to do ioctl's which require writing our own process

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/

// device specifics such as ioctl numbers and major device file
#include "helloioctl.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // open

#include <unistd.h> // exit
#include <sys/ioctl.h> // ioctl

/*
  ioctl call to set a message
//*/
int ioctl_set_msg(int file_desc, char *message)
{
	int ret = 0;

	if (0 > (ret = ioctl(file_desc, IOCTL_SET_MSG, message))) {
		printf("ioctl_set_msg failed: %d\n", ret);
		exit(EXIT_FAILURE);
	}
}

/*
  ioctl call to get the message
//*/
int ioctl_get_msg(int file_desc)
{
	int ret = 0;
	char message[100];
	memset(message, '\0', 100);

	/*
    WARNING: 
    this is dangerous because we don't tell the kernel how far 
    it's allowed to write, so it might overflow the buffer
  
    in a real production program, we would have used two ioctls - one 
    to tell the kernel the buffer length and another to give it 
    the buffer to fill
  //*/
	if (0 > (ret = ioctl(file_desc, IOCTL_GET_MSG, message))) {
		printf("ioctl_get_msg failed: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	printf("get_msg message: %s\n", message);
}

/*
  ioctl call to read characterwise
//*/
int ioctl_get_nth_byte(int file_desc)
{
	int idx = 0;
	char ch = 0;

	printf("get_nth_byte message:");
	do {
		if (0 > (ch = ioctl(file_desc, IOCTL_GET_NTH_BYTE, ++idx))) {
			printf("ioctl_get_nth_byte failed at the %d'th byte:\n",
			       idx);
			exit(EXIT_FAILURE);
		}

		putchar(ch);
	} while (ch != 0);
	putchar('\n');
}

/*
  main()
//*/
int main(void)
{
	int fd = 0, ret = 0;
	char *msg = "message passed by ioctl\n";

	fd = open(DEVICE_FILE_NAME, 0);
	if (0 > fd) {
		printf("can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(EXIT_FAILURE);
	}

	ioctl_get_nth_byte(fd);
	ioctl_get_msg(fd);
	ioctl_set_msg(fd, msg);

	close(fd);
}
