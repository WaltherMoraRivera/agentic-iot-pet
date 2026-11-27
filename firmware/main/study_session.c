#include "study_session.h"

#include <string.h>

void study_init(study_session_t *s)
{
    memset(s, 0, sizeof(*s));
    s->phase = STUDY_IDLE;
}

static uint32_t running_span_ms(const study_session_t *s, int64_t now_ms)
{
    if (s->phase != STUDY_RUNNING || now_ms <= s->running_since_ms) {
        return 0;
    }
    const int64_t span = now_ms - s->running_since_ms;
    return span > STUDY_MAX_DURATION_MS ? STUDY_MAX_DURATION_MS : (uint32_t)span;
}

uint32_t study_elapsed_ms(const study_session_t *s, int64_t now_ms)
{
    const uint32_t total = s->elapsed_ms + running_span_ms(s, now_ms);
    return total > s->target_ms ? s->target_ms : total;
}

study_result_t study_start(study_session_t *s, uint32_t duration_ms, int64_t now_ms)
{
    if (duration_ms < STUDY_MIN_DURATION_MS || duration_ms > STUDY_MAX_DURATION_MS) {
        return STUDY_ERR_RANGE;
    }
    if (study_is_active(s)) {
        return STUDY_ERR_WRONG_PHASE;
    }
    s->session_id += 1;
    s->phase = STUDY_RUNNING;
    s->target_ms = duration_ms;
    s->elapsed_ms = 0;
    s->running_since_ms = now_ms;
    return STUDY_OK;
}

study_result_t study_pause(study_session_t *s, int64_t now_ms)
{
    if (s->phase != STUDY_RUNNING) {
        return STUDY_ERR_WRONG_PHASE;
    }
    s->elapsed_ms = study_elapsed_ms(s, now_ms);
    s->phase = STUDY_PAUSED;
    return STUDY_OK;
}

study_result_t study_resume(study_session_t *s, int64_t now_ms)
{
    if (s->phase != STUDY_PAUSED) {
        return STUDY_ERR_WRONG_PHASE;
    }
    s->phase = STUDY_RUNNING;
    s->running_since_ms = now_ms;
    return STUDY_OK;
}

study_result_t study_finish(study_session_t *s, int64_t now_ms)
{
    if (!study_is_active(s)) {
        return STUDY_ERR_WRONG_PHASE;
    }
    s->elapsed_ms = study_elapsed_ms(s, now_ms);
    s->phase = STUDY_FINISHED;
    return STUDY_OK;
}

bool study_tick(study_session_t *s, int64_t now_ms)
{
    if (s->phase != STUDY_RUNNING || study_elapsed_ms(s, now_ms) < s->target_ms) {
        return false;
    }
    s->elapsed_ms = s->target_ms;
    s->phase = STUDY_FINISHED;
    return true;
}

void study_restore_after_boot(study_session_t *s, bool marker_was_active)
{
    study_init(s);
    if (marker_was_active) {
        s->phase = STUDY_INTERRUPTED;
    }
}

bool study_is_active(const study_session_t *s)
{
    return s->phase == STUDY_RUNNING || s->phase == STUDY_PAUSED;
}
