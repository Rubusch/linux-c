/*
  This is my implementation/adaptation of the outstanding tool of
  "Dany Le" for my own learning purposes. Pls, have a look at his
  blog: https://iohub.dev/

  My libgpiod version of the tool. Note, when verifying via gpiod and
  modern gpio interface - gpioset only keeps the setting as long as
  the PID is around. If the PID exits, the gpio setting returns to
  default. In cases where this is not the case, the behavior is
  undefined.
  The trick then is something like (use mode wait, time or signal)
  $ gpioset -m signal gpiochip0 26=1
  ...and stop it with CTRL+c

  Note: another process is not able to check the gpio state now, the
  resource will be busy.

  usage:
  $ ./gpio-demo.elf -l /dev/gpiochip0
  $ ./gpio-demo.elf -r 26 /dev/gpiochip0
  $ ./gpio-demo.elf -w 26 1 /dev/gpiochip0
  $ ./gpio-demo.elf -p 26 /dev/gpiochip0

  in modern systems using libgpiod, use the following tools
  -l: gpioinfo
  -r: gpioget
  -w: gpioset
  -p: polling

  REFERENCE
   - https://www.acmesystems.it/gpiod
   - https://blog.lxsang.me/post/id/33
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <linux/gpio.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>

typedef enum {
	APP_OPT_LIST,
	APP_OPT_READ,
	APP_OPT_WRITE,
	APP_OPT_POLL,
	APP_OPT_UNKNOWN
} opts_e;

typedef struct {
	char *dev;
	int offset;
	uint8_t val;
	opts_e mode;
} opts_t;

void usage(const char *app)
{
	fprintf(stderr,
		"Usage:\n\t$ %s <options> <device node>\n"
		"e.g.\n\t$ %s -l /dev/gpiochip0\n"
		"e.g.\n\t$ %s -r 26 /dev/gpiochip0\n"
		"e.g.\n\t$ %s -w 26 1 /dev/gpiochip0\n"
		"options:\n"
		"\t -l: gpioinfo for chip\n"
		"\t -r <offset>: read gpio at offset (input)\n"
		"\t -w <val> <offset>: write gpio at offset (output)\n"
		"\t -p <offset>: polling raising event on the gpio at offset\n",
		app, app, app, app);
}

int gpio_open(const char* dev)
{
	int fd = open(dev, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "%s(): failed to open %s: %s\n",
			__func__, dev, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return fd;
}

void do_list(const char *dev)
{
	struct gpiochip_info info;
	struct gpioline_info line_info;
	int fd, ret;

	// open device node, e.g. /dev/gpiochip0
	fd = gpio_open(dev);

	ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
	if (ret == -1) {
		fprintf(stderr, "%s(): failed to get chip info from ioctl: %s\n",
			__func__, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}

	printf("%s(): chip name: %s\n", __func__, info.name);
	printf("%s(): pinctrl in DT: %s\n", __func__, info.label);
	printf("%s(): number of lines: %d\n", __func__, info.lines);

	for (int idx = 0; idx < info.lines; idx++) {
		line_info.line_offset = idx;

		ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &line_info);
		if (ret == -1) {
			fprintf(stderr, "%s(): failed to get chip info from ioctl: %s\n",
				__func__, strerror(errno));
			close(fd);
			exit(EXIT_FAILURE);
		}
		printf("  %s, %d [line_offset]:\t%s,%s\tconsumer: %s,%s\tflags: %s%s%s%s%s"
		       "\n",
		       info.name,
		       idx,
		       line_info.name,
		       (strlen(line_info.name) < 7) ? "\t" : "", /* spacer */
		       (strlen(line_info.consumer) == 0) ? "N/A" : line_info.consumer,
		       (strlen(line_info.consumer) < 7) ? "\t" : "", /* spacer */
		       (line_info.flags & GPIOLINE_FLAG_IS_OUT) ? "OUT" : "IN",
		       (line_info.flags & GPIOLINE_FLAG_ACTIVE_LOW) ? " | ACTIVE_LOW" : " | ACTIVE_HIGH",
		       (line_info.flags & GPIOLINE_FLAG_OPEN_DRAIN) ? " | OPEN_DRAIN" : "",
		       (line_info.flags & GPIOLINE_FLAG_OPEN_SOURCE) ? " | OPEN_SOURCE" : "",
		       (line_info.flags & GPIOLINE_FLAG_KERNEL) ? " | KERNEL" : "" );

