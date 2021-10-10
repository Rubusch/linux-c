// cat_noblock.c
/*
  cat_noblock.c - open a file and display its contents, but exit rather than wait for input

  userspace part of the noblocking demo (kernel module)

  copyright (C) 1998 by Ori Pomeranz
//*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h> // open
#include <unistd.h> // read
#include <stdlib.h> // exit
#include <errno.h> // errno

#define MAX_BYTES 1024 * 4

int main(int argc, char **argv)
{
	// file descriptor for the file to read
	int fd = 0;

	// the number of bytes read
	size_t bytes = 0;

	// buffer for the content to read
	char buffer[MAX_BYTES];
	memset(buffer, '\0', MAX_BYTES);

	if (argc != 2) {
		printf("usage: %s <filename>\n", argv[0]);
		puts("reads the content of a file, but doesn't wait for input");
		exit(EXIT_FAILURE);
	}

	// open the file for reading in non blocking mode
	fd = open(argv[1], O_RDONLY | O_NONBLOCK);

	// if open failed
	if (fd == -1) {
		if (errno == EAGAIN) {
			puts("open would block");
		} else {
			puts("open failed");
		}
		exit(EXIT_FAILURE);
	}

	// read the file and output its contents
	do {
		int idx = 0;

		// read characters from the file
		bytes = read(fd, buffer, MAX_BYTES);

		// if there's an error, report it and die
		if (bytes == -1) {
			if (errno == EAGAIN) {
				puts("normally I'd block, but you told me not to");
			} else {
				puts("another read error");
			}
			exit(EXIT_FAILURE);
		}

		// print the characters
		if (bytes > 0) {
			for (idx = 0; idx < bytes; ++idx) {
				putchar(buffer[idx]);
			}
		}
		// while there are no errors and the file isn't over
	} while (bytes > 0);

	exit(EXIT_SUCCESS);
}
