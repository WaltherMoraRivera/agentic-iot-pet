#include "sensors.h"

#include <math.h>

#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *const TAG = "PET_SENSORS";

#define MIC_SAMPLE_RATE_HZ 16000
#define MIC_WINDOW_SAMPLES (MIC_SAMPLE_RATE_HZ / 10) // 100 ms
#define MIC_CHUNK_SAMPLES 256
#define MIC_FULL_SCALE_24 8388608.0

static sensors_available_t s_available;
static QueueHandle_t s_level_queue;
static i2s_chan_handle_t s_mic;

static uint64_t pin_mask(int gpio)
{
    return gpio < 0 ? 0 : 1ULL << gpio;
}

bool sensors_pin_usable(int gpio, const char *label)
{
    if (gpio < 0) {
        ESP_LOGW(TAG, "%s not connected (pin -1)", label);
        return false;
    }
    if (!GPIO_IS_VALID_GPIO(gpio)) {
        ESP_LOGE(TAG, "%s: GPIO%d does not exist", label, gpio);
        return false;
    }
    if (gpio == 19 || gpio == 20) {
        ESP_LOGE(TAG, "%s: GPIO%d is the USB console", label, gpio);
        return false;
    }
    return true;
}

static bool init_touch(void)
{
    if (!sensors_pin_usable(CONFIG_PET_TOUCH_GPIO, "TTP223")) {
        return false;
    }
    const gpio_config_t cfg = {
        .pin_bit_mask = pin_mask(CONFIG_PET_TOUCH_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // defines the level if unplugged
    };
    return gpio_config(&cfg) == ESP_OK;
}

static bool init_led(void)
{
    if (!sensors_pin_usable(CONFIG_PET_LED_GPIO, "LED")) {
        return false;
    }
    const gpio_config_t cfg = {
        .pin_bit_mask = pin_mask(CONFIG_PET_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    if (gpio_config(&cfg) != ESP_OK) {
        return false;
    }
    gpio_set_level(CONFIG_PET_LED_GPIO, 0);
    return true;
}

static void accumulate(const int32_t *samples, int count, double *sum, double *sum_sq)
{
    for (int i = 0; i < count; ++i) {
        const int32_t s = samples[i] >> 8; // INMP441: 24 bits in a 32-bit slot
        *sum += s;
        *sum_sq += (double)s * s;
    }
}

static void mic_task(void *arg)
{
    (void)arg;
    static int32_t chunk[MIC_CHUNK_SAMPLES];
    for (;;) {
        double sum = 0.0;
        double sum_sq = 0.0;
        int taken = 0;
        while (taken < MIC_WINDOW_SAMPLES) {
            size_t bytes = 0;
            if (i2s_channel_read(s_mic, chunk, sizeof(chunk), &bytes, 1000) != ESP_OK) {
                continue;
            }
            int n = (int)(bytes / sizeof(chunk[0]));
            if (n > MIC_WINDOW_SAMPLES - taken) {
                n = MIC_WINDOW_SAMPLES - taken;
            }
            accumulate(chunk, n, &sum, &sum_sq);
            taken += n;
        }
        // Remove the DC offset before computing RMS.
        const double mean = sum / taken;
        double variance = sum_sq / taken - mean * mean;
        if (variance < 0.0) {
            variance = 0.0;
        }
        float level = (float)(sqrt(variance) / MIC_FULL_SCALE_24);
        if (level > 1.0f) {
            level = 1.0f;
        }
        xQueueOverwrite(s_level_queue, &level);
    }
}

static bool init_mic(void)
{
    const bool pins = sensors_pin_usable(CONFIG_PET_MIC_BCLK_GPIO, "MIC SCK") &&
                      sensors_pin_usable(CONFIG_PET_MIC_WS_GPIO, "MIC WS") &&
                      sensors_pin_usable(CONFIG_PET_MIC_DIN_GPIO, "MIC SD");
    if (!pins) {
        return false;
    }
    // The microphone uses I2S0; the amplifier has its own channel on I2S1.
    const i2s_chan_config_t chan = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    if (i2s_new_channel(&chan, NULL, &s_mic) != ESP_OK) {
        return false;
    }
    i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(MIC_SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,
                                                        I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_PET_MIC_BCLK_GPIO,
            .ws = CONFIG_PET_MIC_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din = CONFIG_PET_MIC_DIN_GPIO,
        },
    };
    std.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT; // L/R tied to GND
    if (i2s_channel_init_std_mode(s_mic, &std) != ESP_OK || i2s_channel_enable(s_mic) != ESP_OK) {
        i2s_del_channel(s_mic);
        s_mic = NULL;
        return false;
    }
    s_level_queue = xQueueCreate(1, sizeof(float));
    if (s_level_queue == NULL ||
        xTaskCreate(mic_task, "pet_mic", 3072, NULL, 5, NULL) != pdPASS) {
        return false;
    }
    return true;
}

sensors_available_t sensors_init(void)
{
    s_available.touch = init_touch();
    s_available.led = init_led();
    s_available.mic = init_mic();
    ESP_LOGI(TAG, "touch=%d led=%d mic=%d", s_available.touch, s_available.led,
             s_available.mic);
    return s_available;
}

bool sensors_touch_pressed(bool *pressed)
{
    if (!s_available.touch) {
        return false;
    }
    *pressed = gpio_get_level(CONFIG_PET_TOUCH_GPIO) != 0; // TTP223 active high
    return true;
}

void sensors_set_led(bool on)
{
    if (s_available.led) {
        gpio_set_level(CONFIG_PET_LED_GPIO, on ? 1 : 0);
    }
}

bool sensors_take_sound_level(float *level)
{
    return s_available.mic && xQueueReceive(s_level_queue, level, 0) == pdTRUE;
}
