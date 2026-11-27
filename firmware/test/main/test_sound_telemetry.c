#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "sound_reaction.h"
#include "telemetry.h"
#include "test_suites.h"
#include "unity.h"

static const sound_config_t CFG = {
    .threshold = 0.5f,
    .hysteresis = SOUND_DEFAULT_HYSTERESIS,
    .post_playback_ms = SOUND_DEFAULT_POST_PLAYBACK_MS,
};

static sound_reaction_t armed(void)
{
    sound_reaction_t r;
    sound_init(&r, &CFG);
    sound_set_calibrated(&r, true);
    return r;
}

// --- Sound reaction ------------------------------------------------------

static void test_sound_rejects_invalid_limits(void)
{
    const sound_config_t bad[] = {
        {.threshold = 0.0f, .hysteresis = 0.0f},
        {.threshold = 1.01f, .hysteresis = 0.05f},
        {.threshold = -0.2f, .hysteresis = 0.05f},
        {.threshold = 0.04f, .hysteresis = 0.05f}, // re-arm level would be negative
        {.threshold = 0.05f, .hysteresis = 0.05f}, // re-arm level would be zero
        {.threshold = 0.5f, .hysteresis = -0.01f},
        {.threshold = NAN, .hysteresis = 0.05f},
    };
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        TEST_ASSERT_FALSE_MESSAGE(sound_config_valid(&bad[i]), "invalid config accepted");
    }
    const sound_config_t edge = {.threshold = 1.0f, .hysteresis = 0.05f};
    TEST_ASSERT_TRUE(sound_config_valid(&edge));
}

static void test_sound_invalid_update_keeps_current_config(void)
{
    sound_reaction_t r = armed();
    const sound_config_t bad = {.threshold = 2.0f, .hysteresis = 0.05f};
    TEST_ASSERT_FALSE(sound_set_config(&r, &bad));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, r.config.threshold);
}

static void test_sound_disabled_until_calibrated(void)
{
    sound_reaction_t r;
    sound_init(&r, &CFG);
    TEST_ASSERT_FALSE(sound_update(&r, 0.9f, 0));
    sound_set_calibrated(&r, true);
    TEST_ASSERT_TRUE(sound_update(&r, 0.9f, 100));
}

static void test_sound_triggers_once_and_rearms_with_hysteresis(void)
{
    sound_reaction_t r = armed();
    TEST_ASSERT_FALSE(sound_update(&r, 0.49f, 0));
    TEST_ASSERT_TRUE(sound_update(&r, 0.50f, 200));
    TEST_ASSERT_FALSE(sound_update(&r, 0.80f, 400));
    // Inside the hysteresis band: still not re-armed.
    TEST_ASSERT_FALSE(sound_update(&r, 0.46f, 600));
    TEST_ASSERT_FALSE(sound_update(&r, 0.60f, 800));
    // Below threshold - hysteresis: re-armed, the next crossing triggers.
    TEST_ASSERT_FALSE(sound_update(&r, 0.44f, 1000));
    TEST_ASSERT_TRUE(sound_update(&r, 0.60f, 1200));
}

static void test_sound_ignores_own_playback_and_wait(void)
{
    sound_reaction_t r = armed();
    sound_playback_started(&r);
    TEST_ASSERT_FALSE(sound_update(&r, 0.95f, 0));
    sound_playback_finished(&r, 1000);
    TEST_ASSERT_FALSE(sound_update(&r, 0.95f, 1000 + SOUND_DEFAULT_POST_PLAYBACK_MS - 1));
    TEST_ASSERT_TRUE(sound_update(&r, 0.95f, 1000 + SOUND_DEFAULT_POST_PLAYBACK_MS));
}

// The acoustic loop A01 guards against: a reaction plays a sound, noise is
// still high when listening resumes, and it must not trigger again.
static void test_sound_reaction_does_not_loop_on_persistent_noise(void)
{
    sound_reaction_t r = armed();
    int64_t t = 0;
    TEST_ASSERT_TRUE(sound_update(&r, 0.9f, t));
    for (int cycle = 0; cycle < 5; ++cycle) {
        sound_playback_started(&r);
        t += 300;
        TEST_ASSERT_FALSE(sound_update(&r, 0.9f, t));
        sound_playback_finished(&r, t);
        t += SOUND_DEFAULT_POST_PLAYBACK_MS + 50;
        TEST_ASSERT_FALSE(sound_update(&r, 0.9f, t));
    }
    TEST_ASSERT_FALSE(sound_update(&r, 0.1f, t + 200));
    TEST_ASSERT_TRUE(sound_update(&r, 0.9f, t + 400));
}

