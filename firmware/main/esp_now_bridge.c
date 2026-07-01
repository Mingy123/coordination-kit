/*
 * ESP-NOW → USB bridge for ESP32-S3.
 *
 * Receives ESP-NOW packets from C6 nodes, forwards them as
 * "MAC HEX data\n" lines over USB-Serial-JTAG console.
 *
 * ponytail: single file, no abstraction, no peer management, no encryption.
 * Upgrade: separate peer table if >20 nodes, binary framing if throughput matters.
 */
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

static const char *TAG = "bridge";

#define ESPNOW_CHAN     1
#define BEACON_MS       3000
#define QLEN            16
#define QITEM           256
#define BCN_MAGIC       "CKIT"
#define BCN_TYPE        0x01
#define LED_GPIO        GPIO_NUM_3

typedef struct __attribute__((packed)) { char m[4]; uint8_t t; } beacon_t;

static QueueHandle_t rxq;

/* Write to USB-Serial-JTAG console */
static void cdc_write(const uint8_t *d, size_t n)
{
    /* ponytail: fwrite to stdout, buffered but fine for low-rate text data */
    fwrite(d, 1, n, stdout);
    fflush(stdout);
}

/* ESP-NOW receive callback */
static void recv_cb(const esp_now_recv_info_t *info, const uint8_t *d, int len)
{
    if (len > QITEM - 8) return;          /* ponytail: drop oversized */
    uint8_t *p = malloc(len + 8);
    if (!p) return;
    memcpy(p, info->src_addr, 6);         /* [0..5] = src MAC */
    p[6] = (len >> 8) & 0xff;             /* [6..7] = payload length */
    p[7] = len & 0xff;
    memcpy(p + 8, d, len);
    if (xQueueSend(rxq, &p, 0) != pdTRUE) free(p);  /* queue full → drop */
}

/* ESP-NOW send callback — logs beacon broadcast status */
static void send_cb(const wifi_tx_info_t *tx, esp_now_send_status_t status)
{
    (void)tx;
    static bool led_on;
    led_on = !led_on;
    gpio_set_level(LED_GPIO, led_on);
    printf("BCN %lu %s\n", (unsigned long)(esp_timer_get_time() / 1000),
           status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
    if (status != ESP_NOW_SEND_SUCCESS) {
        ESP_LOGW(TAG, "beacon send failed");
    }
}

/* Drain queue → CDC */
static void forward_task(void *arg)
{
    (void)arg;
    uint8_t *p;
    char prefix[24];
    while (1) {
        if (xQueueReceive(rxq, &p, portMAX_DELAY) != pdTRUE) continue;
        int dlen = ((int)p[6] << 8) | p[7];
        snprintf(prefix, sizeof(prefix), "%02x:%02x:%02x:%02x:%02x:%02x ",
                 p[0],p[1],p[2],p[3],p[4],p[5]);
        cdc_write((uint8_t*)prefix, strlen(prefix));
        cdc_write(p + 8, dlen);
        cdc_write((uint8_t*)"\n", 1);
        free(p);
    }
}

/* Periodic beacon broadcast */
static void beacon_task(void *arg)
{
    (void)arg;
    const beacon_t bcn = { .m = BCN_MAGIC, .t = BCN_TYPE };
    /* ponytail: broadcast MAC — all peers hear, no pairing needed */
    static const uint8_t bc[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    while (1) {
        esp_now_send(bc, (const uint8_t*)&bcn, sizeof(bcn));
        vTaskDelay(pdMS_TO_TICKS(BEACON_MS));
    }
}

void esp_now_bridge_start(void)
{
    ESP_LOGI(TAG, "starting");

    /* NVS (needed by WiFi) */
    esp_err_t r = nvs_flash_init();
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    /* Status LED */
    gpio_config_t led_cfg = {
        .pin_bit_mask = BIT64(LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = false,
        .pull_up_en = false,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_cfg);
    gpio_set_level(LED_GPIO, 0);

    /* WiFi station mode (no AP) — required by ESP-NOW */
    ESP_LOGI(TAG, "wifi init");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&(wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT()));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "wifi started");

    /* ESP-NOW */
    ESP_LOGI(TAG, "esp-now init");
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(send_cb));
    ESP_LOGI(TAG, "esp-now registered");

    /* Add broadcast peer for beacon sends */
    esp_now_peer_info_t bc_peer = {
        .channel  = ESPNOW_CHAN,
        .ifidx    = ESP_IF_WIFI_STA,
        .encrypt  = false,
    };
    memset(bc_peer.peer_addr, 0xFF, 6);
    ESP_ERROR_CHECK(esp_now_add_peer(&bc_peer));

    ESP_ERROR_CHECK(esp_wifi_set_channel(ESPNOW_CHAN, WIFI_SECOND_CHAN_NONE));

    /* RX queue + tasks */
    ESP_LOGI(TAG, "spawning tasks");
    rxq = xQueueCreate(QLEN, sizeof(uint8_t*));
    xTaskCreate(beacon_task,  "beacon",  2048, NULL, 5, NULL);
    xTaskCreate(forward_task, "fwd",     3072, NULL, 5, NULL);

    ESP_LOGI(TAG, "bridge ready, ch=%d beacon=%dms", ESPNOW_CHAN, BEACON_MS);
}
