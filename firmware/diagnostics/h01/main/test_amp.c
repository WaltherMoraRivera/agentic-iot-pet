#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "bench.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *const TAG = "H01_AMP";

#define SAMPLE_RATE_HZ 16000
// A whole number of samples per period (40 at 16 kHz = 400 Hz), so the
// table repeats without a click.
#define PERIOD_SAMPLES 40
#define TONE_HZ (SAMPLE_RATE_HZ / PERIOD_SAMPLES)
#define PERIODS_PER_WRITE 10

static void write_block(i2s_chan_handle_t tx, const int16_t *block, size_t bytes)
{
    size_t written = 0;
    const esp_err_t err = i2s_channel_write(tx, block, bytes, &written, 1000);
    if (err != ESP_OK || written != bytes) {
        ESP_LOGE(TAG, "I2S write failed: %s (%u of %u bytes)", esp_err_to_name(err),
                 (unsigned)written, (unsigned)bytes);
    }
}

void test_amp_run(void)
{
    const int pins[] = {CONFIG_H01_AMP_BCLK_GPIO, CONFIG_H01_AMP_LRC_GPIO, CONFIG_H01_AMP_DIN_GPIO};
    const char *const labels[] = {"AMP BCLK", "AMP LRC", "AMP DIN"};
    if (!bench_pins_ok(pins, labels, 3)) {
        bench_idle_forever("amplifier pins not usable");
    }

    // I2S1 keeps the amplifier on its own channel, separate from the microphone on I2S0.
    i2s_chan_handle_t tx = NULL;
    i2s_chan_config_t chan = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    chan.auto_clear = true;
    ESP_ERROR_CHECK(i2s_new_channel(&chan, &tx, NULL));

    const i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                        I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_H01_AMP_BCLK_GPIO,
            .ws = CONFIG_H01_AMP_LRC_GPIO,
            .dout = CONFIG_H01_AMP_DIN_GPIO,
            .din = I2S_GPIO_UNUSED,
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx, &std));
    ESP_ERROR_CHECK(i2s_channel_enable(tx));

    // Interleaved stereo frames: the same sample on left and right, so the
    // result does not depend on how the module's SD pin selects the channel.
    static int16_t tone[PERIOD_SAMPLES * PERIODS_PER_WRITE * 2];
    static int16_t silence[PERIOD_SAMPLES * PERIODS_PER_WRITE * 2];
    const float amplitude = 32767.0f * CONFIG_H01_AMP_VOLUME_PCT / 100.0f;
    for (int i = 0; i < PERIOD_SAMPLES * PERIODS_PER_WRITE; ++i) {
        const float phase = 2.0f * (float)M_PI * (float)(i % PERIOD_SAMPLES) / PERIOD_SAMPLES;
        const int16_t s = (int16_t)lrintf(amplitude * sinf(phase));
        tone[2 * i] = s;
        tone[2 * i + 1] = s;
    }
    memset(silence, 0, sizeof(silence));

    // Each block lasts PERIOD_SAMPLES * PERIODS_PER_WRITE / SAMPLE_RATE_HZ = 25 ms.
    const int blocks_per_second = SAMPLE_RATE_HZ / (PERIOD_SAMPLES * PERIODS_PER_WRITE);
    ESP_LOGI(TAG, "Tone %d Hz at %d%% of full scale: 1 s on, 1 s off.", TONE_HZ,
             CONFIG_H01_AMP_VOLUME_PCT);

    for (uint32_t cycle = 0;; ++cycle) {
        ESP_LOGI(TAG, "cycle=%" PRIu32 " tone on", cycle);
        for (int b = 0; b < blocks_per_second; ++b) {
            write_block(tx, tone, sizeof(tone));
        }
        ESP_LOGI(TAG, "cycle=%" PRIu32 " tone off", cycle);
        for (int b = 0; b < blocks_per_second; ++b) {
            write_block(tx, silence, sizeof(silence));
        }
    }
}