static void test_sound_ignores_non_finite_levels(void)
{
    sound_reaction_t r = armed();
    TEST_ASSERT_FALSE(sound_update(&r, NAN, 0));
    TEST_ASSERT_FALSE(sound_update(&r, INFINITY, 0));
}

// --- Telemetry -----------------------------------------------------------

static void test_telemetry_empty_reads_nothing(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    telemetry_sample_t out[4];
    bool gap = true;
    TEST_ASSERT_EQUAL_UINT32(0, telemetry_first_sample_seq(&t));
    TEST_ASSERT_EQUAL(0, telemetry_read_samples(&t, 0, out, 4, &gap));
    TEST_ASSERT_FALSE(gap);
}

static void test_telemetry_reads_pages_in_order_without_duplicates(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    for (int i = 0; i < 10; ++i) {
        TEST_ASSERT_EQUAL_UINT32(i + 1, telemetry_add_sample(&t, i * 1000, 0.1f, 0));
    }
    telemetry_sample_t out[4];
    bool gap = true;
    uint32_t cursor = 0;
    uint32_t expected = 1;
    size_t n;
    while ((n = telemetry_read_samples(&t, cursor, out, 4, &gap)) > 0) {
        TEST_ASSERT_FALSE(gap);
        for (size_t i = 0; i < n; ++i) {
            TEST_ASSERT_EQUAL_UINT32(expected++, out[i].seq);
        }
        cursor = out[n - 1].seq;
    }
    TEST_ASSERT_EQUAL_UINT32(11, expected);
    // Repeating the last page request returns the same entries, not new ones.
    TEST_ASSERT_EQUAL(2, telemetry_read_samples(&t, 8, out, 4, &gap));
    TEST_ASSERT_EQUAL_UINT32(9, out[0].seq);
}

static void test_telemetry_five_minute_cut_is_fully_recoverable(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    for (int s = 0; s < 300; ++s) {
        telemetry_add_sample(&t, s * 1000, 0.2f, 1);
    }
    static telemetry_sample_t out[TELEMETRY_SAMPLE_CAPACITY];
    bool gap = true;
    TEST_ASSERT_EQUAL(300, telemetry_read_samples(&t, 0, out, TELEMETRY_SAMPLE_CAPACITY, &gap));
    TEST_ASSERT_FALSE(gap);
    TEST_ASSERT_EQUAL_UINT32(0, t.samples_dropped);
}

static void test_telemetry_overflow_is_counted_and_reported_as_gap(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    for (uint32_t s = 0; s < TELEMETRY_SAMPLE_CAPACITY + 5; ++s) {
        telemetry_add_sample(&t, s * 1000, 0.2f, 1);
    }
    TEST_ASSERT_EQUAL_UINT32(5, t.samples_dropped);
    TEST_ASSERT_EQUAL_UINT32(6, telemetry_first_sample_seq(&t));

    telemetry_sample_t out[3];
    bool gap = false;
    TEST_ASSERT_EQUAL(3, telemetry_read_samples(&t, 0, out, 3, &gap));
    TEST_ASSERT_TRUE(gap);
    TEST_ASSERT_EQUAL_UINT32(6, out[0].seq);
    TEST_ASSERT_EQUAL_INT64(5000, out[0].uptime_ms);

    // A reader that is up to date sees no gap.
    TEST_ASSERT_EQUAL(1, telemetry_read_samples(&t, TELEMETRY_SAMPLE_CAPACITY + 4, out, 3, &gap));
    TEST_ASSERT_FALSE(gap);
    TEST_ASSERT_EQUAL_UINT32(TELEMETRY_SAMPLE_CAPACITY + 5, out[0].seq);
}

static void test_telemetry_events_are_separate_from_samples(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    telemetry_add_sample(&t, 0, 0.1f, 0);
    TEST_ASSERT_EQUAL_UINT32(1, telemetry_add_event(&t, 10, TELEMETRY_EVENT_TOUCH, 1));
    telemetry_add_sample(&t, 1000, 0.1f, 0);
    TEST_ASSERT_EQUAL_UINT32(2, telemetry_add_event(&t, 1010, TELEMETRY_EVENT_SOUND_REACTION, 0));

    telemetry_event_t ev[4];
    bool gap = true;
    TEST_ASSERT_EQUAL(2, telemetry_read_events(&t, 0, ev, 4, &gap));
    TEST_ASSERT_FALSE(gap);
    TEST_ASSERT_EQUAL(TELEMETRY_EVENT_TOUCH, ev[0].kind);
    TEST_ASSERT_EQUAL(TELEMETRY_EVENT_SOUND_REACTION, ev[1].kind);
    TEST_ASSERT_EQUAL_UINT32(2, t.sample_next_seq - 1);
}

