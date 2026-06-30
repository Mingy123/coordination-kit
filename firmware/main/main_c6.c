/*
 * C6 button sender — polls a GPIO, broadcasts edges over ESP-NOW.
 * Replaces main_s3.c when targeting C6 (no camera, no USB-OTG).
 *
 * Packet format (8 bytes):
 *   magic[4]="CKIT" type=0x02 gpio level
 * S3 bridge receives and forwards to PC over USB CDC.
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"

static const char *TAG = "c6";

/* ponytail: GPIO8 = built-in LED, GPIO9 = BOOT button on ESP32-C6-DevKitC-1 */
#define LED_GPIO      8
#define BTN_GPIO      9

/* ponytail: 30 ms debounce — 3 × 10 ms samples */
#define POLL_MS       10
#define DEBOUNCE      3

typedef struct __attribute__((packed)) {
    char     magic[4];  /* "CKIT" */
    uint8_t  type;      /* 0x02 = button event */
    uint8_t  gpio;      /* GPIO number */
    uint8_t  level;     /* 0=released, 1=pressed */
} btn_evt_t;

void app_main(void)
{
    /* NVS (needed by WiFi) */
    esp_err_t r = nvs_flash_init();
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    /* WiFi station mode — required by ESP-NOW */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&(wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT()));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_now_init());

    /* Stay on channel 1 where S3 broadcasts its beacon */
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    /* GPIO setup */
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_GPIO, GPIO_PULLUP_ONLY);  /* ponytail: assumes active-low button to GND */

    /* ponytail: broadcast MAC — no peer pairing needed */
    static const uint8_t bc[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    /* Debounce state */
    int last_stable = gpio_get_level(BTN_GPIO);
    int stable_count = 0;
    int led = 0;

    ESP_LOGI(TAG, "started btn=GPIO%d led=GPIO%d", BTN_GPIO, LED_GPIO);

    while (1) {
        /* LED heartbeat */
        gpio_set_level(LED_GPIO, led);
        led = !led;

        /* Poll button with debounce */
        int cur = gpio_get_level(BTN_GPIO);
        if (cur == last_stable) {
            stable_count = 0;
        } else {
            stable_count++;
            if (stable_count >= DEBOUNCE) {
                /* Edge detected — send event */
                btn_evt_t evt = { .magic = "CKIT", .type = 0x02,
                                  .gpio = BTN_GPIO, .level = cur };
                esp_now_send(bc, (const uint8_t *)&evt, sizeof(evt));
                ESP_LOGI(TAG, "btn=%d gpio=%d", cur, BTN_GPIO);
                last_stable = cur;
                stable_count = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}
