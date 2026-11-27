#pragma once

#include <stdbool.h>

// Validates that a configured pin can be used by the bench on ESP32-S3.
// Rejects -1 (not configured), pins that do not exist, the USB pins used by
// the console and GPIO26-32 (in-package flash/PSRAM). Warns about GPIO33-37
// (octal PSRAM) and strapping pins. Logs the reason using `label`.
bool bench_pin_ok(int gpio, const char *label);

// Validates several pins and that none of them repeats.
bool bench_pins_ok(const int *gpios, const char *const *labels, int count);

// Blocks forever after a test cannot start, keeping the console alive.
void bench_idle_forever(const char *reason);

void test_touch_led_run(void);
void test_i2c_scan_run(void);
void test_oled_run(void);
void test_mic_run(void);
void test_amp_run(void);
