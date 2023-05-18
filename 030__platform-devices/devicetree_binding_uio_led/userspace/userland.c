/*
  userland (rpi3/4 version)

  The demo shows how to map memory from uio to a handle and then
  provide read/write access to that handle, where a DT probed kernel
  driver will then turn off/off the GPIO27 e.g. a connected red led.

  UIO demo application (based on A. Rios, p. 201)
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>

#define BUFFER_LENGTH         128

#define GPIO_27               27
#define GPIO_22               22
#define GPIO_26               26

#define GPFSEL2_offset        0x08
#define GPSET0_offset         0x1C
#define GPCLR0_offset         0x28

// specify set each individual color led
#define GPIO_27_INDEX         1 << (GPIO_27 % 32)
//#define GPIO_22_INDEX         1 << (GPIO_22 % 32)  
//#define GPIO_26_INDEX         1 << (GPIO_26 % 32)  

// select output function
#define GPIO_27_FUNC          1 << ((GPIO_27 % 10) * 3)
//#define GPIO_22_FUNC          1 << ((GPIO_22 % 10) * 3)  
//#define GPIO_26_FUNC          1 << ((GPIO_26 % 10) * 3)  

// specify mask for each individual color led
#define FSEL_27_MASK          0b111 << ((GPIO_27 % 10) * 3)   /* red since bit 21 [FSEL27] */
//#define FSEL_22_MASK          0b111 << ((GPIO_22 % 10) * 3)   /* green since bit 6 [FSEL22] */  
//#define FSEL_26_MASK          0b111 << ((GPIO_26 % 10) * 3)   /* blue since bit 18 [FSEL26] */  

// setup some convenience helpers
//#define GPIO_SET_FUNCTION_LEDS (GPIO_27_FUNC | GPIO_22_FUNC | GPIO_26_FUNC)  
#define GPIO_SET_FUNCTION_LEDS (GPIO_27_FUNC)
//#define GPIO_MASK_ALL_LEDS (FSEL_27_MASK | FSEL_22_MASK | FSEL_26_MASK)  
#define GPIO_MASK_ALL_LEDS (FSEL_27_MASK)
//#define GPIO_SET_ALL_LEDS (GPIO_27_INDEX | GPIO_22_INDEX | GPIO_26_INDEX)  
#define GPIO_SET_ALL_LEDS (GPIO_27_INDEX)

// obtain the uio size via sysfs
#define SYSFS_UIO_SIZE "/sys/class/uio/uio0/maps/map0/size"


/*
  fgetc() reads the next character from stream and returns it as an
  unsigned char cast to an int, or EOF on end of file or error.

  This wrapper reads a string from stdin input via fgetc by a given
  size.

  #include <stdio.h>

  @str: Where the read string is stored to.
  @strsize: The size of the string to read.
  @prompt: Some text to ask for input, no terminating EOL is needed.
*/
void readstring(char *str, const unsigned int strsize, const char *prompt)
{
	if (NULL == prompt) {
		perror("text is NULL");
		return;
	}
	if (NULL == str) {
		perror("iTxt is NULL");
		return;
	}

	do {
		// reset
		memset(str, '\0', strsize);
		fprintf(stdout, "%s\n", prompt);

		// read in
		register unsigned int c;
		register unsigned int idx = 0;

		// in case of exceeding the size of the variable - put
		// a '\0' at the end and read until '\n', but don't
		// store the characters for cleaning the stream
		for (idx = 0; ('\n' != (c = fgetc(stdin))); ++idx) {
			if ((strsize - 1) > idx) {
				// append to string
				str[idx] = c;
			} else if ((strsize - 1) == idx) {
				/* // either: input too long, reset all
				fprintf(stdout, "input too long - will be reset\n");
				memset(str, '\0', strsize);
				/*/ // or: terminate string
				str[idx] = '\0';
				// */
			}
		}

	} while (0 == strlen(str));
}


int main()
{
	int ret, devuio_fd, mem_fd;
	size_t uio_size;
	void *temp;
	int GPFSEL_read, GPFSEL_write;
	void *demo_driver_map; // the handle to the mapped memory
                               // in case use: char* demo_driver_map
	char sendstring[BUFFER_LENGTH];
	char *led_on = NULL;
	char *led_off = NULL;
	char *do_exit = "exit";

	fprintf(stderr, "%s(): started\n", __func__);

	// opening /dev/mem
	mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (0 > mem_fd) {
		perror("open() /dev/mem failed!");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "%s(): /dev/mem opened\n", __func__);

	// opening /dev/uio0
	devuio_fd = open("/dev/uio0", O_RDWR|O_SYNC);
	if (0 > devuio_fd) {
		perror("open() /dev/uio0 failed!");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "%s(): /dev/uio0 opened\n", __func__);

	// read the size that has to be mapped
	FILE *size_fp = fopen(SYSFS_UIO_SIZE, "r");
	fscanf(size_fp, "0x%lx", &uio_size); // read sysfs passed size by formatted scanf (hex) from size_fp into uio_size
	fclose(size_fp);
	fprintf(stderr, "%s(): the size read from '%s' is 0x%ld\n", __func__, SYSFS_UIO_SIZE, uio_size);

	// memory mapping
	demo_driver_map = mmap(0, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, devuio_fd, 0);
	if (MAP_FAILED == demo_driver_map) {
		perror("mmap() failed to map the devuio memory");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "%s(): memory mapped to 'demo_driver_map'\n", __func__);

	// obtain the read setup
	GPFSEL_read = *(int*) (demo_driver_map + GPFSEL2_offset);

	// update to the write options: clear all leds and functions
	GPFSEL_write = (GPFSEL_read & ~GPIO_MASK_ALL_LEDS) | (GPIO_SET_FUNCTION_LEDS & GPIO_MASK_ALL_LEDS);

	// init with the write options: set dir leds to output
	*(int*) (demo_driver_map + GPFSEL2_offset) = GPFSEL_write;

	// clear all the leds (output is low)
	*(int*) (demo_driver_map + GPCLR0_offset) = GPIO_SET_ALL_LEDS;

	// control loop - some interactive hokuspokus (just shaky demo code here)
	char prompt[128]; memset(prompt, '\0', 128);
	sprintf(prompt, "%s(): enter a led value [on|off|exit]\n", __func__);
	do {
		readstring(sendstring, BUFFER_LENGTH, prompt);
		if (0 == strncmp(led_on, sendstring, 3)) {
			temp = demo_driver_map + GPSET0_offset;
			*(int*) temp = GPIO_27_INDEX; // set GPIO27 on at GPSET0_offset

		} else if (0 == strncmp(led_off, sendstring, 3)) {
			temp = demo_driver_map + GPCLR0_offset;
			*(int*) temp = GPIO_27_INDEX; // set GPIO27 off at GPCLR0_offset

		} else if (0 == strncmp(do_exit, sendstring, 4)) {
			fprintf(stderr, "%s(): exit - terminating\n", __func__);
			break;

		} else {
			fprintf(stderr, "%s(): bad value, terminating\n", __func__);
			exit(EXIT_FAILURE);
		}
	} while (true);

	// cleanup and closing down
	fprintf(stderr, "%s(): closing down\n", __func__);
	ret = munmap(demo_driver_map, uio_size);
	if (0 > ret) {
		perror("munmap() for devuio failed");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}
	close(devuio_fd);
	fprintf(stderr, "%s(): done\n", __func__);
	exit(EXIT_SUCCESS);
}
