//#include <stdio.h>
#include <stdint.h>
#include "driver/i2c.h"
#include "../Drivers/gpio.h"
#include "driver/gpio.h"
#include "oled_print.c"
#include "uart_send.c"
#include "adc_read.c"
#include "timer_count.c"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"

#define MAX_NUM_PEOPLE 5
#define MAX_TEMP_ALLOWED 26
#define SW1 18
#define SW2 19
#define RESET_FAN FAN_FUNCTION = false; MODO = false; COOL_ = false; OLED_PRINT("");
#define SET_POINT 26

void DELAY(int ms);
int GET_AMBIENT_TEMPERATURE(double voltaje);
void LIGHTS_SEC(void);
void FAN_CONTROL_ENABLE(void);
void FAN_CONTROL_DISABLE(void);
int BODY_TEMP(double voltaje);
void GPIO_INIT(void);

const int LED_INIT = 5;
const int RED_LED = 16;
const int BLUE_LED = 17;
const int START = 18;
const int FAN = 19;
const int FAN_ENABLE = 27;
const int S_IN = 23;
const int S_OUT = 15;
const int DOOR = 25;
char buffer [100];
int TEM_AMB;
bool SISTEM_INIT = false;
bool FAN_FUNCTION = false;
bool MODO, COOL_ = false;
bool AUTO, ON, COOL, HEAT = false;

bool AMBIENT_TEMP_CONTROL()
{
    BaseType_t high_task_awoken = pdFALSE;

    TEM_AMB = GET_AMBIENT_TEMPERATURE(READ_ADC_CHAN(AMBIENT_TEMPERATURE_CHANEL));
    if((AUTO == true) && (COOL == true) && (TEM_AMB > SET_POINT)) // a.	En Modo AUTO y cool, si TEMPAMB está por encima del set point entonces FAN: On, de lo contrario Off
		WRITE_PIN(FAN_ENABLE,HIGH);
	else if((AUTO == true) && (COOL == true) && (TEM_AMB < SET_POINT))
		WRITE_PIN(FAN_ENABLE,LOW);
	else if((AUTO == true) && (HEAT == true) && (TEM_AMB < SET_POINT)) // b.	En Modo AUTO y heat, si TEMPAMB está por abajo del set point entonces FAN: On de lo contrario Off
		WRITE_PIN(FAN_ENABLE,HIGH);
	else if((AUTO == true) && (HEAT == true) && (TEM_AMB > SET_POINT))
		WRITE_PIN(FAN_ENABLE,LOW);
	else if((ON == true) && (COOL == true)) // c.	Modo On y cool, entonces FAN: On, de lo contrario Off
		WRITE_PIN(FAN_ENABLE,HIGH);
	else if((AUTO != true) || (ON != true))
		WRITE_PIN(FAN_ENABLE,LOW);

    return (high_task_awoken == pdTRUE);
}

