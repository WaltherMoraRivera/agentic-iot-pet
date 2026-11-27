#include "telemetry.h"

#include <stdint.h>
#include <string.h>

void telemetry_init(telemetry_t *t)
{
    memset(t, 0, sizeof(*t));
    t->sample_next_seq = 1;
    t->event_next_seq = 1;
}

// Oldest stored sequence for a ring whose next sequence is `next`.
static uint32_t first_seq(uint32_t next, uint32_t capacity)
{
    const uint32_t written = next - 1;
    if (written == 0) {
        return 0;
    }
    return written > capacity ? next - capacity : 1;
}

// Valid cursors are 0..next-1 (the last issued sequence). UINT32_MAX is
// checked on its own so that after_seq + 1 never wraps to 0.
static bool cursor_valid(uint32_t next, uint32_t after_seq)
{
    return after_seq != UINT32_MAX && after_seq < next;
}

// Range [from, to) of sequences to copy for a read after `after_seq`.
static size_t plan_read(uint32_t next, uint32_t capacity, uint32_t after_seq, size_t max,
                        uint32_t *from, bool *gap)
{
    const uint32_t first = first_seq(next, capacity);
    *gap = false;
    if (!cursor_valid(next, after_seq) || first == 0 || after_seq + 1 >= next) {
        return 0;
    }
    uint32_t start = after_seq + 1;
    if (start < first) {
        *gap = true;
        start = first;
    }
    const uint32_t available = next - start;
    *from = start;
    return available < max ? available : max;
}

uint32_t telemetry_add_sample(telemetry_t *t, int64_t uptime_ms, float sound_level,
                              uint8_t study_phase)
{
    const uint32_t seq = t->sample_next_seq++;
    if (seq > TELEMETRY_SAMPLE_CAPACITY) {
        ++t->samples_dropped;
    }
    t->samples[(seq - 1) % TELEMETRY_SAMPLE_CAPACITY] = (telemetry_sample_t){
        .seq = seq, .uptime_ms = uptime_ms, .sound_level = sound_level, .study_phase = study_phase};
    return seq;
}

uint32_t telemetry_add_event(telemetry_t *t, int64_t uptime_ms, telemetry_event_kind_t kind,
                             uint32_t value)
{
    const uint32_t seq = t->event_next_seq++;
    if (seq > TELEMETRY_EVENT_CAPACITY) {
        ++t->events_dropped;
    }
    t->events[(seq - 1) % TELEMETRY_EVENT_CAPACITY] = (telemetry_event_t){
        .seq = seq, .uptime_ms = uptime_ms, .kind = (uint8_t)kind, .value = value};
    return seq;
}

uint32_t telemetry_first_sample_seq(const telemetry_t *t)
{
    return first_seq(t->sample_next_seq, TELEMETRY_SAMPLE_CAPACITY);
}

uint32_t telemetry_first_event_seq(const telemetry_t *t)
{
    return first_seq(t->event_next_seq, TELEMETRY_EVENT_CAPACITY);
}

bool telemetry_sample_cursor_valid(const telemetry_t *t, uint32_t after_seq)
{
    return cursor_valid(t->sample_next_seq, after_seq);
}

bool telemetry_event_cursor_valid(const telemetry_t *t, uint32_t after_seq)
{
    return cursor_valid(t->event_next_seq, after_seq);
}

size_t telemetry_read_samples(const telemetry_t *t, uint32_t after_seq, telemetry_sample_t *out,
                              size_t max, bool *gap)
{
    uint32_t from = 0;
    const size_t n = plan_read(t->sample_next_seq, TELEMETRY_SAMPLE_CAPACITY, after_seq, max,
                               &from, gap);
    for (size_t i = 0; i < n; ++i) {
        out[i] = t->samples[(from + i - 1) % TELEMETRY_SAMPLE_CAPACITY];
    }
    return n;
}

size_t telemetry_read_events(const telemetry_t *t, uint32_t after_seq, telemetry_event_t *out,
                             size_t max, bool *gap)
{
    uint32_t from = 0;
    const size_t n = plan_read(t->event_next_seq, TELEMETRY_EVENT_CAPACITY, after_seq, max,
                               &from, gap);
    for (size_t i = 0; i < n; ++i) {
        out[i] = t->events[(from + i - 1) % TELEMETRY_EVENT_CAPACITY];
    }
    return n;
}
