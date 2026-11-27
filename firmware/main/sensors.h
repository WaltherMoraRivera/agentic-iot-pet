#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

// Hardware inputs and the listening LED (T04). Each peripheral is optional:
// with its pins at -1 it stays disabled and the rest keeps working.

typedef struct {
    bool touch;
    bool led;
    bool mic;
} sensors_available_t;

// Configures whatever has valid pins and starts the microphone task.
sensors_available_t sensors_init(void);

// False when the touch sensor is not available.
bool sensors_touch_pressed(bool *pressed);

void sensors_set_led(bool on);

// Latest relative sound level (0..1, digital full scale, not dB) of a 100 ms
// window. Returns false when there is no new window since the last call.
// Samples are only used to compute the level; they are never stored.
bool sensors_take_sound_level(float *level);

// True if the pin can be used: exists, is not reserved (19/20 USB console,
// 26-32 flash/PSRAM) and is not assigned to another signal in menuconfig.
// Strapping pins are accepted with a warning. Logs why a pin is rejected.
bool sensors_pin_usable(int gpio, const char *label);
