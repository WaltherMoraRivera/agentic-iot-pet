#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "gestures.h"
#include "study_session.h"

// Local behaviour of the pet (T04): physical confirmations and the priority
// between gestures. Pure logic with monotonic time passed in.

#define CONFIRMATION_TIMEOUT_MS 15000u

typedef enum {
    CONFIRMATION_NONE = 0,
    CONFIRMATION_PENDING,
    CONFIRMATION_APPLIED,
    CONFIRMATION_CANCELLED,
} confirmation_state_t;

typedef struct {
    uint32_t id;           // local id, increases per created confirmation
    uint32_t operation_id; // operation awaiting the touch
    uint32_t lease_epoch;  // control epoch that requested it
    int64_t created_ms;
    int64_t expires_ms;
    confirmation_state_t state;
} confirmation_t;

typedef enum {
    PET_ACTION_NONE = 0,
    PET_ACTION_CONFIRM,       // apply the pending operation
    PET_ACTION_STUDY_STARTED,
    PET_ACTION_STUDY_PAUSED,
    PET_ACTION_STUDY_RESUMED,
    PET_ACTION_VOICE_REQUEST, // ask the hub to open voice capture
    PET_ACTION_VOICE_RELEASE, // hold ended: stop the capture
} pet_action_t;

// AUTO: the pet alerts when the sound reaction triggers. MANUAL: an alert
// set from the app stays on until the app returns to AUTO; sound reactions
// do not add a second alert meanwhile.
typedef enum {
    ALERT_MODE_AUTO = 0,
    ALERT_MODE_MANUAL,
} alert_mode_t;

typedef struct {
    study_session_t study;
    alert_mode_t alert_mode;
    confirmation_t confirmation;
    uint32_t next_confirmation_id;
    uint32_t default_study_ms;
    bool voice_requested; // a hold opened voice and has not ended yet
} pet_state_t;

void pet_init(pet_state_t *p, uint32_t default_study_ms);

// Creates a confirmation for one operation. Fails (returns false) while
// another confirmation is pending.
bool pet_request_confirmation(pet_state_t *p, uint32_t operation_id, uint32_t lease_epoch,
                              int64_t now_ms);

#define PET_NO_WAITING_TAP GESTURE_NO_WAITING_TAP

// Expires the pending confirmation and advances the study timer.
// waiting_tap_release_ms comes from gesture_waiting_tap_release_ms(): a tap
// released inside the window but not reported yet keeps the confirmation
// pending until the detector reports it. Pass PET_NO_WAITING_TAP otherwise.
void pet_tick(pet_state_t *p, int64_t now_ms, int64_t waiting_tap_release_ms);

// A handover, lost link or new epoch cancels the pending confirmation.
void pet_on_epoch_change(pet_state_t *p, uint32_t current_epoch);

// Applies an alert mode already confirmed by touch. Returns true only when the
// mode changes, so repeating the same request has no second effect.
bool pet_set_alert_mode(pet_state_t *p, alert_mode_t mode);

// True when a sound reaction should produce an alert (only in AUTO).
bool pet_sound_reaction_alerts(const pet_state_t *p);

// Resolves one gesture into at most one action, by priority:
// pending confirmation, then hold for voice, double tap, contextual tap.
// A tap confirms when its release (event.release_ms) is inside the window,
// even if the detector reports it after the window closed.
pet_action_t pet_on_gesture(pet_state_t *p, gesture_event_t event, uint32_t current_epoch,
                            int64_t now_ms);
