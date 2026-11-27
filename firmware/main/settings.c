#include "settings.h"

#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"
#include "sound_reaction.h"
#include "study_session.h"

#define NVS_NAMESPACE "pet"
#define KEY_SETTINGS "settings"
#define KEY_SESSION "session_on"
#define SETTINGS_FORMAT 1

typedef struct {
    uint8_t format;
    pet_settings_t settings;
} stored_settings_t;

pet_settings_t settings_defaults(void)
{
    return (pet_settings_t){
        .sound_threshold = 0.5f,
        .sound_hysteresis = SOUND_DEFAULT_HYSTERESIS,
        .study_default_ms = 25u * 60u * 1000u,
        .volume_pct = 30,
        .muted = false,
    };
}

bool settings_valid(const pet_settings_t *s)
{
    const sound_config_t sound = {.threshold = s->sound_threshold,
                                  .hysteresis = s->sound_hysteresis};
    return sound_config_valid(&sound) && s->study_default_ms >= STUDY_MIN_DURATION_MS &&
           s->study_default_ms <= STUDY_MAX_DURATION_MS && s->volume_pct <= 100;
}

esp_err_t settings_storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err = nvs_flash_erase();
        if (err == ESP_OK) {
            err = nvs_flash_init();
        }
    }
    return err;
}

esp_err_t settings_load(pet_settings_t *out, bool *used_defaults)
{
    *out = settings_defaults();
    *used_defaults = true;

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK; // namespace not created yet: nothing stored
    }
    if (err != ESP_OK) {
        return err;
    }

    stored_settings_t stored;
    size_t size = sizeof(stored);
    err = nvs_get_blob(h, KEY_SETTINGS, &stored, &size);
    nvs_close(h);
    if (err == ESP_ERR_NVS_NOT_FOUND || err == ESP_ERR_NVS_INVALID_LENGTH) {
        return ESP_OK;
    }
    if (err != ESP_OK) {
        return err;
    }
    if (size == sizeof(stored) && stored.format == SETTINGS_FORMAT &&
        settings_valid(&stored.settings)) {
        *out = stored.settings;
        *used_defaults = false;
    }
    return ESP_OK;
}

esp_err_t settings_save(const pet_settings_t *s)
{
    if (!settings_valid(s)) {
        return ESP_ERR_INVALID_ARG;
    }
    stored_settings_t stored;
    memset(&stored, 0, sizeof(stored));
    stored.format = SETTINGS_FORMAT;
    stored.settings = *s;

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_blob(h, KEY_SETTINGS, &stored, sizeof(stored));
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err;
}

esp_err_t settings_set_session_marker(bool active)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_u8(h, KEY_SESSION, active ? 1 : 0);
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err;
}

esp_err_t settings_get_session_marker(bool *active)
{
    *active = false;
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK;
    }
    if (err != ESP_OK) {
        return err;
    }
    uint8_t value = 0;
    err = nvs_get_u8(h, KEY_SESSION, &value);
    nvs_close(h);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK;
    }
    if (err == ESP_OK) {
        *active = value != 0;
    }
    return err;
}
