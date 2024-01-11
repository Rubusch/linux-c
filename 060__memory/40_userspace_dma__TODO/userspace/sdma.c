/*
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define SDMA_BUF_SIZE 4096
//#define SDMA_BUF_SIZE (1024*63)

#define SDMA_DEV "/dev/sdma_test"

#define handle_error(msg)					\
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(void)
{
	char *virtaddr;
	char phrase[128];
	int lothars_dev;// = open(SDMA_DEV, O_RDWR);
	size_t length;
	ssize_t s;
	off_t offset, pa_offset;
//	struct stat sb;

	fprintf(stderr, "trying to open device '%s'\n", SDMA_DEV);
	lothars_dev = open(SDMA_DEV, O_RDWR);
	if (0 > lothars_dev) {
		handle_error("open()");
	}

//	if (-1 == fstat(lothars_dev, &sb)) // to obtain the file size
//		handle_error("fstat()");

	printf("enter phrase: \n");
	scanf("%[^\n]%*c", phrase);

	fprintf(stderr, "XXX phrase '%s'\n", phrase);
	length = strlen(phrase);
	fprintf(stderr, "XXX length '%ld'\n", length);

	offset = 0; // start from begin
	pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

//	fprintf(stderr, "XXX sb.st_size '%ld'\n", sb.st_size);
//	if (offset >= sb.st_size) {
//		fprintf(stderr, "FAILED! offset is past end of file!\n");
//		exit(EXIT_FAILURE);
//	}

	/*
	  offset for mmap() must be page aligned
	  ref: manpage mmap()
	*/
	fprintf(stderr, "XXX pa_offset '%ld'\n", pa_offset);

	/*
	  shared file mapping
	  -> memory-mapped I/O
	  -> kernel/user or IPC
	*/
	virtaddr = (char*) mmap(NULL,
//				length + offset - pa_offset,
				SDMA_BUF_SIZE,
				PROT_READ|PROT_WRITE,
				MAP_SHARED, lothars_dev, 0);
	if (MAP_FAILED == virtaddr) {
		handle_error("mmap()");
	}

//	strcpy(virtaddr, phrase);
		
//	s = write(lothars_dev, phrase, length);   
	s = write(STDOUT_FILENO, virtaddr + offset - pa_offset, length);
//	s = write(lothars_dev, virtaddr + offset - pa_offset, length);
	if (s != length) {
		if (s == -1)
			handle_error("write()");

		fprintf(stderr, "partial write");
		exit(EXIT_FAILURE);
	}

	ioctl(lothars_dev, 0); // NULL ~ 0, why? prefer sync?
	close(lothars_dev);

	exit(EXIT_SUCCESS);
}
