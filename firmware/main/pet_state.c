#include "pet_state.h"

#include <string.h>

void pet_init(pet_state_t *p, uint32_t default_study_ms)
{
    memset(p, 0, sizeof(*p));
    study_init(&p->study);
    p->confirmation.state = CONFIRMATION_NONE;
    p->next_confirmation_id = 1;
    p->default_study_ms = default_study_ms;
}

static bool confirmation_pending(const pet_state_t *p)
{
    return p->confirmation.state == CONFIRMATION_PENDING;
}

static void expire_if_due(pet_state_t *p, int64_t now_ms)
{
    if (confirmation_pending(p) && now_ms >= p->confirmation.expires_ms) {
        p->confirmation.state = CONFIRMATION_CANCELLED;
    }
}

bool pet_request_confirmation(pet_state_t *p, uint32_t operation_id, uint32_t lease_epoch,
                              int64_t now_ms)
{
    expire_if_due(p, now_ms);
    if (confirmation_pending(p)) {
        return false;
    }
    p->confirmation = (confirmation_t){
        .id = p->next_confirmation_id++,
        .operation_id = operation_id,
        .lease_epoch = lease_epoch,
        .created_ms = now_ms,
        .expires_ms = now_ms + CONFIRMATION_TIMEOUT_MS,
        .state = CONFIRMATION_PENDING,
    };
    return true;
}

void pet_tick(pet_state_t *p, int64_t now_ms)
{
    expire_if_due(p, now_ms);
    study_tick(&p->study, now_ms);
}

void pet_on_epoch_change(pet_state_t *p, uint32_t current_epoch)
{
    if (confirmation_pending(p) && p->confirmation.lease_epoch != current_epoch) {
        p->confirmation.state = CONFIRMATION_CANCELLED;
    }
}

// While a confirmation is pending, touches belong to it: they either confirm
// or do nothing, so one touch can never confirm and start something else.
static pet_action_t resolve_confirmation(pet_state_t *p, gesture_event_t event,
                                         uint32_t current_epoch)
{
    if (event.kind != GESTURE_TAP && event.kind != GESTURE_DOUBLE_TAP) {
        return PET_ACTION_NONE;
    }
    if (p->confirmation.lease_epoch != current_epoch) {
        p->confirmation.state = CONFIRMATION_CANCELLED;
        return PET_ACTION_NONE;
    }
    // A press that began before the request (for example the one that opened
    // a mode) cannot confirm; the user has to release and touch again.
    if (event.press_start_ms <= p->confirmation.created_ms) {
        return PET_ACTION_NONE;
    }
    p->confirmation.state = CONFIRMATION_APPLIED;
    return PET_ACTION_CONFIRM;
}

pet_action_t pet_on_gesture(pet_state_t *p, gesture_event_t event, uint32_t current_epoch,
                            int64_t now_ms)
{
    expire_if_due(p, now_ms);

    // Ending a hold always closes voice, even if a confirmation arrived meanwhile.
    if (event.kind == GESTURE_HOLD_END) {
        if (p->voice_requested) {
            p->voice_requested = false;
            return PET_ACTION_VOICE_RELEASE;
        }
        return PET_ACTION_NONE;
    }

    if (confirmation_pending(p)) {
        return resolve_confirmation(p, event, current_epoch);
    }

    switch (event.kind) {
    case GESTURE_HOLD_START:
        p->voice_requested = true;
        return PET_ACTION_VOICE_REQUEST;
    case GESTURE_DOUBLE_TAP:
        if (study_pause(&p->study, now_ms) == STUDY_OK) {
            return PET_ACTION_STUDY_PAUSED;
        }
        if (study_resume(&p->study, now_ms) == STUDY_OK) {
            return PET_ACTION_STUDY_RESUMED;
        }
        return PET_ACTION_NONE;
    case GESTURE_TAP:
        if (study_start(&p->study, p->default_study_ms, now_ms) == STUDY_OK) {
            return PET_ACTION_STUDY_STARTED;
        }
        return PET_ACTION_NONE;
    default:
        return PET_ACTION_NONE;
    }
}
