/*
 * gpio_config.h
 *
 *  Created on: 3 dic. 2023
 *      Author: lupis
 */
//#include <stdio.h>
#include <stdint.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <inttypes.h>

#ifndef DRIVERS_GPIO_CONFIG_H_
#define DRIVERS_GPIO_CONFIG_H_

#define PULLDOWN_ENABLE   (1UL << (7))
#define PULLDOWN_DISABLE  (~(1UL << (7)))
#define PULLUP_ENABLE     (1UL << (8))
#define PULLUP_DISABLE    (~(1UL << (8)))
#define INPUT_ENABLE      (1UL << (9))
#define INPUT_DISABLE     (~(1UL << (9)))

typedef enum{
	LOW = 0x00000001,
	HIGH = 0x00000002,
}IS_PIN_HIGH_OR_LOW;

typedef enum {
	INPUT_MODE 	= 1,
	OUTPUT_MODE = 2,
}gpio_mode;

typedef enum {
	PULLUP_RES 	= 1,
	PULLDOWN_RES = 2,
}RESISTOR_MODE;

#endif /* DRIVERS_GPIO_CONFIG_H_ */
