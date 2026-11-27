#include <inttypes.h>
#include <math.h>
#include <stdint.h>

#include "bench.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *const TAG = "H01_MIC";

#define SAMPLE_RATE_HZ 16000
// 200 ms windows: five level lines per second.
#define WINDOW_SAMPLES (SAMPLE_RATE_HZ / 5)
#define CHUNK_SAMPLES 256
#define FULL_SCALE_24 8388608.0f
// INMP441 needs 2^18 SCK cycles to start (256 ms at 1.024 MHz SCK); its
// output is zero until then, so the first 300 ms are discarded.
#define STARTUP_DISCARD_SAMPLES (SAMPLE_RATE_HZ * 300 / 1000)

// Only the level of each window is computed and logged. Samples stay in a
// reused buffer and are never stored or transmitted.
void test_mic_run(void)
{
    const int pins[] = {CONFIG_H01_MIC_BCLK_GPIO, CONFIG_H01_MIC_WS_GPIO, CONFIG_H01_MIC_DIN_GPIO};
    const char *const labels[] = {"MIC SCK", "MIC WS", "MIC SD"};
    if (!bench_pins_ok(pins, labels, 3)) {
        bench_idle_forever("microphone pins not usable");
    }

    i2s_chan_handle_t rx = NULL;
    const i2s_chan_config_t chan = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan, NULL, &rx));

    // INMP441 sends 24-bit data in a 32-bit slot; L/R tied to GND selects the left slot.
    i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,
                                                        I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_H01_MIC_BCLK_GPIO,
            .ws = CONFIG_H01_MIC_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din = CONFIG_H01_MIC_DIN_GPIO,
        },
    };
    std.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx, &std));
    ESP_ERROR_CHECK(i2s_channel_enable(rx));

    ESP_LOGI(TAG, "Levels are relative to digital full scale (0..1), not dB SPL.");
    static int32_t chunk[CHUNK_SAMPLES];
    uint32_t window = 0;

    int discarded = 0;
    while (discarded < STARTUP_DISCARD_SAMPLES) {
        size_t bytes = 0;
        const esp_err_t err = i2s_channel_read(rx, chunk, sizeof(chunk), &bytes, 1000);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S read failed: %s", esp_err_to_name(err));
            continue;
        }
        discarded += (int)(bytes / sizeof(chunk[0]));
    }
    ESP_LOGI(TAG, "Discarded %d startup samples (300 ms).", discarded);

    for (;;) {
        double sum = 0.0;
        double sum_sq = 0.0;
        int32_t peak = 0;
        int zeros = 0;
        int taken = 0;

        while (taken < WINDOW_SAMPLES) {
            size_t bytes = 0;
            const esp_err_t err = i2s_channel_read(rx, chunk, sizeof(chunk), &bytes, 1000);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "I2S read failed: %s", esp_err_to_name(err));
                continue;
            }
            const int n = (int)(bytes / sizeof(chunk[0]));
            for (int i = 0; i < n && taken < WINDOW_SAMPLES; ++i, ++taken) {
                const int32_t s = chunk[i] >> 8;
                sum += s;
                sum_sq += (double)s * s;
                const int32_t mag = s < 0 ? -s : s;
                if (mag > peak) {
                    peak = mag;
                }
                if (chunk[i] == 0) {
                    ++zeros;
                }
            }
        }

        // Remove the DC offset before computing RMS.
        const double mean = sum / taken;
        double variance = sum_sq / taken - mean * mean;
        if (variance < 0.0) {
            variance = 0.0;
        }
        const float rms = (float)sqrt(variance) / FULL_SCALE_24;
        const float pk = (float)peak / FULL_SCALE_24;
        // Signed offset on the same full-scale range (-1..1).
        const float dc = (float)mean / FULL_SCALE_24;

        ESP_LOGI(TAG, "window=%" PRIu32 " rms=%.5f peak=%.5f dc=%+.5f zeros=%d/%d", window, rms,
                 pk, dc, zeros, taken);
        if (zeros == taken) {
            ESP_LOGW(TAG, "All samples are zero: check SD wiring, L/R to GND and 3.3 V supply.");
        }
        ++window;
    }
}
