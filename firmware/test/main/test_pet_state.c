#include <stdint.h>
#include <sys/time.h>

#include "esp_timer.h"
#include "gestures.h"
#include "pet_state.h"
#include "study_session.h"
#include "test_suites.h"
#include "unity.h"

static const gesture_windows_t W = GESTURE_WINDOWS_PROVISIONAL;

// A gesture whose first tap lasted 100 ms. Tests that depend on the release
// time build the event with ev_released().
static gesture_event_t ev(gesture_kind_t kind, int64_t press_start_ms)
{
    return (gesture_event_t){
        .kind = kind, .press_start_ms = press_start_ms, .release_ms = press_start_ms + 100};
}

static gesture_event_t ev_released(gesture_kind_t kind, int64_t press_start_ms,
                                   int64_t release_ms)
{
    return (gesture_event_t){
        .kind = kind, .press_start_ms = press_start_ms, .release_ms = release_ms};
}

// Runs the real detector and the pet in the same order as pet_app.c, every
// 10 ms, with the sensor pressed in [press_ms, release_ms). Returns how many
// confirmations were applied.
static int run_loop_with_touch(pet_state_t *p, int64_t press_ms, int64_t release_ms,
                               int64_t end_ms)
{
    gesture_detector_t g;
    gesture_init(&g, W);
    int confirmed = 0;
    for (int64_t t = 0; t <= end_ms; t += 10) {
        const gesture_event_t e = gesture_update(&g, t >= press_ms && t < release_ms, t);
        if (e.kind != GESTURE_NONE && pet_on_gesture(p, e, 1, t) == PET_ACTION_CONFIRM) {
            ++confirmed;
        }
        pet_tick(p, t, gesture_waiting_tap_release_ms(&g));
    }
    return confirmed;
}

// --- Study session -------------------------------------------------------

static void test_study_rejects_out_of_range_durations(void)
{
    study_session_t s;
    study_init(&s);
    TEST_ASSERT_EQUAL(STUDY_ERR_RANGE, study_start(&s, STUDY_MIN_DURATION_MS - 1, 0));
    TEST_ASSERT_EQUAL(STUDY_ERR_RANGE, study_start(&s, STUDY_MAX_DURATION_MS + 1, 0));
    TEST_ASSERT_EQUAL(STUDY_IDLE, s.phase);
    TEST_ASSERT_EQUAL(STUDY_OK, study_start(&s, STUDY_MAX_DURATION_MS, 0));
    study_init(&s);
    TEST_ASSERT_EQUAL(STUDY_OK, study_start(&s, STUDY_MIN_DURATION_MS, 0));
}

static void test_study_finishes_once_at_target(void)
{
    study_session_t s;
    study_init(&s);
    TEST_ASSERT_EQUAL(STUDY_OK, study_start(&s, 60000, 1000));
    TEST_ASSERT_FALSE(study_tick(&s, 60999));
    TEST_ASSERT_EQUAL(STUDY_RUNNING, s.phase);
    TEST_ASSERT_EQUAL_UINT32(59999, study_elapsed_ms(&s, 60999));
    TEST_ASSERT_TRUE(study_tick(&s, 61000));
    TEST_ASSERT_EQUAL(STUDY_FINISHED, s.phase);
    TEST_ASSERT_FALSE(study_tick(&s, 70000));
    TEST_ASSERT_EQUAL_UINT32(60000, study_elapsed_ms(&s, 70000));
}

static void test_study_pause_stops_time_and_resume_continues(void)
{
    study_session_t s;
    study_init(&s);
    study_start(&s, 60000, 0);
    TEST_ASSERT_EQUAL(STUDY_OK, study_pause(&s, 10000));
    TEST_ASSERT_EQUAL(STUDY_ERR_WRONG_PHASE, study_pause(&s, 11000));
    TEST_ASSERT_FALSE(study_tick(&s, 500000));
    TEST_ASSERT_EQUAL_UINT32(10000, study_elapsed_ms(&s, 500000));
    TEST_ASSERT_EQUAL(STUDY_OK, study_resume(&s, 500000));
    TEST_ASSERT_FALSE(study_tick(&s, 549999));
    TEST_ASSERT_TRUE(study_tick(&s, 550000));
}

