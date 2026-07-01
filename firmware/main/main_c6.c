/*
 * C6 button sender — polls a GPIO, sends edges over ESP-NOW.
 * Pairs with S3 bridge via beacon discovery, sends unicast with ACK.
 *
 * Packet format (8 bytes):
 *   magic[4]="CKIT" type=0x02 gpio level
 *
 * S3 bridge receives and forwards to PC over USB CDC.
 * On successful send, LED blinks 200 ms.
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"

static const char *TAG = "c6";

/* ponytail: GPIO8 = built-in LED, GPIO9 = BOOT button on ESP32-C6-DevKitC-1 */
#define LED_GPIO      8
#define BTN_GPIO      9

/* ponytail: 30 ms debounce — 3 x 10 ms samples */
#define POLL_MS       10
#define DEBOUNCE      3

/* Beacon header (same layout as S3 bridge) */
typedef struct __attribute__((packed)) {
    char     magic[4];  /* "CKIT" */
    uint8_t  type;      /* 0x01 = beacon */
} ckit_beacon_t;

typedef struct __attribute__((packed)) {
    char     magic[4];  /* "CKIT" */
    uint8_t  type;      /* 0x02 = button event */
    uint8_t  gpio;      /* GPIO number */
    uint8_t  level;     /* 0=released, 1=pressed */
} btn_evt_t;

/* Pairing state — learned from S3 beacon */
static uint8_t s3_addr[6];
static bool    s3_known;

/* LED blink state */
static volatile bool     tx_acked;
static volatile TickType_t led_off_tick;

/* ------------------------------------------------------------------ */
/* ESP-NOW receive callback — discovers and pairs with S3 from beacon */
/* ------------------------------------------------------------------ */
static void recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    if (len < (int)sizeof(ckit_beacon_t)) return;

    const ckit_beacon_t *bcn = (const ckit_beacon_t *)data;
    if (memcmp(bcn->magic, "CKIT", 4) != 0) return;
    if (bcn->type != 0x01) return;                     /* only beacons */

    /* Already paired with this MAC — nothing to do */
    if (s3_known && memcmp(s3_addr, info->src_addr, 6) == 0) return;

    /* Replace old peer entry with new S3 */
    if (s3_known) esp_now_del_peer(s3_addr);
    memcpy(s3_addr, info->src_addr, 6);

    esp_now_peer_info_t peer = {
        .channel  = 1,
        .ifidx    = ESP_IF_WIFI_STA,
        .encrypt  = false,
    };
    memcpy(peer.peer_addr, s3_addr, 6);

    esp_err_t r = esp_now_add_peer(&peer);
    if (r == ESP_OK) {
        s3_known = true;
        ESP_LOGI(TAG, "paired with " MACSTR, MAC2STR(s3_addr));
    } else {
        ESP_LOGE(TAG, "add_peer failed %d", r);
    }
}

/* ---------------------------------------------------------------- */
/* ESP-NOW send callback — blink LED on successful unicast delivery */
/* ---------------------------------------------------------------- */
static void send_cb(const wifi_tx_info_t *tx, esp_now_send_status_t status)
{
    (void)tx;
    if (status == ESP_NOW_SEND_SUCCESS) {
        tx_acked = true;
    }
}

/* ------------------------------------------------------------------ */
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

    /* Register callbacks */
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(send_cb));

    /* GPIO setup */
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_level(LED_GPIO, 0);

    /* Debounce state */
    int last_stable = gpio_get_level(BTN_GPIO);
    int stable_count = 0;

    ESP_LOGI(TAG, "started btn=GPIO%d led=GPIO%d", BTN_GPIO, LED_GPIO);

    while (1) {
        /* ---- LED management ---- */
        if (tx_acked) {
            gpio_set_level(LED_GPIO, 1);
            led_off_tick = xTaskGetTickCount() + pdMS_TO_TICKS(200);
            tx_acked = false;
        }
        if (led_off_tick && xTaskGetTickCount() >= led_off_tick) {
            gpio_set_level(LED_GPIO, 0);
            led_off_tick = 0;
        }

        /* ---- Button poll with debounce ---- */
        int cur = gpio_get_level(BTN_GPIO);
        if (cur == last_stable) {
            stable_count = 0;
        } else {
            stable_count++;
            if (stable_count >= DEBOUNCE) {
                /* Edge detected — send event via unicast to S3 */
                if (s3_known) {
                    btn_evt_t evt = { .magic = "CKIT", .type = 0x02,
                                      .gpio = BTN_GPIO, .level = cur };
                    esp_err_t sr = esp_now_send(s3_addr, (const uint8_t *)&evt, sizeof(evt));
                    if (sr != ESP_OK) {
                        ESP_LOGW(TAG, "esp_now_send failed %d", sr);
                    }
                }
                ESP_LOGI(TAG, "btn=%d gpio=%d%s", cur, BTN_GPIO,
                         s3_known ? "" : " (no peer)");
                last_stable = cur;
                stable_count = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}
