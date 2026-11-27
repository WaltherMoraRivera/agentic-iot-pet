#include "gestures.h"

#include <string.h>

static gesture_event_t none(void)
{
    return (gesture_event_t){.kind = GESTURE_NONE, .press_start_ms = 0};
}

static gesture_event_t make(gesture_kind_t kind, int64_t press_start_ms)
{
    return (gesture_event_t){.kind = kind, .press_start_ms = press_start_ms};
}

bool gesture_init(gesture_detector_t *g, gesture_windows_t windows)
{
    memset(g, 0, sizeof(*g));
    if (windows.tap_max_ms == 0 || windows.double_gap_ms == 0 ||
        windows.tap_max_ms >= windows.hold_min_ms) {
        return false;
    }
    g->windows = windows;
    return true;
}

// Reports the waiting tap as a single tap and clears it.
static gesture_event_t flush_tap(gesture_detector_t *g)
{
    g->tap_pending = false;
    return make(GESTURE_TAP, g->tap_press_start_ms);
}

gesture_event_t gesture_update(gesture_detector_t *g, bool pressed, int64_t now_ms)
{
    const gesture_windows_t *w = &g->windows;

    if (pressed && !g->pressed) {
        // New press. A tap waiting longer than the gap is final: report it now.
        g->pressed = true;
        g->holding = false;
        g->press_start_ms = now_ms;
        if (g->tap_pending && now_ms - g->tap_release_ms > w->double_gap_ms) {
            return flush_tap(g);
        }
        return none();
    }

    if (pressed) {
        const int64_t held = now_ms - g->press_start_ms;
        // The second press is too long for a double tap: the first was a tap.
        if (g->tap_pending && held > w->tap_max_ms) {
            return flush_tap(g);
        }
        if (!g->holding && held >= w->hold_min_ms) {
            g->holding = true;
            return make(GESTURE_HOLD_START, g->press_start_ms);
        }
        return none();
    }

    if (g->pressed) {
        // Release.
        g->pressed = false;
        const int64_t held = now_ms - g->press_start_ms;
        if (g->holding) {
            g->holding = false;
            return make(GESTURE_HOLD_END, g->press_start_ms);
        }
        if (held > w->tap_max_ms) {
            return none(); // ambiguous length: neither tap nor hold
        }
        if (g->tap_pending) {
            g->tap_pending = false;
            return make(GESTURE_DOUBLE_TAP, g->tap_press_start_ms);
        }
        g->tap_pending = true;
        g->tap_press_start_ms = g->press_start_ms;
        g->tap_release_ms = now_ms;
        return none();
    }

    // Released and idle: the waiting tap becomes final once the gap passes.
    if (g->tap_pending && now_ms - g->tap_release_ms > w->double_gap_ms) {
        return flush_tap(g);
    }
    return none();
}