static void test_study_rejects_start_while_active(void)
{
    study_session_t s;
    study_init(&s);
    study_start(&s, 60000, 0);
    const uint32_t id = s.session_id;
    TEST_ASSERT_EQUAL(STUDY_ERR_WRONG_PHASE, study_start(&s, 90000, 100));
    study_pause(&s, 200);
    TEST_ASSERT_EQUAL(STUDY_ERR_WRONG_PHASE, study_start(&s, 90000, 300));
    TEST_ASSERT_EQUAL_UINT32(id, s.session_id);
    TEST_ASSERT_EQUAL(STUDY_PAUSED, s.phase);
}

static void test_study_operations_rejected_when_idle(void)
{
    study_session_t s;
    study_init(&s);
    TEST_ASSERT_EQUAL(STUDY_ERR_WRONG_PHASE, study_pause(&s, 0));
    TEST_ASSERT_EQUAL(STUDY_ERR_WRONG_PHASE, study_resume(&s, 0));
    TEST_ASSERT_EQUAL(STUDY_ERR_WRONG_PHASE, study_finish(&s, 0));
    TEST_ASSERT_FALSE(study_is_active(&s));
}

static void test_study_new_session_after_finish_gets_new_id(void)
{
    study_session_t s;
    study_init(&s);
    study_start(&s, 60000, 0);
    const uint32_t first = s.session_id;
    TEST_ASSERT_EQUAL(STUDY_OK, study_finish(&s, 1000));
    TEST_ASSERT_EQUAL(STUDY_FINISHED, s.phase);
    TEST_ASSERT_EQUAL(STUDY_OK, study_start(&s, 60000, 2000));
    TEST_ASSERT_NOT_EQUAL(first, s.session_id);
    TEST_ASSERT_EQUAL_UINT32(0, study_elapsed_ms(&s, 2000));
}

static void test_study_reboot_with_marker_is_interrupted_without_duration(void)
{
    study_session_t s;
    study_init(&s);
    study_restore_after_boot(&s, true);
    TEST_ASSERT_EQUAL(STUDY_INTERRUPTED, s.phase);
    TEST_ASSERT_EQUAL_UINT32(0, study_elapsed_ms(&s, 999999));
    TEST_ASSERT_FALSE(study_is_active(&s));
    TEST_ASSERT_EQUAL(STUDY_OK, study_start(&s, 60000, 0));

    study_init(&s);
    study_restore_after_boot(&s, false);
    TEST_ASSERT_EQUAL(STUDY_IDLE, s.phase);
}

// Runs on the target: moves the civil clock one hour forward and checks that
// the monotonic time feeding the session does not jump. The lease part of
// this check belongs to T03, which does not exist yet.
static void test_wall_clock_does_not_change_session(void)
{
    study_session_t s;
    study_init(&s);
    const int64_t start_ms = esp_timer_get_time() / 1000;
    study_start(&s, 60000, start_ms);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec += 3600;
    TEST_ASSERT_EQUAL(0, settimeofday(&tv, NULL));

    const int64_t now_ms = esp_timer_get_time() / 1000;
    TEST_ASSERT_FALSE(study_tick(&s, now_ms));
    TEST_ASSERT_LESS_THAN_UINT32(1000, study_elapsed_ms(&s, now_ms));

    tv.tv_sec -= 3600;
    settimeofday(&tv, NULL);
}

// --- Gestures ------------------------------------------------------------

static void test_gesture_rejects_inconsistent_windows(void)
{
    gesture_detector_t g;
    TEST_ASSERT_FALSE(gesture_init(&g, (gesture_windows_t){.tap_max_ms = 800,
                                                           .double_gap_ms = 300,
                                                           .hold_min_ms = 800}));
    TEST_ASSERT_FALSE(gesture_init(&g, (gesture_windows_t){.tap_max_ms = 0,
                                                           .double_gap_ms = 300,
                                                           .hold_min_ms = 800}));
    TEST_ASSERT_FALSE(gesture_init(&g, (gesture_windows_t){.tap_max_ms = 300,
                                                           .double_gap_ms = 0,
                                                           .hold_min_ms = 800}));
    TEST_ASSERT_TRUE(gesture_init(&g, W));
}

