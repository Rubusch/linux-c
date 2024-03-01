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
  -p: watch the line


  compilation:
  NB: I compiled it natively on an arm64 platform, if not libgpiod-dev
  needs to be installed as 'arm64' package for the cross toolchain
  $ sudo dpkg --add-architecture arm64
  $ sudo apt install -y libgpiod2 libgpiod-dev:arm64

  then link against `-lgpiod`

  REFERENCE
   - https://github.com/brgl/libgpiod
   - https://sergioprado.blog/new-linux-kernel-gpio-user-space-interface/
   - https://www.acmesystems.it/gpiod
   - https://blog.lxsang.me/post/id/33
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>

// NB: for cpp there is a <gpiod.hpp>
#include <gpiod.h>

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
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	size_t nlines;

	chip = gpiod_chip_open(dev); // e.g. "/dev/gpiochip0"
	if (!chip) {
		fprintf(stderr, "%s(): failed to open %s: %s\n",
			__func__, dev, strerror(errno));
		exit(EXIT_FAILURE);
	}

	const char *name = gpiod_chip_name(chip);
	if (!name)
		name = "unnamed";
	printf("%s(): chip name: %s\n", __func__, name);

	const char *label = gpiod_chip_label(chip);
	if (!label)
		name = "unlabeled";
	printf("%s(): pinctrl in DT: %s\n", __func__, label);

	nlines = gpiod_chip_num_lines(chip);
	printf("%s(): number of lines: %ld\n", __func__, nlines);

 	for (int idx = 0; idx < nlines; idx++) {
		line = gpiod_chip_get_line(chip, idx);
		if (!line) {
			fprintf(stderr, "%s(): failed to get line handle: %s\n",
				__func__, strerror(errno));
			gpiod_chip_close(chip);
			exit(EXIT_FAILURE);
		}

		const char *line_name = gpiod_line_name(line);
 		if (!line_name)
 			line_name = "unnamed";

		const char *consumer = gpiod_line_consumer(line);
 		if (!consumer)
 			consumer = "N/A";

 		printf("  %s, %d [line_offset]:\t%s,%s\tconsumer: %s,%s\tflags: %s%s%s%s"
 		       "\n",
 		       name,
 		       idx,
 		       line_name,
 		       (strlen(line_name) < 7) ? "\t" : "", /* spacer */
 		       consumer,
 		       (strlen(consumer) < 7) ? "\t" : "", /* spacer */
 		       (GPIOD_LINE_DIRECTION_INPUT == gpiod_line_direction(line)) ? "input" : "output",
 		       (GPIOD_LINE_ACTIVE_STATE_LOW == gpiod_line_active_state(line)) ? " | active_low" : " | active_high",
 		       (gpiod_line_is_open_drain(line)) ? " | open_drain" : "",
 		       (gpiod_line_is_open_source(line)) ? " | open_source" : ""
 			);
 	}

	gpiod_chip_close(chip);
}

void do_read(const char *dev, int offset)
{
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	int ret, value;

	chip = gpiod_chip_open(dev); // e.g. "/dev/gpiochip0"
	if (!chip) {
		fprintf(stderr, "%s(): failed to open %s: %s\n",
			__func__, dev, strerror(errno));
		exit(EXIT_FAILURE);
	}

	line = gpiod_chip_get_line(chip, offset);
	if (!line) {
		fprintf(stderr, "%s(): failed to get line handle from ioctl: %s\n",
		       __func__, strerror(errno));
		gpiod_chip_close(chip);
		exit(EXIT_FAILURE);
	}

	ret = gpiod_line_request_input(line, "gpio_state");
	if (ret) {
		fprintf(stderr, "%s(): failed to get line values using ioctl: %s\n",
			__func__, strerror(errno));
		gpiod_chip_close(chip);
		exit(EXIT_FAILURE);
	}

	value = gpiod_line_get_value(line);

	printf("%s(): gpio at offset %d (input) on chip %s: %d\n",
	       __func__, offset, dev, value);

	gpiod_chip_close(chip);
}

void do_write(const char *dev, int offset, uint8_t value)
{
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	int ret;

	chip = gpiod_chip_open(dev); // e.g. "/dev/gpiochip0"
	if (!chip) {
		fprintf(stderr, "%s(): failed to open %s: %s\n",
			__func__, dev, strerror(errno));
		exit(EXIT_FAILURE);
	}

	line = gpiod_chip_get_line(chip, offset);
	if (!line) {
		fprintf(stderr, "%s(): failed to get line handle from ioctl: %s\n",
		       __func__, strerror(errno));
		gpiod_chip_close(chip);
		exit(EXIT_FAILURE);
	}

	ret = gpiod_line_request_output(line, "gpio_state", -1);
	if (0 > ret) {
		fprintf(stderr, "%s(): failed to get line values using ioctl: %s\n",
			__func__, strerror(errno));
		gpiod_chip_close(chip);
		exit(EXIT_FAILURE);
	}

	gpiod_line_set_value(line, value);
	printf("%s(): gpio at offset %d (input) on chip %s: %d\n",
	       __func__, offset, dev, value);

	gpiod_chip_close(chip);

	// re-read if the setting worked...
	do_read(dev, offset);
}

/*
  the following approach will be possible if current trunk version of
  libgpiod is released on the system (currently won't compile)
 */
void do_poll(const char *dev, int offset)
{
//*
        puts("TODO: not implemented (still)");
/*/
	struct gpiod_info_event *event;
	struct gpiod_line_info *info;
	struct gpiod_chip *chip;
	uint64_t timestamp_ns;

	chip = gpiod_chip_open(dev);
	if (!chip) {
		fprintf(stderr, "failed to open chip: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	info = gpiod_chip_watch_line_info(chip, offset);
	if (!info) {
		fprintf(stderr, "failed to read info: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (true) {
		// blocks until an event is available
		event = gpiod_chip_read_info_event(chip);
		if (!event) {
			fprintf(stderr, "failed to read event: %s\n",
				strerror(errno));
			exit(EXIT_FAILURE);
		}

		info = gpiod_info_event_get_line_info(event);
		timestamp_ns = gpiod_info_event_get_timestamp_ns(event);
		printf("line %3d: %-9s %" PRIu64 ".%" PRIu64 "\n",
		       gpiod_line_info_get_offset(info), event_type(event),
		       timestamp_ns / 1000000000, timestamp_ns % 1000000000);

		gpiod_info_event_free(event);
	}

	gpiod_chip_close(chip);
// */
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

