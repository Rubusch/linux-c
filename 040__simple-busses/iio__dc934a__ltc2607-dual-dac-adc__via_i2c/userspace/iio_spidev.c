/*
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

int8_t read_adc();

// demo board name
//char demo_board[] = "DC934";   

// global variable - the LTC2422 LSB value with 5V full-scale
float LTC2422_lsb = 4.7683761E-6;

// global constants - set second ltc2422 spi timeout
const uint16_t LTC2422_TIMEOUT = 1000;

// spi clock in Hz
#define SPI_CLOCK_RATE 2000000
#define SPI_DATA_CHANNEL_OFFSET 22
#define SPI_DATA_CHANNEL_MASK (1 << SPI_DATA_CHANNEL_OFFSET)
#define LTC2422_CONVERSION_TIME (137 * 1000)

// MISO timeout in ms
#define MISO_TIMEOUT 1000

int8_t ltc2422_read(uint8_t *adc_channel, int32_t *code, uint16_t timeout);
float ltc2422_voltage(uint32_t adc_code, float ltc2422_lsb);

/* read

  returns the data and channel number
  (0 = channel 0; 1 = channel 1)

  returns the status of the spi read
  0 = successful, 1 = unsuccessful
*/
int8_t
ltc2422_read(uint8_t *adc_channel, int32_t *code, uint16_t timeout)
{
	int fd;
	int ret;
	int32_t value;
	uint8_t buffer[4];

	struct spi_ioc_transfer tr = {
		.tx_buf = 0,                       /* no data to send */
		.rx_buf = (unsigned long) buffer,  /* store received data */
		.delay_usecs = 0,                  /* no delay */
		.speed_hz = SPI_CLOCK_RATE,        /* spi clock speed in Hz */
		.bits_per_word = 8,                /* transaction size */
		.len = 3,                          /* number bytes to transfer */
	};

	// open the device
	fd = open("/dev/spidev0.0", O_RDWR);
	if (0 > fd) {
		perror("open()");
		exit(EXIT_FAILURE);
	}

	// perform the transfer
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (1 > ret) {
		perror("ioctl()");
		close(fd);
		return 1;
	}

	// close the device
	close(fd);

	value = buffer[0] << 16;
	value |= buffer[1] << 8;
	value |= buffer[2];

	// determine the channel number
	*adc_channel = (value & SPI_DATA_CHANNEL_MASK) ? 1 : 0;
	fprintf(stderr, "the value is %x\n", value);

	// return the code
	*code = value;

	return 0;
}

/* voltage

   returns the calculated voltage from the ADC code
*/
float
ltc2422_voltage(uint32_t adc_code, float ltc2422_lsb)
{
	float adc_voltage;

	if (adc_code & 0x200000) {
		adc_code &= 0xfffff;

		// clears bits 20-23
		adc_voltage = ((float) adc_code) * LTC2422_lsb;
	} else {
		adc_code &= 0xfffff;

		// clears bits 20-23
		adc_voltage = -1 * ((float) adc_code) * LTC2422_lsb;
	}

	return adc_voltage;
}

int8_t
read_adc()
{
	float adc_voltage;
	int32_t adc_code;
	uint8_t adc_channel;

	/* array for ADC data

	   useful because you don't know which channel until the
	   ltc2422 tells you
	*/
	int32_t adc_code_array[2];
	int8_t return_code;

	// read adc - throw out the stale data
	ltc2422_read(&adc_channel, &adc_code, LTC2422_TIMEOUT);
	usleep(LTC2422_CONVERSION_TIME);  

	// get current data for both channels
	return_code = ltc2422_read(&adc_channel, &adc_code, LTC2422_TIMEOUT);

	// note that channels may return in any order
	adc_code_array[adc_channel] = adc_code;
	usleep(LTC2422_CONVERSION_TIME);  

	// that is, adc_channel will toggle each reading
	return_code = ltc2422_read(&adc_channel, &adc_code, LTC2422_TIMEOUT);
	adc_code_array[adc_channel] = adc_code;

	// the dc934a board connects VOUTA to CH1
	adc_voltage = ltc2422_voltage(adc_code_array[1], LTC2422_lsb);
	fprintf(stderr, "\tADC A : %6.4f\n", adc_voltage);

	// the dc934a board connects VOUTB to CH0
	adc_voltage = ltc2422_voltage(adc_code_array[0], LTC2422_lsb);
	fprintf(stderr, "\tADC B : %6.4f\n", adc_voltage);

	return return_code;
}

int
main(void)
{
	read_adc();

	puts("READY.");
	exit(EXIT_SUCCESS);
}

