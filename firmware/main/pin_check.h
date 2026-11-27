#pragma once

#include <stdbool.h>
#include <stddef.h>

// Pin rules for the ESP32-S3 SuperMini (T04). Pure logic so it can be tested
// without hardware. -1 means "not connected" and is never an error.

typedef enum {
    PIN_OK = 0,
    PIN_DISABLED,      // -1: peripheral not connected
    PIN_ERR_INVALID,   // does not exist on the ESP32-S3 (22-25, > 48, < -1)
    PIN_ERR_RESERVED,  // 19/20 USB console, 26-32 flash and PSRAM
    PIN_ERR_DUPLICATE, // configured for more than one signal
} pin_status_t;

// Checks gpio against the chip and against every configured pin (including
// itself once). A duplicated pin is rejected for every signal that uses it.
pin_status_t pin_check(int gpio, const int *configured, size_t count);

// Strapping pins (0, 3, 45, 46) work but can change the boot mode or the
// flash voltage if an external part drives them at reset: warn, do not reject.
bool pin_is_strapping(int gpio);
