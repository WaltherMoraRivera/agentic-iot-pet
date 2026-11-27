#pragma once

#include <stdbool.h>
#include <stdint.h>

// Study session timer (T04). All times are monotonic milliseconds since boot,
// passed in by the caller. Civil time never enters this module, so changing
// the wall clock cannot stretch or shorten a session.

#define STUDY_MIN_DURATION_MS 60000u
#define STUDY_MAX_DURATION_MS 7200000u

typedef enum {
    STUDY_IDLE = 0,
    STUDY_RUNNING,
    STUDY_PAUSED,
    STUDY_FINISHED,
    STUDY_INTERRUPTED,
} study_phase_t;

typedef enum {
    STUDY_OK = 0,
    STUDY_ERR_RANGE,       // duration outside [MIN, MAX]
    STUDY_ERR_WRONG_PHASE, // operation not valid in the current phase
} study_result_t;

typedef struct {
    uint32_t session_id; // increases per started session within a boot; 0 = none yet
    study_phase_t phase;
    uint32_t target_ms;
    uint32_t elapsed_ms;      // accumulated before the current running span
    int64_t running_since_ms; // valid only while RUNNING
} study_session_t;

void study_init(study_session_t *s);

study_result_t study_start(study_session_t *s, uint32_t duration_ms, int64_t now_ms);
study_result_t study_pause(study_session_t *s, int64_t now_ms);
study_result_t study_resume(study_session_t *s, int64_t now_ms);
study_result_t study_finish(study_session_t *s, int64_t now_ms);

// Advances the timer; moves RUNNING to FINISHED once the target is reached.
// Returns true when this call produced the transition to FINISHED.
bool study_tick(study_session_t *s, int64_t now_ms);

uint32_t study_elapsed_ms(const study_session_t *s, int64_t now_ms);

// After a reboot: if the persisted marker said a session was active, the
// session is reported as interrupted. Its duration after power loss is unknown
// and is not invented.
void study_restore_after_boot(study_session_t *s, bool marker_was_active);

// True while a session should keep the persisted "active" marker set.
bool study_is_active(const study_session_t *s);