static void test_telemetry_event_overflow_counted(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    for (uint32_t i = 0; i < TELEMETRY_EVENT_CAPACITY + 2; ++i) {
        telemetry_add_event(&t, i, TELEMETRY_EVENT_TOUCH, i);
    }
    TEST_ASSERT_EQUAL_UINT32(2, t.events_dropped);
    telemetry_event_t ev[1];
    bool gap = false;
    TEST_ASSERT_EQUAL(1, telemetry_read_events(&t, 1, ev, 1, &gap));
    TEST_ASSERT_TRUE(gap);
    TEST_ASSERT_EQUAL_UINT32(3, ev[0].seq);
}

// A cursor beyond the last issued sequence comes from another boot or a
// broken client: it is flagged invalid and reads nothing, without a gap.
static void test_telemetry_rejects_cursor_beyond_last_seq(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    TEST_ASSERT_TRUE(telemetry_sample_cursor_valid(&t, 0));
    TEST_ASSERT_FALSE(telemetry_sample_cursor_valid(&t, 1));
    for (int i = 0; i < 5; ++i) {
        telemetry_add_sample(&t, i * 1000, 0.1f, 0);
    }
    TEST_ASSERT_TRUE(telemetry_sample_cursor_valid(&t, 5));
    TEST_ASSERT_FALSE(telemetry_sample_cursor_valid(&t, 6));

    telemetry_sample_t out[4];
    bool gap = true;
    TEST_ASSERT_EQUAL(0, telemetry_read_samples(&t, 5, out, 4, &gap));
    TEST_ASSERT_FALSE(gap);
    gap = true;
    TEST_ASSERT_EQUAL(0, telemetry_read_samples(&t, 6, out, 4, &gap));
    TEST_ASSERT_FALSE(gap);

    telemetry_add_event(&t, 0, TELEMETRY_EVENT_TOUCH, 1);
    TEST_ASSERT_TRUE(telemetry_event_cursor_valid(&t, 1));
    TEST_ASSERT_FALSE(telemetry_event_cursor_valid(&t, 2));
}

// after_seq + 1 must not wrap to 0 and return the whole buffer as a gap.
static void test_telemetry_max_cursor_does_not_wrap(void)
{
    static telemetry_t t;
    telemetry_init(&t);
    for (int i = 0; i < 3; ++i) {
        telemetry_add_sample(&t, i * 1000, 0.1f, 0);
        telemetry_add_event(&t, i * 1000, TELEMETRY_EVENT_TOUCH, 1);
    }
    TEST_ASSERT_FALSE(telemetry_sample_cursor_valid(&t, UINT32_MAX));
    TEST_ASSERT_FALSE(telemetry_event_cursor_valid(&t, UINT32_MAX));

    telemetry_sample_t out[4];
    bool gap = true;
    TEST_ASSERT_EQUAL(0, telemetry_read_samples(&t, UINT32_MAX, out, 4, &gap));
    TEST_ASSERT_FALSE(gap);
    telemetry_event_t ev[4];
    gap = true;
    TEST_ASSERT_EQUAL(0, telemetry_read_events(&t, UINT32_MAX, ev, 4, &gap));
    TEST_ASSERT_FALSE(gap);
}

void run_sound_reaction_tests(void)
{
    RUN_TEST(test_sound_rejects_invalid_limits);
    RUN_TEST(test_sound_invalid_update_keeps_current_config);
    RUN_TEST(test_sound_disabled_until_calibrated);
    RUN_TEST(test_sound_triggers_once_and_rearms_with_hysteresis);
    RUN_TEST(test_sound_ignores_own_playback_and_wait);
    RUN_TEST(test_sound_reaction_does_not_loop_on_persistent_noise);
    RUN_TEST(test_sound_ignores_non_finite_levels);
}

void run_telemetry_tests(void)
{
    RUN_TEST(test_telemetry_empty_reads_nothing);
    RUN_TEST(test_telemetry_reads_pages_in_order_without_duplicates);
    RUN_TEST(test_telemetry_five_minute_cut_is_fully_recoverable);
    RUN_TEST(test_telemetry_overflow_is_counted_and_reported_as_gap);
    RUN_TEST(test_telemetry_events_are_separate_from_samples);
    RUN_TEST(test_telemetry_event_overflow_counted);
    RUN_TEST(test_telemetry_rejects_cursor_beyond_last_seq);
    RUN_TEST(test_telemetry_max_cursor_does_not_wrap);
}
