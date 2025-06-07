//#include <stdio.h>
#include <stdint.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <inttypes.h>
#include "../Drivers/gpio.c"
#include "../Drivers/gpio_config.h"
#include "../Drivers/gpio_adresses.h"

#ifndef DRIVERS_GPIO_H_
#define DRIVERS_GPIO_H_

extern void WRITE_PIN(uint8_t pin_num, IS_PIN_HIGH_OR_LOW HL);
extern void SET_PIN_MODE (uint8_t pin_num, gpio_mode mode);
extern void ENABLE_RES(uint8_t pin_num, RESISTOR_MODE mode);
extern int READ_PIN(uint8_t pin_num);

#endif /* DRIVERS_GPIO_H_ */
