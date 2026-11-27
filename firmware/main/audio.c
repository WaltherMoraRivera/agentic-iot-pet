#include "audio.h"

#include <string.h>

#include "driver/i2s_std.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "sensors.h"

static const char *const TAG = "PET_AUDIO";

#define DMA_DESC_NUM 4
#define DMA_FRAME_NUM 240
#define BLOCK_FRAMES 240

static i2s_chan_handle_t s_tx;
static QueueHandle_t s_requests;
static QueueHandle_t s_events;
static volatile uint8_t s_volume_pct = 30;
static volatile bool s_muted;
static bool s_available;

static int64_t now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

static void post_event(bool playing)
{
    const audio_event_t e = {.playing = playing, .at_ms = now_ms()};
    // If the main loop is behind, keep the newest state: losing a start
    // followed by its finish is harmless, keeping a stale "playing" is not.
    if (xQueueSend(s_events, &e, 0) != pdTRUE) {
        xQueueReset(s_events);
        xQueueSend(s_events, &e, 0);
    }
}

static void write_frames(const int16_t *stereo, size_t frames)
{
    size_t written = 0;
    i2s_channel_write(s_tx, stereo, frames * 2 * sizeof(int16_t), &written, portMAX_DELAY);
}

static void play(const sound_pcm_t *pcm, uint8_t volume_pct)
{
    static int16_t block[BLOCK_FRAMES * 2];
    const int32_t scale = volume_pct; // percent of the stored amplitude

    post_event(true);
    for (size_t pos = 0; pos < pcm->count; pos += BLOCK_FRAMES) {
        size_t n = pcm->count - pos;
        if (n > BLOCK_FRAMES) {
            n = BLOCK_FRAMES;
        }
        for (size_t i = 0; i < n; ++i) {
            // Same sample on both slots: works whatever channel the module mixes.
            const int16_t s = (int16_t)((int32_t)pcm->samples[pos + i] * scale / 100);
            block[2 * i] = s;
            block[2 * i + 1] = s;
        }
        write_frames(block, n);
    }
    // Push the whole DMA ring with silence so the finish notification is sent
    // only after the last real sample has been played.
    memset(block, 0, sizeof(block));
    for (int i = 0; i < DMA_DESC_NUM * DMA_FRAME_NUM / BLOCK_FRAMES; ++i) {
        write_frames(block, BLOCK_FRAMES);
    }
    post_event(false);
}

static void audio_task(void *arg)
{
    (void)arg;
    sound_id_t sound;
    for (;;) {
        if (xQueueReceive(s_requests, &sound, portMAX_DELAY) == pdTRUE && sound < SOUND_COUNT) {
            play(&sound_pcm_table[sound], s_volume_pct);
        }
    }
}

bool audio_init(void)
{
    const bool pins = sensors_pin_usable(CONFIG_PET_AMP_BCLK_GPIO, "AMP BCLK") &&
                      sensors_pin_usable(CONFIG_PET_AMP_LRC_GPIO, "AMP LRC") &&
                      sensors_pin_usable(CONFIG_PET_AMP_DIN_GPIO, "AMP DIN");
    if (!pins) {
        ESP_LOGW(TAG, "Amplifier not configured: sounds disabled");
        return false;
    }

    i2s_chan_config_t chan = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    chan.dma_desc_num = DMA_DESC_NUM;
    chan.dma_frame_num = DMA_FRAME_NUM;
    chan.auto_clear = true;
    if (i2s_new_channel(&chan, &s_tx, NULL) != ESP_OK) {
        return false;
    }
    const i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SOUND_SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                        I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_PET_AMP_BCLK_GPIO,
            .ws = CONFIG_PET_AMP_LRC_GPIO,
            .dout = CONFIG_PET_AMP_DIN_GPIO,
            .din = I2S_GPIO_UNUSED,
        },
    };
    if (i2s_channel_init_std_mode(s_tx, &std) != ESP_OK || i2s_channel_enable(s_tx) != ESP_OK) {
        i2s_del_channel(s_tx);
        s_tx = NULL;
        return false;
    }

    s_requests = xQueueCreate(4, sizeof(sound_id_t));
    s_events = xQueueCreate(8, sizeof(audio_event_t));
    if (s_requests == NULL || s_events == NULL ||
        xTaskCreate(audio_task, "pet_audio", 3072, NULL, 4, NULL) != pdPASS) {
        return false;
    }
    s_available = true;
    return true;
}

void audio_set_output(uint8_t volume_pct, bool muted)
{
    s_volume_pct = volume_pct > 100 ? 100 : volume_pct;
    s_muted = muted;
}

bool audio_play(sound_id_t sound)
{
    if (!s_available || s_muted || s_volume_pct == 0 || sound >= SOUND_COUNT) {
        return false;
    }
    return xQueueSend(s_requests, &sound, 0) == pdTRUE;
}

bool audio_take_event(audio_event_t *event)
{
    return s_available && xQueueReceive(s_events, event, 0) == pdTRUE;
}
