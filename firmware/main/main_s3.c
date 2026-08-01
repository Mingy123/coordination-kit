/*
 * S3 main — DFRobot DFR1154 (ESP32-S3 + OV3660)
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "speaker.h"

static const char *TAG = "s3_main";

/* forward-declared in esp_now_bridge.c */
void esp_now_bridge_start(void);

void app_main(void)
{
    start_speaker();
    esp_now_bridge_start();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));
    }
}
