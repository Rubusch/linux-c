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

void read_adc();

// the LTC2422 LSB value with 5V full-scale
float LTC2422_lsb = 4.7683761E-6;
// the LTC2422 LSB value with 3.3V full-scale
//float LTC2422_lsb = 3.1471252E-6;

// check which number is the ADC iio:deviceX and replace 'X' by the
// number
#define LTC2422_FILE_VOLTAGE "/sys/bus/iio/devices/iio:device2/out_voltage0_raw"
#define SPI_DATA_CHANNEL_OFFSET 22
#define SPI_DATA_CHANNEL_MASK (1 << SPI_DATA_CHANNEL_OFFSET)
#define LTC2422_CONVERSION_TIME (137 * 1000)  /* ms */

int8_t ltc2422_read(uint8_t *adc_channel, int32_t *code);
float ltc2422_voltage(uint32_t adc_code, float ltc2422_lsb);
                  
/* read

  returns the data and channel number
  (0 = channel 0; 1 = channel 1)

  returns the status of the spi read
  0 = successful, 1 = unsuccessful
*/
int8_t
ltc2422_read(uint8_t *adc_channel, int32_t *code)
{
	int a2d_reading = 0;
	FILE *file;
	int read;

	file = fopen(LTC2422_FILE_VOLTAGE, "r");
	if (!file) {
		perror("fopen()");
		return -1;
	}

	read = fscanf(file, "%d", &a2d_reading);
	if (0 >= read) {
		fprintf(stderr, "FAILED: unable to read values from voltage input\n");
		exit(EXIT_FAILURE);
	}

	// determine the channel number
	*adc_channel = (a2d_reading & SPI_DATA_CHANNEL_MASK) ? 1 : 0;
	*code = a2d_reading;

	fclose(file);

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

void
read_adc()
{
	float adc_voltage;
	int32_t adc_code;
	uint8_t adc_channel;

	ltc2422_read(&adc_channel, &adc_code);
	usleep(LTC2422_CONVERSION_TIME);

	ltc2422_read(&adc_channel, &adc_code);
	adc_voltage = ltc2422_voltage(adc_code, LTC2422_lsb);
	fprintf(stderr, "the value of the ADC channel %d\n", adc_channel);
	fprintf(stderr, "\tis : %6.4f\n", adc_voltage);
	usleep(LTC2422_CONVERSION_TIME);

	ltc2422_read(&adc_channel, &adc_code);
	adc_voltage = ltc2422_voltage(adc_code, LTC2422_lsb);
	fprintf(stderr, "the value of the ADC channel %d\n", adc_channel);
	fprintf(stderr, "\tis : %6.4f\n", adc_voltage);
}

int
main(void)
{
	read_adc();
	puts("READY.");
	exit(EXIT_SUCCESS);
}
