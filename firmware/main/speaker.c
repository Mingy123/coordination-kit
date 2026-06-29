/*
 * Sine wave playback on MAX98357A I2S speaker amp.
 *
 * DFR1154 wiring:
 *   BCLK  → GPIO45  (module pin 51)
 *   LRCLK → GPIO46  (module pin 52)
 *   DIN   → MTMS    (module pin 48) — GPIO42
 *   SD#   → MTDO    (module pin 46) — GPIO40
 *   GAIN  → MTDI    (module pin 47) — GPIO15, leave as-is
 */

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

static const char *TAG = "speaker";

/* cfg */
#define SAMPLE_RATE  48000
#define SINE_FREQ    440        /* A4 */
#define AMPLITUDE    0.35       /* avoid clipping */
#define DURATION_SEC 10
#define SPEAKER_SD   40    /* MTDO */

/* precompute a single period then loop it */
static int16_t *sine_buf;
static size_t   sine_len;       /* in samples */

static esp_err_t init_sine_buf(void)
{
    uint32_t period_samples = SAMPLE_RATE / SINE_FREQ;  /* exact for integer Hz */
    sine_len = period_samples;

    sine_buf = heap_caps_malloc(period_samples * sizeof(int16_t), MALLOC_CAP_DMA);
    if (!sine_buf) return ESP_ERR_NO_MEM;

    for (uint32_t i = 0; i < period_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        sine_buf[i] = (int16_t)(AMPLITUDE * 32767.0 * sin(2.0 * M_PI * SINE_FREQ * t));
    }
    return ESP_OK;
}

static i2s_chan_handle_t tx_handle = NULL;

static esp_err_t init_i2s(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk  = I2S_GPIO_UNUSED,
            .bclk  = GPIO_NUM_45,
            .ws    = GPIO_NUM_46,
            .dout  = GPIO_NUM_42,      /* MTMS */
            .din   = I2S_GPIO_UNUSED,
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));

    return ESP_OK;
}

void speaker_task(void *arg)
{
    /* enable speaker amp */
    gpio_reset_pin(SPEAKER_SD);
    gpio_set_direction(SPEAKER_SD, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKER_SD, 1);   /* SD# HIGH = normal operation */

    if (init_sine_buf() != ESP_OK) {
        ESP_LOGE(TAG, "malloc failed");
        vTaskDelete(NULL);
        return;
    }

    if (init_i2s() != ESP_OK) {
        ESP_LOGE(TAG, "I2S init failed");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "playing %d Hz sine @ %d Hz for %d s", SINE_FREQ, SAMPLE_RATE, DURATION_SEC);

    size_t total = SAMPLE_RATE * DURATION_SEC;
    size_t written = 0;
    size_t bytes;

    while (written < total) {
        /* write one period at a time — repeats seamlessly */
        i2s_channel_write(tx_handle, sine_buf, sine_len * sizeof(int16_t), &bytes, portMAX_DELAY);
        written += sine_len;
    }

    ESP_LOGI(TAG, "done — muting");
    gpio_set_level(SPEAKER_SD, 0);   /* SD# LOW = shutdown */

    i2s_channel_disable(tx_handle);
    i2s_del_channel(tx_handle);

    free(sine_buf);
    vTaskDelete(NULL);
}

void start_speaker(void)
{
    xTaskCreate(speaker_task, "speaker", 4096, NULL, 5, NULL);
}
