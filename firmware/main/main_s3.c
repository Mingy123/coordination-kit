/*
 * S3 main — DFRobot DFR1154 (ESP32-S3 + OV3660)
 *
 * Formats SD card, then launches ESP-NOW bridge.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "speaker.h"
#include "sd_card.h"

static const char *TAG = "s3_main";

/* forward-declared in esp_now_bridge.c */
void esp_now_bridge_start(void);

void app_main(void)
{
    esp_now_bridge_start();

    ESP_LOGI(TAG, "formatting SD card...");
    esp_err_t sdr = sd_format("/sdcard");
    if (sdr == ESP_OK) {
        ESP_LOGI(TAG, "SD card ready at /sdcard");
    } else {
        ESP_LOGW(TAG, "SD card init failed (%d)", sdr);
        /* ponytail: beep-error table from speaker.c */
        switch (sdr) {
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_INVALID_STATE:
            beep_error(200, 3); break;  // bus / wiring
        case ESP_ERR_NOT_FOUND:
            beep_error(400, 2); break;  // no card
        case ESP_FAIL:
            beep_error(300, 4); break;  // format fail
        default:
            beep_error(500, 5); break;  // mount / unknown
        }
    }

    esp_now_bridge_start();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));
    }
}