static void test_gesture_tap_reported_after_double_gap(void)
{
    gesture_detector_t g;
    gesture_init(&g, W);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, true, 1000).kind);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, false, 1100).kind);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, false, 1100 + W.double_gap_ms).kind);
    const gesture_event_t e = gesture_update(&g, false, 1101 + W.double_gap_ms);
    TEST_ASSERT_EQUAL(GESTURE_TAP, e.kind);
    TEST_ASSERT_EQUAL_INT64(1000, e.press_start_ms);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, false, 5000).kind);
}

static void test_gesture_double_tap_never_also_taps(void)
{
    gesture_detector_t g;
    gesture_init(&g, W);
    gesture_update(&g, true, 0);
    gesture_update(&g, false, 100);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, true, 250).kind);
    const gesture_event_t e = gesture_update(&g, false, 350);
    TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, e.kind);
    TEST_ASSERT_EQUAL_INT64(0, e.press_start_ms);
    for (int64_t t = 360; t < 3000; t += 10) {
        TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, false, t).kind);
    }
}

static void test_gesture_hold_start_and_end(void)
{
    gesture_detector_t g;
    gesture_init(&g, W);
    gesture_update(&g, true, 0);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, true, W.hold_min_ms - 1).kind);
    TEST_ASSERT_EQUAL(GESTURE_HOLD_START, gesture_update(&g, true, W.hold_min_ms).kind);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, true, W.hold_min_ms + 500).kind);
    TEST_ASSERT_EQUAL(GESTURE_HOLD_END, gesture_update(&g, false, W.hold_min_ms + 600).kind);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, false, 10000).kind);
}

static void test_gesture_ambiguous_press_produces_nothing(void)
{
    gesture_detector_t g;
    gesture_init(&g, W);
    gesture_update(&g, true, 0);
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, false, W.tap_max_ms + 100).kind);
    for (int64_t t = W.tap_max_ms + 110; t < 5000; t += 10) {
        TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, false, t).kind);
    }
}

static void test_gesture_tap_then_hold_reports_both_in_order(void)
{
    gesture_detector_t g;
    gesture_init(&g, W);
    gesture_update(&g, true, 0);
    gesture_update(&g, false, 100);
    gesture_update(&g, true, 200);
    // The second press outlasts a tap: the first one is reported as a tap.
    TEST_ASSERT_EQUAL(GESTURE_NONE, gesture_update(&g, true, 200 + W.tap_max_ms).kind);
    TEST_ASSERT_EQUAL(GESTURE_TAP, gesture_update(&g, true, 201 + W.tap_max_ms).kind);
    TEST_ASSERT_EQUAL(GESTURE_HOLD_START, gesture_update(&g, true, 200 + W.hold_min_ms).kind);
    TEST_ASSERT_EQUAL(GESTURE_HOLD_END, gesture_update(&g, false, 2000).kind);
}

// --- Pet state: confirmation and priorities -------------------------------

static void test_confirmation_applies_once_with_fresh_tap(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    TEST_ASSERT_TRUE(pet_request_confirmation(&p, 42, 3, 1000));
    TEST_ASSERT_EQUAL(PET_ACTION_CONFIRM, pet_on_gesture(&p, ev(GESTURE_TAP, 2000), 3, 2700));
    TEST_ASSERT_EQUAL(CONFIRMATION_APPLIED, p.confirmation.state);
    TEST_ASSERT_EQUAL_UINT32(42, p.confirmation.operation_id);
    TEST_ASSERT_NOT_EQUAL(PET_ACTION_CONFIRM, pet_on_gesture(&p, ev(GESTURE_TAP, 3000), 3, 3700));
}

static void test_confirmation_double_tap_applies_once(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 7, 1, 0);
    TEST_ASSERT_EQUAL(PET_ACTION_CONFIRM, pet_on_gesture(&p, ev(GESTURE_DOUBLE_TAP, 100), 1, 500));
    TEST_ASSERT_EQUAL(STUDY_IDLE, p.study.phase);
}

static void test_confirmation_expires_without_effect(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 0);
    pet_tick(&p, CONFIRMATION_TIMEOUT_MS - 1, PET_NO_WAITING_TAP);
    TEST_ASSERT_EQUAL(CONFIRMATION_PENDING, p.confirmation.state);
    pet_tick(&p, CONFIRMATION_TIMEOUT_MS, PET_NO_WAITING_TAP);
    TEST_ASSERT_EQUAL(CONFIRMATION_CANCELLED, p.confirmation.state);
    TEST_ASSERT_NOT_EQUAL(PET_ACTION_CONFIRM,
                          pet_on_gesture(&p, ev(GESTURE_TAP, 15100), 1, 15800));
}

