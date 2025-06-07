/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_log.h"

static const char *TAG2 = "example";

typedef struct {
    uint64_t event_count;
} example_queue_element_t;

void TIMER_INIT(gptimer_count_direction_t COUNT_DIR, int TIMER_INT, void *FUNCTION_NAME())
{
	gptimer_handle_t gptimer = NULL;
	QueueHandle_t queue = xQueueCreate(10, sizeof(example_queue_element_t));
	if (!queue) {
		ESP_LOGE(TAG2, "Creating queue failed");
		return;
	}

	gptimer_config_t timer_config = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT,
		.direction = COUNT_DIR,
		.resolution_hz = 1000000, // 1KHz, 1 tick=1ms
	};
	ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

	gptimer_event_callbacks_t cbs = {
		.on_alarm = FUNCTION_NAME,
	};

	ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, queue));

	gptimer_alarm_config_t alarm_config = {
		.reload_count = 0, // counter will reload with 0 on alarm event
		.alarm_count = (1000 * TIMER_INT), // period = 1s @resolution 1MHz
		.flags.auto_reload_on_alarm = true, // enable auto-reload
	};

	ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
	ESP_ERROR_CHECK(gptimer_enable(gptimer));
	ESP_ERROR_CHECK(gptimer_start(gptimer));
}

