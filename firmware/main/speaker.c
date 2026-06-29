/*
 * Sine wave playback on MAX98357A I2S speaker amp.
 *
 * DFR1154 wiring:
 *   BCLK  → GPIO45  (module pin 51)
 *   LRCLK → GPIO46  (module pin 52)
 *   DIN   → MTMS    (module pin 48) — GPIO42
 *   SD#   → MTDO    (module pin 46) — GPIO40
 *   GAIN  → MTDI    (module pin 47) — GPIO41, leave as-is
 */

#include <math.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

static const char *TAG = "speaker";

/* cfg */
#define SAMPLE_RATE  48000
#define SINE_FREQ    440        /* A4 */
#define AMPLITUDE    0.35       /* avoid clipping */
#define SPEAKER_SD   40         /* MTDO */
#define CHUNK_SAMPLES 256

/* Q32.32 phase accumulator — no wrap click */
#define PHASE_INC ((uint64_t)((double)SINE_FREQ * 0x1p32 / (double)SAMPLE_RATE + 0.5))

static i2s_chan_handle_t tx_handle;

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
    gpio_reset_pin(SPEAKER_SD);
    gpio_set_direction(SPEAKER_SD, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKER_SD, 1);

    if (init_i2s() != ESP_OK) {
        ESP_LOGE(TAG, "I2S init failed");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "playing %d Hz sine @ %d Hz", SINE_FREQ, SAMPLE_RATE);

    int16_t buf[CHUNK_SAMPLES];
    uint64_t phase = 0;
    size_t bytes;

    while (1) {
        for (int i = 0; i < CHUNK_SAMPLES; i++) {
            /* Q32.32 → fixed-point sin approximation */
            uint32_t p = (phase >> 16) & 0xffff;       /* Q16.16 */
            double rad = (double)p * 2.0 * M_PI / 65536.0;
            buf[i] = (int16_t)(AMPLITUDE * 32767.0 * sin(rad));
            phase += PHASE_INC;
        }
        i2s_channel_write(tx_handle, buf, sizeof(buf), &bytes, portMAX_DELAY);
    }
}

void start_speaker(void)
{
    xTaskCreate(speaker_task, "speaker", 4096, NULL, 5, NULL);
}