void app_main(void)
{
	int PEOPLE_INSIDE = 0;
	int PEOPLE_TEMP;
	UART_INIT();
	OLED_INIT();
	ADC_INIT(ADC_UNIT_1, ADC_BITWIDTH_12, AMBIENT_TEMPERATURE_CHANEL); // AMBIENT TEMPERATURE SENSOR
	ADC_INIT(ADC_UNIT_1, ADC_BITWIDTH_12, PEOPLE_TEMPERATURE_CHANEL); // CORPORAL TEMPERATURE SENSOR
	TIMER_INIT(GPTIMER_COUNT_UP,1000,&AMBIENT_TEMP_CONTROL);
	GPIO_INIT();

	UART_SEND("Sistema: OFF \r\n");
	OLED_PRINT("Sistema: OFF \r\n");

	while(SISTEM_INIT == false){
		if(READ_PIN(START) != 1){
			SISTEM_INIT = true;
			while(READ_PIN(START) != 1);
		}

	}

	AUTO = true;
	HEAT = true;

	WRITE_PIN(LED_INIT,HIGH);
	UART_SEND("Sistema:ON  \r\nDOOR: closed\r\n");
	OLED_PRINT("Sistema:ON  \r\nDOOR: closed\r\n");

	while(SISTEM_INIT == true){
		PEOPLE_TEMP = BODY_TEMP(READ_ADC_CHAN(PEOPLE_TEMPERATURE_CHANEL));
		sprintf(buffer,"Ambient temperature= %d Body temperature= %d ON: %d COOL: %d HEAT: %d AUTO: %d\r\n", TEM_AMB,PEOPLE_TEMP,ON,COOL,HEAT,AUTO);
		UART_SEND(buffer);
		DELAY(100);

		if(READ_PIN(S_IN) != 1){
			while(READ_PIN(S_IN) != 1);
			PEOPLE_TEMP = BODY_TEMP(READ_ADC_CHAN(PEOPLE_TEMPERATURE_CHANEL));
			++PEOPLE_INSIDE;
			if((PEOPLE_INSIDE <= MAX_NUM_PEOPLE) && (PEOPLE_TEMP <= MAX_TEMP_ALLOWED)){
				WRITE_PIN(DOOR,HIGH);
				sprintf(buffer,"NUM PEOPLE = %d\r\nTEMP = %d\r\nDOOR: open \r\n",PEOPLE_INSIDE,PEOPLE_TEMP);
				UART_SEND(buffer);
				OLED_PRINT(buffer);
				DELAY(5000);
				WRITE_PIN(DOOR,LOW);
				OLED_PRINT("");
			}
			if((PEOPLE_INSIDE > MAX_NUM_PEOPLE) && (PEOPLE_TEMP <= MAX_TEMP_ALLOWED)){
				PEOPLE_INSIDE -=1;
				sprintf(buffer,"NUM PEOPLE = %d\r\nTEMP = %d\r\nWe are Full, wait \r\n",PEOPLE_INSIDE,PEOPLE_TEMP);
				UART_SEND(buffer);
				OLED_PRINT(buffer);
				DELAY(5000);
				OLED_PRINT("");
			}
			if(PEOPLE_TEMP > MAX_TEMP_ALLOWED){
				PEOPLE_INSIDE -=1;
				sprintf(buffer,"TEMP OUT OF RANGE\r\nTEMP= %d", PEOPLE_TEMP);
				UART_SEND(buffer);
				OLED_PRINT(buffer);
				LIGHTS_SEC();
				OLED_PRINT("");
			}
		}
		if(READ_PIN(S_OUT) != 1){
			while(READ_PIN(S_OUT) != 1);
			PEOPLE_INSIDE -= 1;
			if(PEOPLE_INSIDE < 0)
				PEOPLE_INSIDE = 0;
			sprintf(buffer,"SOMEONE LEAVING\r\nNUM PEOPLE = %d\r\n", PEOPLE_INSIDE);
			UART_SEND(buffer);
			OLED_PRINT(buffer);
			DELAY(5000);
			OLED_PRINT("");
		}
		if(READ_PIN(FAN) != 1){
			while(READ_PIN(FAN) != 1);
			FAN_FUNCTION = true;
			FAN_CONTROL_ENABLE();
		}

		if(READ_PIN(START) != 1){
			while(READ_PIN(START) != 1);
			FAN_FUNCTION = true;
			FAN_CONTROL_DISABLE();
		}
	}
}

void GPIO_INIT(void){
	SET_PIN_MODE(LED_INIT, OUTPUT_MODE);
	SET_PIN_MODE(RED_LED, OUTPUT_MODE);
	SET_PIN_MODE(BLUE_LED, OUTPUT_MODE);
	SET_PIN_MODE(FAN_ENABLE, OUTPUT_MODE);
	SET_PIN_MODE(DOOR, OUTPUT_MODE);

	WRITE_PIN(LED_INIT,LOW);

	SET_PIN_MODE(START, INPUT_MODE);
	ENABLE_RES(START, PULLUP_RES);

	SET_PIN_MODE(FAN, INPUT_MODE);
	ENABLE_RES(FAN, PULLUP_RES);

	SET_PIN_MODE(S_IN, INPUT_MODE);
	ENABLE_RES(S_IN, PULLUP_RES);

	SET_PIN_MODE(S_OUT, INPUT_MODE);
	ENABLE_RES(S_OUT, PULLUP_RES);
}

int BODY_TEMP(double voltaje){ // each 100mv = 1C°
	int TEMP;
	voltaje *= 1000; // volts to mili volts
	TEMP = (int)(voltaje/100);
	return TEMP;
}