static void test_confirmation_late_tap_without_tick_is_rejected(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 0);
    // The tap arrives after expiry even if no tick ran in between.
    TEST_ASSERT_NOT_EQUAL(PET_ACTION_CONFIRM,
                          pet_on_gesture(&p, ev(GESTURE_TAP, 14900), 1, CONFIRMATION_TIMEOUT_MS));
    TEST_ASSERT_EQUAL(CONFIRMATION_CANCELLED, p.confirmation.state);
}

// The single tap is reported about 350 ms after the release. A tap released
// at 14.9 s is inside the 15 s window even if the event arrives later.
static void test_confirmation_accepts_tap_released_before_expiry(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 0);
    TEST_ASSERT_EQUAL(PET_ACTION_CONFIRM,
                      pet_on_gesture(&p, ev_released(GESTURE_TAP, 14800, 14900), 1, 15260));
    TEST_ASSERT_EQUAL(CONFIRMATION_APPLIED, p.confirmation.state);
}

static void test_confirmation_rejects_tap_released_at_expiry(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 0);
    TEST_ASSERT_EQUAL(PET_ACTION_NONE,
                      pet_on_gesture(&p, ev_released(GESTURE_TAP, 14900,
                                                     CONFIRMATION_TIMEOUT_MS),
                                     1, CONFIRMATION_TIMEOUT_MS + 360));
    TEST_ASSERT_EQUAL(CONFIRMATION_CANCELLED, p.confirmation.state);
}

// Same case through the detector and the periodic tick: the tick at 15 s must
// not cancel a confirmation whose tap was already released.
static void test_confirmation_loop_accepts_tap_released_at_14_9_s(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 0);
    TEST_ASSERT_EQUAL(1, run_loop_with_touch(&p, 14800, 14900, 17000));
    TEST_ASSERT_EQUAL(CONFIRMATION_APPLIED, p.confirmation.state);
    TEST_ASSERT_EQUAL(STUDY_IDLE, p.study.phase);
}

static void test_confirmation_loop_rejects_tap_released_after_expiry(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 0);
    TEST_ASSERT_EQUAL(0, run_loop_with_touch(&p, 14950, 15050, 17000));
    TEST_ASSERT_EQUAL(CONFIRMATION_CANCELLED, p.confirmation.state);
}

static void test_gesture_events_carry_first_release(void)
{
    gesture_detector_t g;
    gesture_init(&g, W);
    gesture_update(&g, true, 1000);
    gesture_update(&g, false, 1100);
    TEST_ASSERT_EQUAL_INT64(1100, gesture_waiting_tap_release_ms(&g));
    const gesture_event_t tap = gesture_update(&g, false, 1101 + W.double_gap_ms);
    TEST_ASSERT_EQUAL(GESTURE_TAP, tap.kind);
    TEST_ASSERT_EQUAL_INT64(1100, tap.release_ms);
    TEST_ASSERT_EQUAL_INT64(PET_NO_WAITING_TAP, gesture_waiting_tap_release_ms(&g));

    gesture_update(&g, true, 5000);
    gesture_update(&g, false, 5100);
    gesture_update(&g, true, 5200);
    const gesture_event_t dbl = gesture_update(&g, false, 5300);
    TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, dbl.kind);
    TEST_ASSERT_EQUAL_INT64(5100, dbl.release_ms);
}

static void test_confirmation_needs_press_started_after_request(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 1000);
    TEST_ASSERT_EQUAL(PET_ACTION_NONE, pet_on_gesture(&p, ev(GESTURE_TAP, 900), 1, 1500));
    TEST_ASSERT_EQUAL(CONFIRMATION_PENDING, p.confirmation.state);
    TEST_ASSERT_EQUAL(STUDY_IDLE, p.study.phase);
    TEST_ASSERT_EQUAL(PET_ACTION_CONFIRM, pet_on_gesture(&p, ev(GESTURE_TAP, 2000), 1, 2500));
}

