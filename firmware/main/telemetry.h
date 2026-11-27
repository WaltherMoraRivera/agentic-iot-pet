#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// In-memory telemetry for recovery after a link cut (T04/T05): one periodic
// sample per second in a ring of 600 (ten minutes), plus functional events
// in a separate ring. Overwritten entries are counted, never hidden.
// Sequences start at 1 within a boot and never repeat.

#define TELEMETRY_SAMPLE_CAPACITY 600u
#define TELEMETRY_EVENT_CAPACITY 64u

typedef struct {
    uint32_t seq;
    int64_t uptime_ms;
    float sound_level; // relative 0..1, not dB
    uint8_t study_phase;
} telemetry_sample_t;

typedef enum {
    TELEMETRY_EVENT_TOUCH = 1,
    TELEMETRY_EVENT_SOUND_REACTION,
    TELEMETRY_EVENT_STUDY_CHANGED,
    TELEMETRY_EVENT_CONFIRMATION,
} telemetry_event_kind_t;

typedef struct {
    uint32_t seq;
    int64_t uptime_ms;
    uint8_t kind;  // telemetry_event_kind_t
    uint32_t value; // meaning depends on kind (action, phase, ...)
} telemetry_event_t;

typedef struct {
    telemetry_sample_t samples[TELEMETRY_SAMPLE_CAPACITY];
    uint32_t sample_next_seq;
    uint32_t samples_dropped;
    telemetry_event_t events[TELEMETRY_EVENT_CAPACITY];
    uint32_t event_next_seq;
    uint32_t events_dropped;
} telemetry_t;

void telemetry_init(telemetry_t *t);

// Stores the sample and assigns its sequence. Returns the sequence.
uint32_t telemetry_add_sample(telemetry_t *t, int64_t uptime_ms, float sound_level,
                              uint8_t study_phase);
uint32_t telemetry_add_event(telemetry_t *t, int64_t uptime_ms, telemetry_event_kind_t kind,
                             uint32_t value);

// Oldest sequence still stored, or 0 when empty.
uint32_t telemetry_first_sample_seq(const telemetry_t *t);
uint32_t telemetry_first_event_seq(const telemetry_t *t);

// A cursor is valid from 0 up to the last issued sequence. A larger one
// (including UINT32_MAX) was never issued in this boot: the API must reject
// it with INVALID_MESSAGE instead of reading.
bool telemetry_sample_cursor_valid(const telemetry_t *t, uint32_t after_seq);
bool telemetry_event_cursor_valid(const telemetry_t *t, uint32_t after_seq);

// Copies up to max entries with seq > after_seq, in order. *gap is set when
// entries after after_seq were already overwritten, so the reader can show
// the real gap instead of a silent jump. An invalid cursor reads nothing and
// sets no gap.
size_t telemetry_read_samples(const telemetry_t *t, uint32_t after_seq, telemetry_sample_t *out,
                              size_t max, bool *gap);
size_t telemetry_read_events(const telemetry_t *t, uint32_t after_seq, telemetry_event_t *out,
                             size_t max, bool *gap);
