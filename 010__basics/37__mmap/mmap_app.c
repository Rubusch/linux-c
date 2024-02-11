#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "mmap_dev.h"

int main(int argc, char **argv)
{
	int fd, status, offset;
	char text[4096];
	void *ptr;

	if (2 > argc) {
		printf("usage: %s [m,r,w,p] {data}\n", argv[0]);
		return 0;
	}

	fd = open(MMAP_DEV_NODE, O_RDWR);
	if (0 > fd) {
		perror("open() failed");
		return 1;
	}

	switch (argv[1][0]) {
		case 'r':
			status = read(fd, text, 4096);
			fprintf(stderr, "%s: READ: I got %d bytes with '%s'\n",
				__FILE__, status, text);
			break;
		case 'w':
			if (3 > argc) {
				printf("usage: %s w [data]\n", argv[0]);
				break;
			}

			memset(text, 0, 4096);
			strcpy(text, argv[2]);

			status = write(fd, text, 4096);
			fprintf(stderr, "%s: wrote %d bytes\n", __FILE__, status);
			break;
		case 'm':
			ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
				   MAP_SHARED, fd, 0);
			if (MAP_FAILED == ptr) {
				perror("mmap() failed");
				break;
			}

			if (2 < argc) {
				memset(text, 0, 4096);
				strcpy(text, argv[2]);
				memcpy(ptr, text, 4096);
			}

			memset(text, 0, 4096);
			memcpy(text, ptr, 4096);
			fprintf(stderr, "%s: got '%s'\n", __FILE__, text);
			munmap(ptr, 4096);
			break;
		case 'p':
			if (3 > argc) {
				printf("usage: %s p [offset]\n", argv[0]);
				break;
			}
			offset = atoi(argv[2]);
			fprintf(stderr, "%s: offset '%d'\n", __FILE__, offset);

			ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
				   MAP_SHARED, fd, 0);
			if (ptr == MAP_FAILED) {
				perror("mmap() failed");
				break;
			}

			fprintf(stderr,
				"%s: byte on offset %d (dec): *(ptr + 0x%02x) = '0x%02x'\n",
				__FILE__, offset, offset,
				*((char*) ptr + offset));

			munmap(ptr, 4096);
			break;
		default:
			fprintf(stderr, "%s: '%c' is invalid.\n",
				__FILE__, argv[1][0]);
			break;
	}

	close(fd);
}
