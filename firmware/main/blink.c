/*
 * Minimal blink — works on ESP32-S3 and ESP32-C6.
 * Override BLINK_GPIO at build time:
 *   idf.py build -DBLINK_GPIO=48
 *
 * ESP32-C6-DevKitM-1 built-in LED = GPIO8
 * ESP32-S3-DevKitC-1 built-in RGB = GPIO48
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#ifndef BLINK_GPIO
#define BLINK_GPIO 8
#endif

static const char *TAG = "blink";

void app_main(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Blinking GPIO%d", BLINK_GPIO);

    while (1) {
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