void FAN_CONTROL_DISABLE(void){
	UART_SEND("TEMP_DISABLE\r\nSW1: MODO\r\nSW2: COOL\r\n");
	OLED_PRINT("TEMP_DISABLE\r\nSW1: MODO\r\nSW2: COOL\r\n");
	while(FAN_FUNCTION == true){
		if(READ_PIN(SW1) != 1){
			while(READ_PIN(SW1) != 1);
			MODO = true;
			UART_SEND("MODO SELECTED\r\nSW1: AUTO\r\nSW2: ON\r\n");
			OLED_PRINT("MODO SELECTED\r\nSW1: AUTO\r\nSW2: ON\r\n");
			while(MODO == true){
				if(READ_PIN(SW1) != 1){ // AUTO MODE
					while(READ_PIN(SW1) != 1);
					AUTO = false;
					UART_SEND("AUTO DISABLED\r\n");
					OLED_PRINT("AUTO DISABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
				if(READ_PIN(SW2) != 1){ // ON MODE
					while(READ_PIN(SW2) != 1);
					ON = false;
					UART_SEND("ON DISABLED\r\n");
					OLED_PRINT("ON DISABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
			}
		}
		if(READ_PIN(SW2) != 1){
			while(READ_PIN(SW2) != 1);
			COOL_ = true;
			UART_SEND("COOL SELECTED\r\nSW1: COOL\r\nSW2: HEAT\r\n");
			OLED_PRINT("COOL SELECTED\r\nSW1: COOL\r\nSW2: HEAT\r\n");
			while(COOL_ == true){
				if(READ_PIN(SW1) != 1){ // AUTO MODE
					while(READ_PIN(SW1) != 1);
					COOL = false;
					UART_SEND("COOL DISABLED\r\n");
					OLED_PRINT("COOL DISABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
				if(READ_PIN(SW2) != 1){ // ON MODE
					while(READ_PIN(SW2) != 1);
					HEAT = false;
					UART_SEND("HEAT DISABLED\r\n");
					OLED_PRINT("HEAT DISABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
			}
		}
	}
}

void FAN_CONTROL_ENABLE(void){
	UART_SEND("TEMP_ENABLE\r\nSW1: MODO\r\nSW2: COOL\r\n");
	OLED_PRINT("TEMP_ENABLE\r\nSW1: MODO\r\nSW2: COOL\r\n");
	while(FAN_FUNCTION == true){
		if(READ_PIN(SW1) != 1){
			while(READ_PIN(SW1) != 1);
			MODO = true;
			UART_SEND("MODO SELECTED\r\nSW1: AUTO\r\nSW2: ON\r\n");
			OLED_PRINT("MODO SELECTED\r\nSW1: AUTO\r\nSW2: ON\r\n");
			while(MODO == true){
				if(READ_PIN(SW1) != 1){ // AUTO MODE
					while(READ_PIN(SW1) != 1);
					AUTO = true;
					UART_SEND("AUTO ENABLED\r\n");
					OLED_PRINT("AUTO ENABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
				if(READ_PIN(SW2) != 1){ // ON MODE
					while(READ_PIN(SW2) != 1);
					ON = true;
					UART_SEND("ON ENABLED\r\n");
					OLED_PRINT("ON ENABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
			}
		}
		if(READ_PIN(SW2) != 1){
			while(READ_PIN(SW2) != 1);
			COOL_ = true;
			UART_SEND("COOL SELECTED\r\nSW1: COOL\r\nSW2: HEAT\r\n");
			OLED_PRINT("COOL SELECTED\r\nSW1: COOL\r\nSW2: HEAT\r\n");
			while(COOL_ == true){
				if(READ_PIN(SW1) != 1){ // AUTO MODE
					while(READ_PIN(SW1) != 1);
					COOL = true;
					UART_SEND("COOL ENABLED\r\n");
					OLED_PRINT("COOL ENABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
				if(READ_PIN(SW2) != 1){ // ON MODE
					while(READ_PIN(SW2) != 1);
					HEAT = true;
					UART_SEND("HEAT ENABLED\r\n");
					OLED_PRINT("HEAT ENABLED\r\n");
					DELAY(5000);
					RESET_FAN
				}
			}
		}
	}
}

void LIGHTS_SEC(void){
	for(int i = 0; i < 10; i++){
		WRITE_PIN(RED_LED,HIGH);
		WRITE_PIN(BLUE_LED,LOW);
		DELAY(250);
		WRITE_PIN(RED_LED,LOW);
		WRITE_PIN(BLUE_LED,HIGH);
		DELAY(250);
	}
	WRITE_PIN(RED_LED,LOW);
	WRITE_PIN(BLUE_LED,LOW);
}

int GET_AMBIENT_TEMPERATURE(double voltaje){
	int temperature;
	if(voltaje >= 2.531)
		temperature = 0;
	else if((voltaje < 2.531) && (voltaje >= 2.501))
		temperature = 1;
	else if((voltaje < 2.501) && (voltaje >= 2.469))
			temperature = 2;
	else if((voltaje < 2.469) && (voltaje >= 2.437))
			temperature = 3;
	else if((voltaje < 2.437) && (voltaje >= 2.405))
			temperature = 4;
	else if((voltaje < 2.405) && (voltaje >= 2.372))
			temperature = 5;
	else if((voltaje < 2.372) && (voltaje >= 2.338))
			temperature = 6;
	else if((voltaje < 2.338) && (voltaje >= 2.304))
			temperature = 7;
	else if((voltaje < 2.304) && (voltaje >= 2.27))
			temperature = 8;
	else if((voltaje < 2.27) && (voltaje >= 2.23))
			temperature = 9;
	else if((voltaje < 2.23) && (voltaje >= 2.199))
			temperature = 10;
	else if((voltaje < 2.199) && (voltaje >= 2.164))
			temperature = 11;
	else if((voltaje < 2.164) && (voltaje >= 2.128))
			temperature = 12;
	else if((voltaje < 2.128) && (voltaje >= 2.091))
			temperature = 13;
	else if((voltaje < 2.091) && (voltaje >= 2.055))
			temperature = 14;
	else if((voltaje < 2.055) && (voltaje >= 2.018))
			temperature = 15;
	else if((voltaje < 2.018) && (voltaje >= 1.981))
			temperature = 16;
	else if((voltaje < 1.981) && (voltaje >= 1.944))
			temperature = 17;
	else if((voltaje < 1.944) && (voltaje >= 1.907))
			temperature = 18;
	else if((voltaje < 1.907) && (voltaje >= 1.87))
			temperature = 19;
	else if((voltaje < 1.87) && (voltaje >= 1.833))
			temperature = 20;
	else if((voltaje < 1.833) && (voltaje >= 1.796))
			temperature = 21;
	else if((voltaje < 1.796) && (voltaje >= 1.759))
			temperature = 22;
	else if((voltaje < 1.759) && (voltaje >= 1.722))
			temperature = 23;
	else if((voltaje < 1.722) && (voltaje >= 1.685))
			temperature = 24;
	else if((voltaje < 1.685) && (voltaje >= 1.65))
			temperature = 25;
	else if((voltaje < 1.65) && (voltaje >= 1.612))
			temperature = 26;
	else if((voltaje < 1.612) && (voltaje >= 1.576))
			temperature = 27;
	else if((voltaje < 1.576) && (voltaje >= 1.54))
			temperature = 28;
	else if((voltaje < 1.54) && (voltaje >= 1.505))
			temperature = 29;
	else if((voltaje < 1.505) && (voltaje >= 1.47))
			temperature = 30;
	else if((voltaje < 1.47) && (voltaje >= 1.435))
			temperature = 31;
	else if((voltaje < 1.435) && (voltaje >= 1.4))
			temperature = 32;
	else if((voltaje < 1.4) && (voltaje >= 1.367))
			temperature = 33;
	else if((voltaje < 1.367) && (voltaje >= 1.333))
			temperature = 34;
	else if((voltaje < 1.333) && (voltaje >= 1.3))
			temperature = 35;
	else if((voltaje < 1.3) && (voltaje >= 1.26))
			temperature = 36;
	else if((voltaje < 1.26) && (voltaje >= 1.235))
			temperature = 37;
	else if((voltaje < 1.235) && (voltaje >= 1.204))
			temperature = 38;
	else if((voltaje < 1.204) && (voltaje >= 1.1731))
			temperature = 39;
	else if((voltaje < 1.1731) && (voltaje >= 1.142))
			temperature = 40;
	else if((voltaje < 1.142) && (voltaje >= 1.112))
			temperature = 41;
	else if((voltaje < 1.112) && (voltaje >= 1.083))
			temperature = 42;
	else if((voltaje < 1.083) && (voltaje >= 1.05))
			temperature = 43;
	else if((voltaje < 1.05) && (voltaje >= 1.026))
			temperature = 44;
	else if((voltaje < 1.026) && (voltaje >= 0.9987))
			temperature = 45;
	else temperature = 45;
	return temperature;
}

void DELAY(int ms) {
    TickType_t ticks = pdMS_TO_TICKS(ms);
    vTaskDelay(ticks);
}