// further (of the 8 available) flags:
//		       GPIOLINE_FLAG_BIAS_PULL_UP
//		       GPIOLINE_FLAG_BIAS_PULL_DOWN
//		       GPIOLINE_FLAG_BIAS_DISABLE

	}

	close(fd);
}

void do_read(const char *dev, int offset)
{
	struct gpiohandle_request rq;
	struct gpiohandle_data data;
	int fd, ret;

	printf("%s(): read gpio at line %d on chip %s\n", __func__, offset, dev);
	fd = gpio_open(dev);

	// init request
	rq.lineoffsets[0] = offset;
	rq.flags = GPIOHANDLE_REQUEST_INPUT;
	rq.lines = 1;

	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
	if (-1 == ret) {
		fprintf(stderr, "%s(): failed to get line handle from ioctl: %s\n",
		       __func__, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

	ret = ioctl(rq.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (-1 == ret) {
		fprintf(stderr, "%s(): failed to get line values using ioctl: %s\n",
			__func__, strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("%s(): gpio at offset %d (input) on chip %s: %d\n",
	       __func__, offset, dev, data.values[0]);

	close(rq.fd);
}

void do_write(const char *dev, int offset, uint8_t value)
{
	struct gpiohandle_request rq;
	struct gpiohandle_data data;
	int fd, ret;

	printf("%s(): write %d to gpio at line %d on chip %s\n",
	       __func__, value, offset, dev);

	fd = gpio_open(dev);

	rq.lineoffsets[0] = offset;
	rq.flags = GPIOHANDLE_REQUEST_OUTPUT;
	rq.lines = 1;
	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
	if (-1 == ret) {
		close(fd);
		fprintf(stderr, "%s(): failed to get line handle: %s\n",
			__func__, strerror(errno));
		exit(EXIT_FAILURE);
	}
	close(fd);

	data.values[0] = value;
	ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
	if (-1 == ret) {
		fprintf(stderr, "%s(): failed to set line value: %s\n",
			__func__, strerror(errno));
	}
	sleep(1);
	close(rq.fd); // has to be closed, if not the device will be
		      // "busy" later on (for a different descriptor)

	// re-read if the setting worked...
	do_read(dev, offset);
}

void do_poll(const char *dev, int offset)
{
	struct gpioevent_request rq;
	struct pollfd pfd;
	int fd, ret;

	fd = gpio_open(dev);

	rq.lineoffset = offset;
	rq.eventflags = GPIOEVENT_EVENT_RISING_EDGE;
	ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &rq);
	if (-1 == ret) {
		fprintf(stderr, "%s(): failed to get line event from ioctl : %s\n",
			__func__, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

	pfd.fd = rq.fd;
	pfd.events = POLLIN;
	ret = poll(&pfd, 1, -1);
	if (-1 == ret) {
		fprintf(stderr, "%s(): failed while polling on gpio: %s\n",
			__func__, strerror(errno));
	} else if (pfd.revents & POLLIN) {
		printf("%s(): rising edge event on gpio offset: %d, of %s\n",
		       __func__, offset, dev);
	}
	close(rq.fd);
}

int main(int argc, char *argv[])
{
	int ret;
	opts_t opt;

	opt.val = 0;
	if (argc == 5) {
		// e.g.: %s -w 26 1 /dev/gpiochip0
		opt.val = (uint8_t) atoi(argv[3]);
	}
	opt.dev = NULL;
	opt.mode = APP_OPT_UNKNOWN;
	while ((ret = getopt(argc, argv, "lr:w:p:")) != -1) {
		switch (ret) {
		case 'p':
			opt.mode = APP_OPT_POLL;
			opt.offset = atoi(optarg);
			break;

		case 'w':
			opt.mode = APP_OPT_WRITE;
			opt.offset = atoi(optarg);
			break;
		case 'r':
			opt.mode = APP_OPT_READ;
			opt.offset = atoi(optarg);
			break;
		case 'l':
			opt.mode = APP_OPT_LIST;
			break;
		default:
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (optind >= argc || opt.mode == APP_OPT_UNKNOWN) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	opt.dev = argv[argc-1];
	switch (opt.mode) {
	case APP_OPT_POLL:
		do_poll(opt.dev, opt.offset);
		break;
	case APP_OPT_WRITE:
		do_write(opt.dev, opt.offset, opt.val);
		break;
	case APP_OPT_READ:
		do_read(opt.dev, opt.offset);
		break;
	case APP_OPT_LIST:
		do_list(opt.dev);
		break;
	default:
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("%s(): READY.\n", __func__);
	exit(EXIT_SUCCESS);
}

