/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static int adc_raw[2][10];
int crear_handle = 0;
adc_oneshot_unit_handle_t adc1_handle;

typedef enum {
	AMBIENT_TEMPERATURE_CHANEL = 0, /*!< Decrease count value */
	PEOPLE_TEMPERATURE_CHANEL = 5,   /*!< Increase count value */
} CHANEL_TO_READ;

void ADC_INIT(int ADC_UNIT_1, int ADC_RES, CHANEL_TO_READ ADC_CHANNEL)
{
    //-------------ADC1 Init---------------//
	if(crear_handle == 0){
		crear_handle = 1;
		adc_oneshot_unit_init_cfg_t init_config1 = {
			.unit_id = ADC_UNIT_1,
		};
		ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
	}

	//-------------ADC1 Config---------------//
	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_RES,
		.atten = ADC_ATTEN_DB_11,
	};

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &config));

}

double READ_ADC_CHAN(CHANEL_TO_READ ADC_CHANNEL){
		ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw[0][0]));
		double voltage = (adc_raw[0][0] * 3.3) / 4095;
		return voltage;
}
