/*
 * C6 blink — minimal app_main for ESP32-C6 target.
 * Replaces usb_webcam_main.c when targeting C6 (no camera, no USB-OTG).
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "blink";

/* ponytail: GPIO8 = built-in LED on ESP32-C6-DevKitC-1; change for your PCB */
#define LED_GPIO 8

void app_main(void)
{
    ESP_LOGI(TAG, "C6 blink starting (GPIO%d)", LED_GPIO);

    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