static void test_confirmation_cancelled_by_epoch_change(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 4, 0);
    pet_on_epoch_change(&p, 5);
    TEST_ASSERT_EQUAL(CONFIRMATION_CANCELLED, p.confirmation.state);
    TEST_ASSERT_NOT_EQUAL(PET_ACTION_CONFIRM, pet_on_gesture(&p, ev(GESTURE_TAP, 100), 5, 500));
}

static void test_confirmation_rejects_tap_from_other_epoch(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 4, 0);
    TEST_ASSERT_EQUAL(PET_ACTION_NONE, pet_on_gesture(&p, ev(GESTURE_TAP, 100), 5, 500));
    TEST_ASSERT_EQUAL(CONFIRMATION_CANCELLED, p.confirmation.state);
}

static void test_only_one_pending_confirmation(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    TEST_ASSERT_TRUE(pet_request_confirmation(&p, 1, 1, 0));
    TEST_ASSERT_FALSE(pet_request_confirmation(&p, 2, 1, 100));
    TEST_ASSERT_EQUAL_UINT32(1, p.confirmation.operation_id);
}

static void test_hold_does_not_open_voice_while_confirmation_pending(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_request_confirmation(&p, 1, 1, 0);
    TEST_ASSERT_EQUAL(PET_ACTION_NONE, pet_on_gesture(&p, ev(GESTURE_HOLD_START, 100), 1, 900));
    TEST_ASSERT_EQUAL(PET_ACTION_NONE, pet_on_gesture(&p, ev(GESTURE_HOLD_END, 100), 1, 1500));
    TEST_ASSERT_EQUAL(CONFIRMATION_PENDING, p.confirmation.state);
}

static void test_hold_opens_voice_only_and_never_starts_study(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    TEST_ASSERT_EQUAL(PET_ACTION_VOICE_REQUEST,
                      pet_on_gesture(&p, ev(GESTURE_HOLD_START, 0), 1, 800));
    TEST_ASSERT_EQUAL(STUDY_IDLE, p.study.phase);
    TEST_ASSERT_EQUAL(PET_ACTION_VOICE_RELEASE,
                      pet_on_gesture(&p, ev(GESTURE_HOLD_END, 0), 1, 2000));
    TEST_ASSERT_EQUAL(STUDY_IDLE, p.study.phase);
}

static void test_voice_release_still_sent_if_confirmation_arrives_mid_hold(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_on_gesture(&p, ev(GESTURE_HOLD_START, 0), 1, 800);
    pet_request_confirmation(&p, 1, 1, 1000);
    TEST_ASSERT_EQUAL(PET_ACTION_VOICE_RELEASE,
                      pet_on_gesture(&p, ev(GESTURE_HOLD_END, 0), 1, 1500));
    TEST_ASSERT_EQUAL(CONFIRMATION_PENDING, p.confirmation.state);
}

static void test_tap_starts_study_and_double_tap_toggles_pause(void)
{
    pet_state_t p;
    pet_init(&p, 1500000);
    TEST_ASSERT_EQUAL(PET_ACTION_STUDY_STARTED, pet_on_gesture(&p, ev(GESTURE_TAP, 0), 1, 700));
    TEST_ASSERT_EQUAL_UINT32(1500000, p.study.target_ms);
    TEST_ASSERT_EQUAL(PET_ACTION_NONE, pet_on_gesture(&p, ev(GESTURE_TAP, 2000), 1, 2700));
    TEST_ASSERT_EQUAL(PET_ACTION_STUDY_PAUSED,
                      pet_on_gesture(&p, ev(GESTURE_DOUBLE_TAP, 3000), 1, 3300));
    TEST_ASSERT_EQUAL(PET_ACTION_STUDY_RESUMED,
                      pet_on_gesture(&p, ev(GESTURE_DOUBLE_TAP, 4000), 1, 4300));
    TEST_ASSERT_EQUAL(STUDY_RUNNING, p.study.phase);
}

static void test_double_tap_without_session_does_nothing(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    TEST_ASSERT_EQUAL(PET_ACTION_NONE, pet_on_gesture(&p, ev(GESTURE_DOUBLE_TAP, 0), 1, 300));
    TEST_ASSERT_EQUAL(STUDY_IDLE, p.study.phase);
}

