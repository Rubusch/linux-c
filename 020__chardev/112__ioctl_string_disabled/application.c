/*
  The Userspace implementation to the loadable kernel module.

  THIS CODE RUNS IN USERSPACE!!

  Until now we could have used cat for input and output, but now we
  need to do ioctl's which require writing our own process.

  ---
  References:
  Linux Kernel Module Programming Guide, Peter Jay Salzman, 2007-05-18
  Highly inspired by / many thanks to embetronicx.com (2021) - https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver
*/

// device specifics such as ioctl numbers and major device file
#include "helloioctl.h"

/*
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // open

#include <unistd.h> // exit
#include <sys/ioctl.h> // ioctl
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "helloioctl.h"

/*
  ioctl call to set a message

int ioctl_set_msg(int file_desc, char* message)
{
	int ret = 0;

	if (0 > (ret = ioctl(file_desc, IOCTL_SET_MSG, message))) {
		printf("ioctl_set_msg failed: %d\n", ret);
		exit(EXIT_FAILURE);
	}
}
// */

/*
  ioctl call to get the message

int ioctl_get_msg(int file_desc)
{
	int ret = 0;
	char message[100];
	memset(message, '\0', 100);

//	  WARNING:
//	  this is dangerous because we don't tell the kernel how far
//	  it's allowed to write, so it might overflow the buffer
//
//	  in a real production program, we would have used two ioctls
//	  - one to tell the kernel the buffer length and another to
//	  give it the buffer to fill
	if (0 > (ret = ioctl(file_desc, IOCTL_GET_MSG, message))) {
		printf("ioctl_get_msg failed: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	printf("get_msg message: %s\n", message);
}
// */

/*
  ioctl call to read characterwise

int ioctl_get_nth_byte(int file_desc)
{
	int idx = 0;
	char ch = 0;

	printf("get_nth_byte message:");
	do {
		if (0 > (ch = ioctl(file_desc, IOCTL_GET_NTH_BYTE, ++idx))) {
			printf("ioctl_get_nth_byte failed at the %d'th byte:\n", idx);
			exit(EXIT_FAILURE);
		}

		putchar(ch);
	} while (ch != 0);
	putchar('\n');
}
// */

/*
  main()
*/
int main(void)
{
	int fd = 0;
	int32_t value;
	int32_t number;
	//	int ret=0;
	//	char* msg = "message passed by ioctl\n";
	char device_name[32];

	memset(device_name, '\0', sizeof(device_name));
	strcat(device_name, "/dev/");
	strcat(device_name, HELLO_DEVICE_NAME);

	fprintf(stdout, "XXX device_name '%s'\n", device_name);

	if (0 > (fd = open(device_name, O_RDWR))) {
		perror("open failed");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "enter a character to send to ioctl device:\n");
	scanf("%d", &number);
	fprintf(stdout, "writing\n");
	ioctl(fd, WR_VALUE, (int32_t *)&number);

	fprintf(stdout, "reading\n");
	ioctl(fd, RD_VALUE, (int32_t *)&value);
	fprintf(stdout, "value = %d\n", value);

	/*
	fd = open(HELLO_DEVICE_FILENAME, 0);
	if (0 > fd) {
		printf("can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(EXIT_FAILURE);
	}

	ioctl_get_nth_byte(fd);
	ioctl_get_msg(fd);
	ioctl_set_msg(fd, msg);
// */
	close(fd);
	fprintf(stdout, "READY.\n");
	exit(EXIT_SUCCESS);
}
