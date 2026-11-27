#pragma once

#include <stdbool.h>
#include <stdint.h>

// Classifies raw TTP223 edges into gestures (T04). Pure logic: the caller
// feeds the sensor level and the monotonic time, and polls for timeouts.
//
// A single tap is reported only after the double-tap gap has passed, so a
// double tap never also produces a tap. A press longer than tap_max_ms but
// shorter than hold_min_ms is ambiguous and produces nothing.

typedef struct {
    uint32_t tap_max_ms;    // longest press still counted as a tap
    uint32_t double_gap_ms; // longest release between the two taps of a double tap
    uint32_t hold_min_ms;   // press length that starts a hold
} gesture_windows_t;

// Provisional windows until H01 measures real contact durations; record the
// calibrated values in the test results before relying on them.
#define GESTURE_WINDOWS_PROVISIONAL {.tap_max_ms = 300, .double_gap_ms = 350, .hold_min_ms = 800}

typedef enum {
    GESTURE_NONE = 0,
    GESTURE_TAP,
    GESTURE_DOUBLE_TAP,
    GESTURE_HOLD_START,
    GESTURE_HOLD_END,
} gesture_kind_t;

typedef struct {
    gesture_kind_t kind;
    int64_t press_start_ms; // start of the first press that formed the gesture
} gesture_event_t;

typedef struct {
    gesture_windows_t windows;
    bool pressed;
    bool holding;
    int64_t press_start_ms;
    bool tap_pending; // one tap seen, waiting to know if a second follows
    int64_t tap_press_start_ms;
    int64_t tap_release_ms;
} gesture_detector_t;

// Returns false if the windows are inconsistent (zero values or
// tap_max_ms >= hold_min_ms).
bool gesture_init(gesture_detector_t *g, gesture_windows_t windows);

// Feeds the current sensor level. Call on every poll; repeated levels are fine.
gesture_event_t gesture_update(gesture_detector_t *g, bool pressed, int64_t now_ms);
