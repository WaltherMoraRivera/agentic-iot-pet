#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sounds_pcm.h"

// Short sounds through the MAX98357A on I2S1 (T04). Playback runs in its own
// task so the timer and the gestures never wait for audio.

typedef struct {
    bool playing;   // true: playback started; false: the last sample left the speaker
    int64_t at_ms;  // monotonic time of the change
} audio_event_t;

// Returns false when the amplifier pins are not configured; play requests are
// then ignored and the pet keeps working silently.
bool audio_init(void);

// Volume 0..100 and mute. Takes effect from the next sound.
void audio_set_output(uint8_t volume_pct, bool muted);

// Queues a sound. Ignored when muted, at volume 0 or without amplifier.
// Returns true if the sound was queued.
bool audio_play(sound_id_t sound);

// Start/finish notifications, so the sound reaction can ignore our own sound.
// Returns false when there is no pending notification.
bool audio_take_event(audio_event_t *event);