static void test_session_continues_without_network_events(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    pet_on_gesture(&p, ev(GESTURE_TAP, 0), 1, 700);
    // Losing the link changes the epoch; the local session must not stop.
    pet_on_epoch_change(&p, 2);
    pet_tick(&p, 30000, PET_NO_WAITING_TAP);
    TEST_ASSERT_EQUAL(STUDY_RUNNING, p.study.phase);
    pet_tick(&p, 60700, PET_NO_WAITING_TAP);
    TEST_ASSERT_EQUAL(STUDY_FINISHED, p.study.phase);
}

static void test_manual_alert_and_return_to_auto_change_once(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    TEST_ASSERT_EQUAL(ALERT_MODE_AUTO, p.alert_mode);
    TEST_ASSERT_TRUE(pet_sound_reaction_alerts(&p));

    TEST_ASSERT_TRUE(pet_set_alert_mode(&p, ALERT_MODE_MANUAL));
    TEST_ASSERT_FALSE(pet_set_alert_mode(&p, ALERT_MODE_MANUAL));
    // The manual alert is already on: a sound reaction must not add another.
    TEST_ASSERT_FALSE(pet_sound_reaction_alerts(&p));

    TEST_ASSERT_TRUE(pet_set_alert_mode(&p, ALERT_MODE_AUTO));
    TEST_ASSERT_FALSE(pet_set_alert_mode(&p, ALERT_MODE_AUTO));
    TEST_ASSERT_TRUE(pet_sound_reaction_alerts(&p));
}

static void test_alert_mode_rejects_unknown_values(void)
{
    pet_state_t p;
    pet_init(&p, 60000);
    TEST_ASSERT_FALSE(pet_set_alert_mode(&p, (alert_mode_t)7));
    TEST_ASSERT_EQUAL(ALERT_MODE_AUTO, p.alert_mode);
}

void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_study_rejects_out_of_range_durations);
    RUN_TEST(test_study_finishes_once_at_target);
    RUN_TEST(test_study_pause_stops_time_and_resume_continues);
    RUN_TEST(test_study_rejects_start_while_active);
    RUN_TEST(test_study_operations_rejected_when_idle);
    RUN_TEST(test_study_new_session_after_finish_gets_new_id);
    RUN_TEST(test_study_reboot_with_marker_is_interrupted_without_duration);
    RUN_TEST(test_wall_clock_does_not_change_session);
    RUN_TEST(test_gesture_rejects_inconsistent_windows);
    RUN_TEST(test_gesture_tap_reported_after_double_gap);
    RUN_TEST(test_gesture_double_tap_never_also_taps);
    RUN_TEST(test_gesture_hold_start_and_end);
    RUN_TEST(test_gesture_ambiguous_press_produces_nothing);
    RUN_TEST(test_gesture_tap_then_hold_reports_both_in_order);
    RUN_TEST(test_confirmation_applies_once_with_fresh_tap);
    RUN_TEST(test_confirmation_double_tap_applies_once);
    RUN_TEST(test_confirmation_expires_without_effect);
    RUN_TEST(test_confirmation_late_tap_without_tick_is_rejected);
    RUN_TEST(test_confirmation_accepts_tap_released_before_expiry);
    RUN_TEST(test_confirmation_rejects_tap_released_at_expiry);
    RUN_TEST(test_confirmation_loop_accepts_tap_released_at_14_9_s);
    RUN_TEST(test_confirmation_loop_rejects_tap_released_after_expiry);
    RUN_TEST(test_gesture_events_carry_first_release);
    RUN_TEST(test_confirmation_needs_press_started_after_request);
    RUN_TEST(test_confirmation_cancelled_by_epoch_change);
    RUN_TEST(test_confirmation_rejects_tap_from_other_epoch);
    RUN_TEST(test_only_one_pending_confirmation);
    RUN_TEST(test_hold_does_not_open_voice_while_confirmation_pending);
    RUN_TEST(test_hold_opens_voice_only_and_never_starts_study);
    RUN_TEST(test_voice_release_still_sent_if_confirmation_arrives_mid_hold);
    RUN_TEST(test_tap_starts_study_and_double_tap_toggles_pause);
    RUN_TEST(test_double_tap_without_session_does_nothing);
    RUN_TEST(test_session_continues_without_network_events);
    RUN_TEST(test_manual_alert_and_return_to_auto_change_once);
    RUN_TEST(test_alert_mode_rejects_unknown_values);
    run_sound_reaction_tests();
    run_telemetry_tests();
    run_settings_tests();
    run_pin_tests();
    UNITY_END();
}
