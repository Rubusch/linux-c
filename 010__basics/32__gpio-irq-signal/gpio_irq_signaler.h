#ifndef _GPIO_IRQ_SIGNALER__H_
#define _GPIO_IRQ_SIGNALER__H_

#define GPIO_NUM (26 + 512) /* Don't do this!!! This is old gpio
			     * lib stuff! 512 in the kernel is defined
			     * as GPIO_DYNAMIC_BASE, but instead of
			     * setting up a gpionumber in modern gpio
			     * you need to setup a gpio_descriptor. A
			     * gpio_descriptor then is defined by a
			     * gpiochip and an offset e.g. here
			     * 26. For the sake of a basic old demo,
			     * this still uses the old API here
			     */
#define GPIO_DEV_NAME "lothars_gpio_dev"

// c language macro string concatenation *trick*
#define GPIO_DEV_PATH "/dev/" GPIO_DEV_NAME

#define REGISTER_UAPP _IO('R', 'g')
#define SIG_NUM (30)


#endif /* _GPIO_IRQ_SIGNALER__H_ */
