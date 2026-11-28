#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

// Persistent configuration and the active-session marker (T04), in NVS.
// Written when something changes, never once per second.

typedef struct {
    float sound_threshold;     // (0, 1]
    float sound_hysteresis;    // >= 0, threshold - hysteresis > 0
    uint32_t study_default_ms; // STUDY_MIN_DURATION_MS..STUDY_MAX_DURATION_MS
    uint8_t volume_pct;        // 0..100
    bool muted;
} pet_settings_t;

// Provisional defaults until calibration on the board.
pet_settings_t settings_defaults(void);

bool settings_valid(const pet_settings_t *s);

// Initialises NVS. Erases and re-creates the partition only when NVS reports
// it has no free pages or a newer format, as ESP-IDF documents.
esp_err_t settings_storage_init(void);

// Loads the stored settings. If nothing is stored, or what is stored is
// invalid or from another format, *out gets the defaults and *used_defaults
// is set.
esp_err_t settings_load(pet_settings_t *out, bool *used_defaults);

// Rejects invalid settings with ESP_ERR_INVALID_ARG and keeps the stored ones.
esp_err_t settings_save(const pet_settings_t *s);

esp_err_t settings_set_session_marker(bool active);
// A missing marker reads as inactive.
esp_err_t settings_get_session_marker(bool *active);
