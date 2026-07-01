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
    start_speaker();
    esp_now_bridge_start();

    ESP_LOGI(TAG, "formatting SD card...");
    esp_err_t sdr = sd_format("/sdcard");
    if (sdr == ESP_OK) {
        ESP_LOGI(TAG, "SD card ready at /sdcard");
    } else {
        ESP_LOGW(TAG, "SD card init failed (%d)", sdr);
        switch (sdr) {
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_INVALID_STATE:
            request_beep(200, 3); break;
        case ESP_ERR_NOT_FOUND:
            request_beep(400, 2); break;
        case ESP_FAIL:
            request_beep(300, 4); break;
        default:
            request_beep(500, 5); break;
        }
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));
    }
}
