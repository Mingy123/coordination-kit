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

/* ponytail: shared sine fill, reused by speaker_task and beep_error */
static void fill_sine(int16_t *buf, uint32_t n, uint64_t *phase, uint64_t step)
{
    for (uint32_t i = 0; i < n; i++) {
        double rad = (double)((*phase >> 16) & 0xffff) * 2.0 * M_PI / 65536.0;
        buf[i] = (int16_t)(AMPLITUDE * 32767.0 * sin(rad));
        *phase += step;
    }
}

void speaker_task(void *arg)
{
    gpio_reset_pin(SPEAKER_SD);
    gpio_set_direction(SPEAKER_SD, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKER_SD, 1);

    init_i2s();
    ESP_LOGI(TAG, "440 Hz loop");

    int16_t buf[CHUNK_SAMPLES];
    uint64_t phase = 0;
    uint64_t step  = (uint64_t)((double)SINE_FREQ * 0x1p32 / (double)SAMPLE_RATE + 0.5);
    size_t bytes;

    while (1) {
        fill_sine(buf, CHUNK_SAMPLES, &phase, step);
        i2s_channel_write(tx_handle, buf, sizeof(buf), &bytes, portMAX_DELAY);
    }
}

/*
 * Error beep codes — called on boot failure, loops forever.
 *
 *  3 beeps × 200 Hz  — SPI bus init failed (wiring issue)
 *  2 beeps × 400 Hz  — card not detected / init failed
 *  4 beeps × 300 Hz  — format failed (write-protect / bad card)
 *  5 beeps × 500 Hz  — mount failed after format
 */
void beep_error(int freq_hz, int beeps)
{
    /* ponytail: init I2S + prefill zeros before powering amp,
       so MAX98357A sees a settled bus, not power-on garbage */
    init_i2s();
    int16_t zero[CHUNK_SAMPLES] = {0};
    size_t bytes;
    for (int j = 0; j < SAMPLE_RATE * 50 / 1000; j += CHUNK_SAMPLES) {
        i2s_channel_write(tx_handle, zero, sizeof(zero), &bytes, portMAX_DELAY);
    }

    gpio_reset_pin(SPEAKER_SD);
    gpio_set_direction(SPEAKER_SD, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKER_SD, 1);

    uint64_t step = (uint64_t)((double)freq_hz * 0x1p32 / (double)SAMPLE_RATE + 0.5);
    int16_t tone[CHUNK_SAMPLES];

    for (int i = 0; i < beeps; i++) {
        /* ramp up over first 5 ms */
        uint64_t ph = 0;
        uint32_t ramp = SAMPLE_RATE * 5 / 1000;
        for (uint32_t j = 0; j < ramp; j += CHUNK_SAMPLES) {
            uint32_t n = CHUNK_SAMPLES;
            if (j + n > ramp) n = ramp - j;
            fill_sine(tone, n, &ph, step);
            for (uint32_t k = 0; k < n; k++)
                tone[k] = (int16_t)((double)tone[k] * (double)(j + k) / (double)ramp);
            i2s_channel_write(tx_handle, tone, n * 2, &bytes, portMAX_DELAY);
        }

        /* sustained tone */
        for (int j = 0; j < SAMPLE_RATE * 140 / 1000; j += CHUNK_SAMPLES) {
            fill_sine(tone, CHUNK_SAMPLES, &ph, step);
            i2s_channel_write(tx_handle, tone, sizeof(tone), &bytes, portMAX_DELAY);
        }

        /* ramp down over last 5 ms */
        for (uint32_t j = 0; j < ramp; j += CHUNK_SAMPLES) {
            uint32_t n = CHUNK_SAMPLES;
            if (j + n > ramp) n = ramp - j;
            fill_sine(tone, n, &ph, step);
            for (uint32_t k = 0; k < n; k++)
                tone[k] = (int16_t)((double)tone[k] * (double)(ramp - j - k) / (double)ramp);
            i2s_channel_write(tx_handle, tone, n * 2, &bytes, portMAX_DELAY);
        }

        /* silence between beeps */
        for (int j = 0; j < SAMPLE_RATE * 100 / 1000; j += CHUNK_SAMPLES) {
            i2s_channel_write(tx_handle, zero, sizeof(zero), &bytes, portMAX_DELAY);
        }
    }
}

void start_speaker(void)
{
    xTaskCreate(speaker_task, "speaker", 4096, NULL, 5, NULL);
}
