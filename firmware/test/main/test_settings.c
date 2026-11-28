#include <stdbool.h>

#include "nvs.h"
#include "nvs_flash.h"
#include "settings.h"
#include "study_session.h"
#include "test_suites.h"
#include "unity.h"

// These tests use the NVS partition of the board they run on and erase it
// first. Flash the product firmware again afterwards; its stored settings
// will be back to the defaults.

static void fresh_storage(void)
{
    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_erase());
    TEST_ASSERT_EQUAL(ESP_OK, settings_storage_init());
}

static void test_settings_defaults_are_valid(void)
{
    const pet_settings_t d = settings_defaults();
    TEST_ASSERT_TRUE(settings_valid(&d));
}

static void test_settings_empty_storage_loads_defaults(void)
{
    fresh_storage();
    pet_settings_t s;
    bool used_defaults = false;
    TEST_ASSERT_EQUAL(ESP_OK, settings_load(&s, &used_defaults));
    TEST_ASSERT_TRUE(used_defaults);
    const pet_settings_t d = settings_defaults();
    TEST_ASSERT_EQUAL_FLOAT(d.sound_threshold, s.sound_threshold);
    TEST_ASSERT_EQUAL_UINT32(d.study_default_ms, s.study_default_ms);
}

static void test_settings_round_trip(void)
{
    fresh_storage();
    pet_settings_t s = settings_defaults();
    s.sound_threshold = 0.3f;
    s.study_default_ms = 45u * 60u * 1000u;
    s.volume_pct = 10;
    s.muted = true;
    TEST_ASSERT_EQUAL(ESP_OK, settings_save(&s));

    pet_settings_t loaded;
    bool used_defaults = true;
    TEST_ASSERT_EQUAL(ESP_OK, settings_load(&loaded, &used_defaults));
    TEST_ASSERT_FALSE(used_defaults);
    TEST_ASSERT_EQUAL_FLOAT(0.3f, loaded.sound_threshold);
    TEST_ASSERT_EQUAL_UINT32(45u * 60u * 1000u, loaded.study_default_ms);
    TEST_ASSERT_EQUAL_UINT8(10, loaded.volume_pct);
    TEST_ASSERT_TRUE(loaded.muted);
}

static void test_settings_invalid_save_keeps_stored_values(void)
{
    fresh_storage();
    pet_settings_t good = settings_defaults();
    good.sound_threshold = 0.4f;
    TEST_ASSERT_EQUAL(ESP_OK, settings_save(&good));

    pet_settings_t bad = good;
    bad.sound_threshold = 0.03f; // below the hysteresis
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, settings_save(&bad));
    bad = good;
    bad.study_default_ms = STUDY_MIN_DURATION_MS - 1;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, settings_save(&bad));
    bad = good;
    bad.volume_pct = 101;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, settings_save(&bad));

    pet_settings_t loaded;
    bool used_defaults = true;
    settings_load(&loaded, &used_defaults);
    TEST_ASSERT_FALSE(used_defaults);
    TEST_ASSERT_EQUAL_FLOAT(0.4f, loaded.sound_threshold);
}

static void test_settings_corrupt_blob_falls_back_to_defaults(void)
{
    fresh_storage();
    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open("pet", NVS_READWRITE, &h));
    const uint8_t junk[3] = {9, 9, 9};
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, "settings", junk, sizeof(junk)));
    nvs_commit(h);
    nvs_close(h);

    pet_settings_t s;
    bool used_defaults = false;
    TEST_ASSERT_EQUAL(ESP_OK, settings_load(&s, &used_defaults));
    TEST_ASSERT_TRUE(used_defaults);
}

static void test_session_marker_round_trip(void)
{
    fresh_storage();
    bool active = true;
    TEST_ASSERT_EQUAL(ESP_OK, settings_get_session_marker(&active));
    TEST_ASSERT_FALSE(active);
    TEST_ASSERT_EQUAL(ESP_OK, settings_set_session_marker(true));
    TEST_ASSERT_EQUAL(ESP_OK, settings_get_session_marker(&active));
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_EQUAL(ESP_OK, settings_set_session_marker(false));
    TEST_ASSERT_EQUAL(ESP_OK, settings_get_session_marker(&active));
    TEST_ASSERT_FALSE(active);
}

void run_settings_tests(void)
{
    RUN_TEST(test_settings_defaults_are_valid);
    RUN_TEST(test_settings_empty_storage_loads_defaults);
    RUN_TEST(test_settings_round_trip);
    RUN_TEST(test_settings_invalid_save_keeps_stored_values);
    RUN_TEST(test_settings_corrupt_blob_falls_back_to_defaults);
    RUN_TEST(test_session_marker_round_trip);
}
