#include <stddef.h>

#include "pin_check.h"
#include "test_suites.h"
#include "unity.h"

static void test_pin_disabled_is_not_an_error(void)
{
    const int pins[] = {-1, -1, 4};
    TEST_ASSERT_EQUAL(PIN_DISABLED, pin_check(-1, pins, 3));
    TEST_ASSERT_EQUAL(PIN_OK, pin_check(4, pins, 3));
}

static void test_pin_rejects_missing_gpio(void)
{
    const int pins[] = {22, 25, 49, -2};
    for (size_t i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL(PIN_ERR_INVALID, pin_check(pins[i], pins, 4));
    }
    const int ok[] = {21, 33, 48};
    for (size_t i = 0; i < 3; ++i) {
        TEST_ASSERT_EQUAL(PIN_OK, pin_check(ok[i], ok, 3));
    }
}

static void test_pin_rejects_usb_and_flash_pins(void)
{
    const int pins[] = {19, 20, 26, 29, 32};
    for (size_t i = 0; i < 5; ++i) {
        TEST_ASSERT_EQUAL(PIN_ERR_RESERVED, pin_check(pins[i], pins, 5));
    }
}

static void test_pin_rejects_duplicates_for_every_signal(void)
{
    // The LED and the mic SD are both set to GPIO5.
    const int pins[] = {4, 5, 6, 7, 5, -1, -1, -1};
    TEST_ASSERT_EQUAL(PIN_OK, pin_check(pins[0], pins, 8));
    TEST_ASSERT_EQUAL(PIN_ERR_DUPLICATE, pin_check(pins[1], pins, 8));
    TEST_ASSERT_EQUAL(PIN_ERR_DUPLICATE, pin_check(pins[4], pins, 8));
    // Several disconnected peripherals are not duplicates of each other.
    TEST_ASSERT_EQUAL(PIN_DISABLED, pin_check(pins[5], pins, 8));
}

static void test_pin_strapping_only_warns(void)
{
    const int pins[] = {0, 3, 45, 46};
    for (size_t i = 0; i < 4; ++i) {
        TEST_ASSERT_TRUE(pin_is_strapping(pins[i]));
        TEST_ASSERT_EQUAL(PIN_OK, pin_check(pins[i], pins, 4));
    }
    TEST_ASSERT_FALSE(pin_is_strapping(4));
    TEST_ASSERT_FALSE(pin_is_strapping(-1));
}

void run_pin_tests(void)
{
    RUN_TEST(test_pin_disabled_is_not_an_error);
    RUN_TEST(test_pin_rejects_missing_gpio);
    RUN_TEST(test_pin_rejects_usb_and_flash_pins);
    RUN_TEST(test_pin_rejects_duplicates_for_every_signal);
    RUN_TEST(test_pin_strapping_only_warns);
}
