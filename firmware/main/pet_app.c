#include "pet_app.h"

#include <inttypes.h>

#include "audio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gestures.h"
#include "pet_state.h"
#include "sdkconfig.h"
#include "sensors.h"
#include "settings.h"
#include "sound_reaction.h"
#include "telemetry.h"

static const char *const TAG = "PET";

#define LOOP_MS 10
#define SAMPLE_PERIOD_MS 1000

// No control authority exists before T03; every confirmation would belong to
// this placeholder epoch. Nothing requests confirmations until the API exists.
#define LOCAL_EPOCH 0u

static pet_state_t s_pet;
static sound_reaction_t s_sound;
static gesture_detector_t s_gestures;
static telemetry_t s_telemetry;
static pet_settings_t s_settings;

static int64_t now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

static void set_session_marker(bool active)
{
    const esp_err_t err = settings_set_session_marker(active);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not store session marker: %s", esp_err_to_name(err));
    }
}

static void record_study_change(int64_t now)
{
    telemetry_add_event(&s_telemetry, now, TELEMETRY_EVENT_STUDY_CHANGED, s_pet.study.phase);
    ESP_LOGI(TAG, "study session=%" PRIu32 " phase=%d elapsed=%" PRIu32 " ms target=%" PRIu32
                  " ms",
             s_pet.study.session_id, (int)s_pet.study.phase,
             study_elapsed_ms(&s_pet.study, now), s_pet.study.target_ms);
}

static void handle_action(pet_action_t action, int64_t now)
{
    switch (action) {
    case PET_ACTION_STUDY_STARTED:
        set_session_marker(true);
        record_study_change(now);
        audio_play(SOUND_TOUCH);
        break;
    case PET_ACTION_STUDY_PAUSED:
    case PET_ACTION_STUDY_RESUMED:
        record_study_change(now);
        audio_play(SOUND_TOUCH);
        break;
    case PET_ACTION_CONFIRM:
        telemetry_add_event(&s_telemetry, now, TELEMETRY_EVENT_CONFIRMATION,
                            s_pet.confirmation.operation_id);
        audio_play(SOUND_CONFIRM);
        break;
    case PET_ACTION_VOICE_REQUEST:
        // The LED is the listening indicator. Capture itself happens in the
        // app (T08); without a hub the request only shows on the board.
        sensors_set_led(true);
        ESP_LOGI(TAG, "voice requested (no hub connected)");
        break;
    case PET_ACTION_VOICE_RELEASE:
        sensors_set_led(false);
        ESP_LOGI(TAG, "voice released");
        break;
    case PET_ACTION_NONE:
    default:
        break;
    }
}

static void restore_after_boot(void)
{
    bool marker = false;
    if (settings_get_session_marker(&marker) != ESP_OK) {
        ESP_LOGE(TAG, "Could not read session marker; assuming no active session");
    }
    study_restore_after_boot(&s_pet.study, marker);
    if (marker) {
        // The duration after the power loss is unknown and is not invented.
        ESP_LOGW(TAG, "Previous session was interrupted by a reboot");
        record_study_change(now_ms());
        set_session_marker(false);
    }
}

static void load_settings(void)
{
    esp_err_t err = settings_storage_init();
    bool used_defaults = true;
    if (err == ESP_OK) {
        err = settings_load(&s_settings, &used_defaults);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Settings storage unavailable (%s): using defaults", esp_err_to_name(err));
        s_settings = settings_defaults();
    }
    ESP_LOGI(TAG, "settings %s: threshold=%.3f hysteresis=%.3f study=%" PRIu32
                  " ms volume=%u%% muted=%d",
             used_defaults ? "defaults" : "stored", s_settings.sound_threshold,
             s_settings.sound_hysteresis, s_settings.study_default_ms,
             (unsigned)s_settings.volume_pct, s_settings.muted);
}

static void pet_task(void *arg)
{
    (void)arg;
    load_settings();
    telemetry_init(&s_telemetry);
    pet_init(&s_pet, s_settings.study_default_ms);
    restore_after_boot();

    const sound_config_t sound_cfg = {
        .threshold = s_settings.sound_threshold,
        .hysteresis = s_settings.sound_hysteresis,
        .post_playback_ms = SOUND_DEFAULT_POST_PLAYBACK_MS,
    };
    sound_init(&s_sound, &sound_cfg);
#if CONFIG_PET_SOUND_CALIBRATED
    sound_set_calibrated(&s_sound, true);
#else
    ESP_LOGW(TAG, "Sound capture not calibrated: level is recorded, reactions are off");
#endif

    const gesture_windows_t windows = GESTURE_WINDOWS_PROVISIONAL;
    gesture_init(&s_gestures, windows);
    sensors_init();
    if (audio_init()) {
        audio_set_output(s_settings.volume_pct, s_settings.muted);
    }

    float last_level = 0.0f;
    int64_t next_sample_ms = now_ms();
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(LOOP_MS) > 0 ? pdMS_TO_TICKS(LOOP_MS) : 1;

    for (;;) {
        const int64_t now = now_ms();

        audio_event_t audio;
        while (audio_take_event(&audio)) {
            if (audio.playing) {
                sound_playback_started(&s_sound);
            } else {
                sound_playback_finished(&s_sound, audio.at_ms);
            }
        }

        bool pressed = false;
        if (sensors_touch_pressed(&pressed)) {
            const gesture_event_t gesture = gesture_update(&s_gestures, pressed, now);
            if (gesture.kind != GESTURE_NONE) {
                telemetry_add_event(&s_telemetry, now, TELEMETRY_EVENT_TOUCH, gesture.kind);
                handle_action(pet_on_gesture(&s_pet, gesture, LOCAL_EPOCH, now), now);
            }
        }

        const study_phase_t before = s_pet.study.phase;
        pet_tick(&s_pet, now);
        if (before != s_pet.study.phase && s_pet.study.phase == STUDY_FINISHED) {
            set_session_marker(false);
            record_study_change(now);
            audio_play(SOUND_STUDY_DONE);
        }

        float level;
        if (sensors_take_sound_level(&level)) {
            last_level = level;
            if (sound_update(&s_sound, level, now)) {
                telemetry_add_event(&s_telemetry, now, TELEMETRY_EVENT_SOUND_REACTION,
                                    (uint32_t)(level * 1000.0f));
                if (pet_sound_reaction_alerts(&s_pet)) {
                    audio_play(SOUND_ALERT);
                }
            }
        }

        if (now >= next_sample_ms) {
            telemetry_add_sample(&s_telemetry, now, last_level, (uint8_t)s_pet.study.phase);
            next_sample_ms += SAMPLE_PERIOD_MS;
        }

        xTaskDelayUntil(&last_wake, period);
    }
}

void pet_app_start(void)
{
    if (xTaskCreate(pet_task, "pet", 6144, NULL, 6, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Could not start the pet task");
    }
}
