#include "sound_reaction.h"

#include <math.h>
#include <string.h>

bool sound_config_valid(const sound_config_t *config)
{
    const float t = config->threshold;
    const float h = config->hysteresis;
    return isfinite(t) && isfinite(h) && t > 0.0f && t <= 1.0f && h >= 0.0f && t - h > 0.0f;
}

bool sound_init(sound_reaction_t *r, const sound_config_t *config)
{
    memset(r, 0, sizeof(*r));
    if (!sound_config_valid(config)) {
        return false;
    }
    r->config = *config;
    return true;
}

bool sound_set_config(sound_reaction_t *r, const sound_config_t *config)
{
    if (!sound_config_valid(config)) {
        return false;
    }
    r->config = *config;
    r->above = false; // re-evaluate against the new limits
    return true;
}

void sound_set_calibrated(sound_reaction_t *r, bool calibrated)
{
    r->calibrated = calibrated;
    r->above = false;
}

void sound_playback_started(sound_reaction_t *r)
{
    r->playing = true;
}

void sound_playback_finished(sound_reaction_t *r, int64_t now_ms)
{
    r->playing = false;
    r->inhibit_until_ms = now_ms + r->config.post_playback_ms;
}

bool sound_update(sound_reaction_t *r, float level, int64_t now_ms)
{
    if (!r->calibrated || !isfinite(level)) {
        return false;
    }
    if (r->playing || now_ms < r->inhibit_until_ms) {
        // Levels measured over our own sound are ignored and do not change the
        // detector. If a reaction caused the playback, `above` stays set, so
        // noise that persists afterwards cannot trigger again until it drops.
        return false;
    }
    if (r->above) {
        if (level < r->config.threshold - r->config.hysteresis) {
            r->above = false;
        }
        return false;
    }
    if (level >= r->config.threshold) {
        r->above = true;
        return true;
    }
    return false;
}
