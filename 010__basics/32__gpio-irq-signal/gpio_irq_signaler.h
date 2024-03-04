#ifndef _GPIO_IRQ_SIGNALER__H_
#define _GPIO_IRQ_SIGNALER__H_


#define GPIO_NUM 26
#define GPIO_DEV_NAME "lothars_gpio_dev"

// c language macro string concatenation *trick*
#define GPIO_DEV_PATH "/dev/" GPIO_DEV_NAME

#define REGISTER_UAPP _IO('R', 'g')
#define SIG_NUM 30


#endif /* _GPIO_IRQ_SIGNALER__H_ */
