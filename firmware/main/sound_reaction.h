#pragma once

#include <stdbool.h>
#include <stdint.h>

// Reaction to the relative sound level measured by the INMP441 (T04).
// Levels are normalised to digital full scale, 0..1; they are not dB SPL.
// Pure logic with monotonic time passed in.

#define SOUND_DEFAULT_HYSTERESIS 0.05f
// Initial wait after the pet's own sound before listening again; to be
// measured and adjusted on the board.
#define SOUND_DEFAULT_POST_PLAYBACK_MS 200u

typedef struct {
    float threshold;  // valid range (0, 1]
    float hysteresis; // re-arm when the level falls below threshold - hysteresis
    uint32_t post_playback_ms;
} sound_config_t;

typedef struct {
    sound_config_t config;
    bool calibrated; // reactions stay off until capture is calibrated
    bool above;      // level is above the threshold and has not re-armed yet
    bool playing;    // the pet's own sound is playing
    int64_t inhibit_until_ms;
} sound_reaction_t;

// True when the configuration produces valid limits: threshold in (0, 1],
// hysteresis >= 0 and threshold - hysteresis > 0.
bool sound_config_valid(const sound_config_t *config);

// Starts with reactions disabled until sound_set_calibrated(true).
// Returns false and keeps no configuration if it is invalid.
bool sound_init(sound_reaction_t *r, const sound_config_t *config);

// Replaces the configuration only if valid; the current one stays otherwise.
bool sound_set_config(sound_reaction_t *r, const sound_config_t *config);

void sound_set_calibrated(sound_reaction_t *r, bool calibrated);

// Own playback: reactions are inhibited while playing and for
// post_playback_ms after it ends, so the pet does not react to itself.
void sound_playback_started(sound_reaction_t *r);
void sound_playback_finished(sound_reaction_t *r, int64_t now_ms);

// Feeds one level window. Returns true only on the window that crosses the
// threshold; it re-arms after the level drops below threshold - hysteresis.
bool sound_update(sound_reaction_t *r, float level, int64_t now_ms);
