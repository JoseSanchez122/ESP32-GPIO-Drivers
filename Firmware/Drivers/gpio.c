//#include <stdio.h>
#include <stdint.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <stdbool.h>
//#include <inttypes.h>
#include "../Drivers/gpio_config.h"
#include "../Drivers/gpio_adresses.h"

/**************************************************************************
* Function: ENABLE_PIN_AS_INPUT
* Preconditions: NONE
* Overview: WRITES 1 AT THE 9TH BIT OF THE SPECIFIED REGISTER TO ENABLE IT AS INPUT.
* Input: PIN NUMBER.
* Output: NONE.
*
*****************************************************************************/

;static void ENABLE_PIN_AS_INPUT(uint8_t pin_num){
	(*(volatile uint32_t*)(IO_MUX_REGISTERS[pin_num])) |= INPUT_ENABLE;
}

/**************************************************************************
* Function: DISABLE_PIN_AS_INPUT
* Preconditions: NONE
* Overview: WRITES 0 AT THE 9TH BIT OF THE SPECIFIED REGISTER TO DISABLE IT AS INPUT.
* Input: PIN NUMBER.
* Output: NONE.
*
*****************************************************************************/
static void DISABLE_PIN_AS_INPUT(uint8_t pin_num){
	(*(volatile uint32_t*)(IO_MUX_REGISTERS[pin_num])) &= INPUT_DISABLE;
}

/**************************************************************************
* Function: ENABLE_PIN_AS_OUTPUT
* Preconditions: NONE
* Overview: WRITES 1 AT THE SPECIFIED BIT OF THE OUTPUT ENABLE REGISTER TO TURN THAT PIN INTO AN OUTPUT PIN
* Input: PIN NUMBER.
* Output: NONE.
*
*****************************************************************************/
static void ENABLE_PIN_AS_OUTPUT(uint8_t pin_num){
	if(pin_num < 32)
		OUTPUT_ENABLE_REG_0_31 |= (1 << pin_num);
	if(pin_num > 31)
		OUTPUT_ENABLE_REG_32_33 |= (1 << (pin_num - 32));
}

/**************************************************************************
* Function: DISABLE_PIN_AS_OUTPUT
* Preconditions: NONE
* Overview: WRITES 0 AT THE SPECIFIED BIT OF THE OUTPUT ENABLE REGISTER TO DISABLE THAT PIN AS AN OUTPUT PIN
* Input: PIN NUMBER.
* Output: NONE.
*
*****************************************************************************/
static void DISABLE_PIN_AS_OUTPUT(uint8_t pin_num){
	if(pin_num < 32)
		OUTPUT_ENABLE_REG_0_31 &= (~(1 << pin_num));
	if(pin_num > 31)
		OUTPUT_ENABLE_REG_32_33 &= (~(1 << (pin_num - 32)));
}

/**************************************************************************
* Function: SET_PIN_MODE.
* Preconditions: NONE.
* Overview: ENABLES A PIN AS INPUT OR OUTPUT DEPENDING ON THE MODE SELECTED
* Input: PIN NUMBER AND MODE.
* Output: NONE.
*
*****************************************************************************/
void SET_PIN_MODE(uint8_t pin_num, gpio_mode mode){

//Note that the Input GPIO pads are 0-19,21-23,25-27,32-39
	if (mode == INPUT_MODE){
		DISABLE_PIN_AS_OUTPUT(pin_num);
		ENABLE_PIN_AS_INPUT(pin_num);
	}

//while the Output GPIO pads are 0-19,21-23,25-27,32-33
	if (mode == OUTPUT_MODE){
		DISABLE_PIN_AS_INPUT(pin_num);
		ENABLE_PIN_AS_OUTPUT(pin_num);
	}
}

/**************************************************************************
* Function: ENABLE_RES.
* Preconditions: THE PIN WHERE WE WANT TO ENABLE THE RESISTOR HAS TO BE PREVIOUSLY ENABLED AS INPUT.
* Overview: ENABLES THE INTERNAL RESISTOR AS PULLUPM OT PULLDOWN
* Input: PIN NUMBER AND RESISTOR TYPE.
* Output: NONE.
*
*****************************************************************************/
void ENABLE_RES(uint8_t pin_num, RESISTOR_MODE mode){
	if (mode == PULLUP_RES){
		(*(volatile uint32_t*)(IO_MUX_REGISTERS[pin_num])) &= PULLUP_DISABLE;
		(*(volatile uint32_t*)(IO_MUX_REGISTERS[pin_num])) |= PULLUP_ENABLE;
	}

	if (mode == PULLDOWN_RES){
		(*(volatile uint32_t*)(IO_MUX_REGISTERS[pin_num])) &= PULLUP_DISABLE;
		(*(volatile uint32_t*)(IO_MUX_REGISTERS[pin_num])) |= PULLDOWN_ENABLE;
	}

}

/**************************************************************************
* Function: READ_PIN.
* Preconditions: THE PIN THAT WE WANT TO READ HAS TO BE PREVIOUSLY ENABLED AS INPUT.
* Overview: READS THE DIGITAL VALUE FROM A PIN.
* Input: PIN NUMBER.
* Output: 1, IF IT HIGH, 0 IF IT IS LOW.
*
*****************************************************************************/
int READ_PIN(uint8_t pin_num){
	int val;
	if(pin_num < 32){
		if(((INPUT_STATUS_REG_0_31 >> pin_num) & 0x1) == 1)
			val = 1;
		else
			val = 0;
	}
	if(pin_num > 31){
		if(((INPUT_STATUS_REG_32_39 >> (pin_num - 32)) & 0x1) == 1)
			val = 1;
		else
			val = 0;
	}
	return val;
}

/**************************************************************************
* Function: WRITE_PIN.
* Preconditions: THE PIN WHERE WE WANT TO WRITE HAS TO BE PREVIOUSLY ENABLED AS OUTPUT.
* Overview: WRITES A DIGITAL VALUE TO A PIN, HIGH OR LOW
* Input: PIN NUMBER, VALUE TO WRITE.
* Output: NONE.
*
*****************************************************************************/
void WRITE_PIN(uint8_t pin_num, IS_PIN_HIGH_OR_LOW HL){
	if(pin_num < 32){
		if(HL == HIGH)
			SET_OUTPUT_PIN_REG_0_31 |= (1 << pin_num);
		if(HL == LOW)
			CLEAR_OUTPUT_PIN_REG_0_31 |= (1 << pin_num);
	}
	if(pin_num > 31){
		if(HL == HIGH)
			SET_OUTPUT_PIN_REG_32_33 |= (1 << (pin_num - 32));
		if(HL == LOW)
			CLEAR_OUTPUT_PIN_REG_32_33 |= (1 << (pin_num - 32));
	}
}

